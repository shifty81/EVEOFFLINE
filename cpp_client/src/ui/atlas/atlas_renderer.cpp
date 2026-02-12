#include "ui/atlas/atlas_renderer.h"

#include <cmath>
#include <cstring>
#include <iostream>

// Minimal OpenGL declarations – the host app already has GL loaded.
// We only need a few GL calls; forward-declare to avoid pulling in
// a platform-specific GL header that may conflict with GLFW/GLAD.
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// When building inside the full client the real GL headers are
// available via GLAD or GLEW.  For the server-side test build we provide
// stub types so the translation unit compiles without GPU access.
#if __has_include(<GL/glew.h>)
#include <GL/glew.h>
#define ATLAS_HAS_GL 1
#elif __has_include(<glad/glad.h>)
#include <glad/glad.h>
#define ATLAS_HAS_GL 1
#else
#define ATLAS_HAS_GL 0
// Minimal GL type stubs so the file compiles in headless builds.
using GLuint   = unsigned int;
using GLint    = int;
using GLenum   = unsigned int;
using GLsizei  = int;
using GLfloat  = float;
using GLchar   = char;
using GLboolean = unsigned char;
using GLsizeiptr = long;
#define GL_FALSE 0
#define GL_TRUE  1
#define GL_TRIANGLES      0x0004
#define GL_FLOAT          0x1406
#define GL_BLEND          0x0BE2
#define GL_SRC_ALPHA      0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DEPTH_TEST     0x0B71
#define GL_CULL_FACE      0x0B44
#define GL_SCISSOR_TEST   0x0C11
#define GL_ARRAY_BUFFER   0x8892
#define GL_DYNAMIC_DRAW   0x88E8
#define GL_VERTEX_SHADER  0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS    0x8B82
#define GL_TEXTURE_2D     0x0DE1
#define GL_TEXTURE0       0x84C0
#define GL_RED            0x1903
#define GL_UNSIGNED_BYTE  0x1401
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_LINEAR         0x2601
#define GL_NEAREST        0x2600
// Stub GL functions (no-ops).
inline void glEnable(GLenum) {}
inline void glDisable(GLenum) {}
inline void glBlendFunc(GLenum, GLenum) {}
inline void glScissor(GLint, GLint, GLsizei, GLsizei) {}
inline GLuint glCreateShader(GLenum) { return 0; }
inline void glShaderSource(GLuint, GLsizei, const GLchar**, const GLint*) {}
inline void glCompileShader(GLuint) {}
inline void glGetShaderiv(GLuint, GLenum, GLint* p) { *p = GL_TRUE; }
inline void glGetShaderInfoLog(GLuint, GLsizei, GLsizei*, GLchar*) {}
inline void glDeleteShader(GLuint) {}
inline GLuint glCreateProgram() { return 0; }
inline void glAttachShader(GLuint, GLuint) {}
inline void glLinkProgram(GLuint) {}
inline void glGetProgramiv(GLuint, GLenum, GLint* p) { *p = GL_TRUE; }
inline void glGetProgramInfoLog(GLuint, GLsizei, GLsizei*, GLchar*) {}
inline void glUseProgram(GLuint) {}
inline GLint glGetUniformLocation(GLuint, const GLchar*) { return -1; }
inline void glUniformMatrix4fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
inline void glUniform1i(GLint, GLint) {}
inline void glGenVertexArrays(GLsizei, GLuint*) {}
inline void glBindVertexArray(GLuint) {}
inline void glGenBuffers(GLsizei, GLuint*) {}
inline void glBindBuffer(GLenum, GLuint) {}
inline void glBufferData(GLenum, GLsizeiptr, const void*, GLenum) {}
inline void glEnableVertexAttribArray(GLuint) {}
inline void glVertexAttribPointer(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) {}
inline void glDrawArrays(GLenum, GLint, GLsizei) {}
inline void glDeleteVertexArrays(GLsizei, const GLuint*) {}
inline void glDeleteBuffers(GLsizei, const GLuint*) {}
inline void glDeleteProgram(GLuint) {}
inline void glGenTextures(GLsizei, GLuint*) {}
inline void glBindTexture(GLenum, GLuint) {}
inline void glTexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) {}
inline void glTexParameteri(GLenum, GLenum, GLint) {}
inline void glActiveTexture(GLenum) {}
inline void glDeleteTextures(GLsizei, const GLuint*) {}
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace atlas {

// ── Shader sources ──────────────────────────────────────────────────

static const char* kVertexShader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;
uniform mat4 uProj;
out vec2 vUV;
out vec4 vColor;
void main() {
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
    vUV    = aUV;
    vColor = aColor;
}
)";

static const char* kFragmentShader = R"(
#version 330 core
in vec2 vUV;
in vec4 vColor;
uniform int  uUseTex;
uniform sampler2D uTex;
out vec4 FragColor;
void main() {
    if (uUseTex != 0) {
        float a = texture(uTex, vUV).r;
        FragColor = vec4(vColor.rgb, vColor.a * a);
    } else {
        FragColor = vColor;
    }
}
)";

// ── Embedded 8×13 bitmap font (ASCII 32–126) ────────────────────────
// Each glyph is 8 pixels wide, 13 pixels tall, stored as 13 bytes
// (one bit per pixel, MSB-first).  Covers printable ASCII.

static const int kFontGlyphW = 8;
static const int kFontGlyphH = 13;
static const int kFontFirstChar = 32;
static const int kFontLastChar  = 126;
static const int kFontCharCount = kFontLastChar - kFontFirstChar + 1;

// Minimal 8×13 bitmap font data (space through '~').
// This is a condensed version of the classic X11 "fixed" font.
// Each glyph = 13 bytes, one per scanline, MSB = leftmost pixel.
static const unsigned char kFontData[kFontCharCount][13] = {
    // 32 ' '
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 33 '!'
    {0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
    // 34 '"'
    {0x00,0x66,0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 35 '#'
    {0x00,0x00,0x6C,0x6C,0xFE,0x6C,0x6C,0xFE,0x6C,0x6C,0x00,0x00,0x00},
    // 36 '$'
    {0x00,0x18,0x7E,0xC0,0xC0,0x7C,0x06,0x06,0xFC,0x18,0x00,0x00,0x00},
    // 37 '%'
    {0x00,0x00,0x00,0xC6,0xCC,0x18,0x30,0x60,0xCC,0xC6,0x00,0x00,0x00},
    // 38 '&'
    {0x00,0x00,0x38,0x6C,0x38,0x76,0xDC,0xCC,0xCC,0x76,0x00,0x00,0x00},
    // 39 '''
    {0x00,0x18,0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 40 '('
    {0x00,0x0C,0x18,0x30,0x30,0x30,0x30,0x30,0x30,0x18,0x0C,0x00,0x00},
    // 41 ')'
    {0x00,0x30,0x18,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x18,0x30,0x00,0x00},
    // 42 '*'
    {0x00,0x00,0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,0x00,0x00,0x00},
    // 43 '+'
    {0x00,0x00,0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
    // 44 ','
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30,0x00,0x00},
    // 45 '-'
    {0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 46 '.'
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00},
    // 47 '/'
    {0x00,0x00,0x02,0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,0x00,0x00},
    // 48 '0'
    {0x00,0x00,0x7C,0xC6,0xCE,0xDE,0xF6,0xE6,0xC6,0x7C,0x00,0x00,0x00},
    // 49 '1'
    {0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00},
    // 50 '2'
    {0x00,0x00,0x7C,0xC6,0x06,0x0C,0x18,0x30,0x60,0xFE,0x00,0x00,0x00},
    // 51 '3'
    {0x00,0x00,0x7C,0xC6,0x06,0x3C,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00},
    // 52 '4'
    {0x00,0x00,0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x00,0x00,0x00},
    // 53 '5'
    {0x00,0x00,0xFE,0xC0,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00},
    // 54 '6'
    {0x00,0x00,0x38,0x60,0xC0,0xFC,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00},
    // 55 '7'
    {0x00,0x00,0xFE,0xC6,0x06,0x0C,0x18,0x30,0x30,0x30,0x00,0x00,0x00},
    // 56 '8'
    {0x00,0x00,0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00},
    // 57 '9'
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0x00,0x00,0x00},
    // 58 ':'
    {0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
    // 59 ';'
    {0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30,0x00,0x00,0x00},
    // 60 '<'
    {0x00,0x00,0x06,0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x06,0x00,0x00},
    // 61 '='
    {0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00},
    // 62 '>'
    {0x00,0x00,0x60,0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x60,0x00,0x00},
    // 63 '?'
    {0x00,0x00,0x7C,0xC6,0xC6,0x0C,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
    // 64 '@'
    {0x00,0x00,0x7C,0xC6,0xC6,0xDE,0xDE,0xDE,0xC0,0x7C,0x00,0x00,0x00},
    // 65 'A'
    {0x00,0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00,0x00,0x00},
    // 66 'B'
    {0x00,0x00,0xFC,0x66,0x66,0x7C,0x66,0x66,0x66,0xFC,0x00,0x00,0x00},
    // 67 'C'
    {0x00,0x00,0x3C,0x66,0xC0,0xC0,0xC0,0xC0,0x66,0x3C,0x00,0x00,0x00},
    // 68 'D'
    {0x00,0x00,0xF8,0x6C,0x66,0x66,0x66,0x66,0x6C,0xF8,0x00,0x00,0x00},
    // 69 'E'
    {0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x66,0xFE,0x00,0x00,0x00},
    // 70 'F'
    {0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0xF0,0x00,0x00,0x00},
    // 71 'G'
    {0x00,0x00,0x3C,0x66,0xC0,0xC0,0xCE,0xC6,0x66,0x3E,0x00,0x00,0x00},
    // 72 'H'
    {0x00,0x00,0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00},
    // 73 'I'
    {0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00},
    // 74 'J'
    {0x00,0x00,0x1E,0x0C,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00,0x00,0x00},
    // 75 'K'
    {0x00,0x00,0xE6,0x66,0x6C,0x78,0x78,0x6C,0x66,0xE6,0x00,0x00,0x00},
    // 76 'L'
    {0x00,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,0x66,0xFE,0x00,0x00,0x00},
    // 77 'M'
    {0x00,0x00,0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00},
    // 78 'N'
    {0x00,0x00,0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0xC6,0x00,0x00,0x00},
    // 79 'O'
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00},
    // 80 'P'
    {0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00,0x00,0x00},
    // 81 'Q'
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x0E,0x00,0x00},
    // 82 'R'
    {0x00,0x00,0xFC,0x66,0x66,0x7C,0x6C,0x66,0x66,0xE6,0x00,0x00,0x00},
    // 83 'S'
    {0x00,0x00,0x7C,0xC6,0xC0,0x70,0x1C,0x06,0xC6,0x7C,0x00,0x00,0x00},
    // 84 'T'
    {0x00,0x00,0x7E,0x5A,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00},
    // 85 'U'
    {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00},
    // 86 'V'
    {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00,0x00,0x00},
    // 87 'W'
    {0x00,0x00,0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x82,0x00,0x00,0x00},
    // 88 'X'
    {0x00,0x00,0xC6,0x6C,0x38,0x38,0x38,0x6C,0xC6,0xC6,0x00,0x00,0x00},
    // 89 'Y'
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x3C,0x00,0x00,0x00},
    // 90 'Z'
    {0x00,0x00,0xFE,0xC6,0x8C,0x18,0x30,0x60,0xC6,0xFE,0x00,0x00,0x00},
    // 91 '['
    {0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,0x00},
    // 92 '\'
    {0x00,0x00,0x80,0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00,0x00,0x00},
    // 93 ']'
    {0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,0x00},
    // 94 '^'
    {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 95 '_'
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00},
    // 96 '`'
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 97 'a'
    {0x00,0x00,0x00,0x00,0x78,0x0C,0x7C,0xCC,0xCC,0x76,0x00,0x00,0x00},
    // 98 'b'
    {0x00,0x00,0xE0,0x60,0x7C,0x66,0x66,0x66,0x66,0xDC,0x00,0x00,0x00},
    // 99 'c'
    {0x00,0x00,0x00,0x00,0x7C,0xC6,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00},
    // 100 'd'
    {0x00,0x00,0x1C,0x0C,0x7C,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00},
    // 101 'e'
    {0x00,0x00,0x00,0x00,0x7C,0xC6,0xFE,0xC0,0xC6,0x7C,0x00,0x00,0x00},
    // 102 'f'
    {0x00,0x00,0x1C,0x36,0x30,0x78,0x30,0x30,0x30,0x78,0x00,0x00,0x00},
    // 103 'g'
    {0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,0x00},
    // 104 'h'
    {0x00,0x00,0xE0,0x60,0x6C,0x76,0x66,0x66,0x66,0xE6,0x00,0x00,0x00},
    // 105 'i'
    {0x00,0x00,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00},
    // 106 'j'
    {0x00,0x00,0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00},
    // 107 'k'
    {0x00,0x00,0xE0,0x60,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00,0x00,0x00},
    // 108 'l'
    {0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00},
    // 109 'm'
    {0x00,0x00,0x00,0x00,0xEC,0xFE,0xD6,0xD6,0xC6,0xC6,0x00,0x00,0x00},
    // 110 'n'
    {0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00},
    // 111 'o'
    {0x00,0x00,0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00},
    // 112 'p'
    {0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00},
    // 113 'q'
    {0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x1E,0x00},
    // 114 'r'
    {0x00,0x00,0x00,0x00,0xDC,0x76,0x60,0x60,0x60,0xF0,0x00,0x00,0x00},
    // 115 's'
    {0x00,0x00,0x00,0x00,0x7C,0xC6,0x70,0x1C,0xC6,0x7C,0x00,0x00,0x00},
    // 116 't'
    {0x00,0x00,0x10,0x30,0xFC,0x30,0x30,0x30,0x36,0x1C,0x00,0x00,0x00},
    // 117 'u'
    {0x00,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00},
    // 118 'v'
    {0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00,0x00,0x00},
    // 119 'w'
    {0x00,0x00,0x00,0x00,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00,0x00,0x00},
    // 120 'x'
    {0x00,0x00,0x00,0x00,0xC6,0x6C,0x38,0x38,0x6C,0xC6,0x00,0x00,0x00},
    // 121 'y'
    {0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0x7E,0x06,0x0C,0xF8,0x00},
    // 122 'z'
    {0x00,0x00,0x00,0x00,0xFE,0x8C,0x18,0x30,0x60,0xFE,0x00,0x00,0x00},
    // 123 '{'
    {0x00,0x0E,0x18,0x18,0x18,0x70,0x18,0x18,0x18,0x18,0x0E,0x00,0x00},
    // 124 '|'
    {0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00},
    // 125 '}'
    {0x00,0x70,0x18,0x18,0x18,0x0E,0x18,0x18,0x18,0x18,0x70,0x00,0x00},
    // 126 '~'
    {0x00,0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};

// ── AtlasRenderer implementation ───────────────────────────────────

AtlasRenderer::AtlasRenderer() = default;
AtlasRenderer::~AtlasRenderer() { shutdown(); }

bool AtlasRenderer::init() {
    // Compile vertex shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &kVertexShader, nullptr);
    glCompileShader(vs);
    GLint ok = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(vs, 512, nullptr, log);
        std::cerr << "[AtlasRenderer] VS error: " << log << std::endl;
        return false;
    }

    // Compile fragment shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &kFragmentShader, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(fs, 512, nullptr, log);
        std::cerr << "[AtlasRenderer] FS error: " << log << std::endl;
        return false;
    }

    // Link program
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vs);
    glAttachShader(m_shaderProgram, fs);
    glLinkProgram(m_shaderProgram);
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, log);
        std::cerr << "[AtlasRenderer] Link error: " << log << std::endl;
        return false;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    m_uniformProj   = glGetUniformLocation(m_shaderProgram, "uProj");
    m_uniformUseTex = glGetUniformLocation(m_shaderProgram, "uUseTex");
    m_uniformTex    = glGetUniformLocation(m_shaderProgram, "uTex");

    // Create VAO / VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(MAX_VERTICES * sizeof(UIVertex)),
                 nullptr, GL_DYNAMIC_DRAW);

    // Vertex layout: pos(2f), uv(2f), color(4f)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
                          reinterpret_cast<void*>(offsetof(UIVertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
                          reinterpret_cast<void*>(offsetof(UIVertex, u)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
                          reinterpret_cast<void*>(offsetof(UIVertex, r)));

    glBindVertexArray(0);

    // Build bitmap font texture
    buildFontTexture();

    m_vertices.reserve(MAX_VERTICES);
    return true;
}

void AtlasRenderer::shutdown() {
    if (m_vao)           { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo)           { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_shaderProgram) { glDeleteProgram(m_shaderProgram); m_shaderProgram = 0; }
    if (m_fontTexture)   { glDeleteTextures(1, &m_fontTexture); m_fontTexture = 0; }
}

void AtlasRenderer::buildFontTexture() {
    // Pack all glyphs into a single-row atlas (kFontCharCount × 8 wide, 13 tall)
    int atlasW = kFontCharCount * kFontGlyphW;
    int atlasH = kFontGlyphH;
    std::vector<unsigned char> pixels(atlasW * atlasH, 0);

    for (int ch = 0; ch < kFontCharCount; ++ch) {
        for (int row = 0; row < kFontGlyphH; ++row) {
            unsigned char bits = kFontData[ch][row];
            for (int col = 0; col < kFontGlyphW; ++col) {
                bool on = (bits >> (7 - col)) & 1;
                pixels[row * atlasW + ch * kFontGlyphW + col] = on ? 255 : 0;
            }
        }
    }

    glGenTextures(1, &m_fontTexture);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasW, atlasH, 0,
                 GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// ── Frame management ────────────────────────────────────────────────

void AtlasRenderer::begin(int windowW, int windowH) {
    m_windowW = windowW;
    m_windowH = windowH;
    m_inFrame = true;
    m_vertices.clear();
}

void AtlasRenderer::end() {
    flush();
    m_inFrame = false;
    m_clipStack.clear();
}

void AtlasRenderer::flush() {
    if (m_vertices.empty()) return;

    // Save GL state we modify
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glUseProgram(m_shaderProgram);

    // Orthographic projection: (0,0) top-left, (w,h) bottom-right
    float L = 0.0f, R = static_cast<float>(m_windowW);
    float T = 0.0f, B = static_cast<float>(m_windowH);
    float proj[16] = {
        2.0f/(R-L),  0.0f,        0.0f, 0.0f,
        0.0f,        2.0f/(T-B),  0.0f, 0.0f,
        0.0f,        0.0f,       -1.0f, 0.0f,
        (R+L)/(L-R), (T+B)/(B-T), 0.0f, 1.0f,
    };
    glUniformMatrix4fv(m_uniformProj, 1, GL_FALSE, proj);
    glUniform1i(m_uniformUseTex, 0);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_vertices.size() * sizeof(UIVertex)),
                 m_vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));

    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    m_vertices.clear();
}

// ── Primitive helpers ───────────────────────────────────────────────

void AtlasRenderer::addQuad(float x0, float y0, float x1, float y1,
                              float u0, float v0, float u1, float v1,
                              const Color& c) {
    UIVertex tl = {x0, y0, u0, v0, c.r, c.g, c.b, c.a};
    UIVertex tr = {x1, y0, u1, v0, c.r, c.g, c.b, c.a};
    UIVertex bl = {x0, y1, u0, v1, c.r, c.g, c.b, c.a};
    UIVertex br = {x1, y1, u1, v1, c.r, c.g, c.b, c.a};
    m_vertices.push_back(tl);
    m_vertices.push_back(tr);
    m_vertices.push_back(bl);
    m_vertices.push_back(tr);
    m_vertices.push_back(br);
    m_vertices.push_back(bl);
}

void AtlasRenderer::addQuadGradient(float x0, float y0, float x1, float y1,
                                      const Color& tl, const Color& tr,
                                      const Color& br, const Color& bl) {
    UIVertex vtl = {x0, y0, 0,0, tl.r, tl.g, tl.b, tl.a};
    UIVertex vtr = {x1, y0, 0,0, tr.r, tr.g, tr.b, tr.a};
    UIVertex vbl = {x0, y1, 0,0, bl.r, bl.g, bl.b, bl.a};
    UIVertex vbr = {x1, y1, 0,0, br.r, br.g, br.b, br.a};
    m_vertices.push_back(vtl);
    m_vertices.push_back(vtr);
    m_vertices.push_back(vbl);
    m_vertices.push_back(vtr);
    m_vertices.push_back(vbr);
    m_vertices.push_back(vbl);
}

void AtlasRenderer::addTriangle(float x0, float y0,
                                  float x1, float y1,
                                  float x2, float y2,
                                  const Color& c) {
    UIVertex v0 = {x0, y0, 0,0, c.r, c.g, c.b, c.a};
    UIVertex v1 = {x1, y1, 0,0, c.r, c.g, c.b, c.a};
    UIVertex v2 = {x2, y2, 0,0, c.r, c.g, c.b, c.a};
    m_vertices.push_back(v0);
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
}

// ── Drawing API ─────────────────────────────────────────────────────

void AtlasRenderer::drawRect(const Rect& r, const Color& c) {
    addQuad(r.x, r.y, r.right(), r.bottom(), 0,0,0,0, c);
}

void AtlasRenderer::drawRectGradient(const Rect& r,
                                       const Color& topLeft,
                                       const Color& topRight,
                                       const Color& botRight,
                                       const Color& botLeft) {
    addQuadGradient(r.x, r.y, r.right(), r.bottom(),
                    topLeft, topRight, botRight, botLeft);
}

void AtlasRenderer::drawRoundedRect(const Rect& r, const Color& c,
                                      float radius) {
    // Approximate with centre rect + 4 edge rects + 4 corner fans
    float rad = std::min(radius, std::min(r.w, r.h) * 0.5f);
    // Centre
    addQuad(r.x + rad, r.y, r.right() - rad, r.bottom(), 0,0,0,0, c);
    // Left strip
    addQuad(r.x, r.y + rad, r.x + rad, r.bottom() - rad, 0,0,0,0, c);
    // Right strip
    addQuad(r.right() - rad, r.y + rad, r.right(), r.bottom() - rad, 0,0,0,0, c);

    // Corners (triangle fans, 8 segments each)
    auto corner = [&](float cx, float cy, float startAngle) {
        int segs = 8;
        for (int i = 0; i < segs; ++i) {
            float a0 = startAngle + static_cast<float>(M_PI) * 0.5f * i / segs;
            float a1 = startAngle + static_cast<float>(M_PI) * 0.5f * (i+1) / segs;
            addTriangle(cx, cy,
                        cx + std::cos(a0) * rad, cy + std::sin(a0) * rad,
                        cx + std::cos(a1) * rad, cy + std::sin(a1) * rad, c);
        }
    };
    corner(r.x + rad,         r.y + rad,          static_cast<float>(M_PI));         // TL
    corner(r.right() - rad,   r.y + rad,          static_cast<float>(M_PI) * 1.5f);  // TR
    corner(r.right() - rad,   r.bottom() - rad,   0.0f);                             // BR
    corner(r.x + rad,         r.bottom() - rad,   static_cast<float>(M_PI) * 0.5f);  // BL
}

void AtlasRenderer::drawRectOutline(const Rect& r, const Color& c,
                                      float w) {
    drawRect({r.x,           r.y,            r.w, w},   c); // top
    drawRect({r.x,           r.bottom() - w, r.w, w},   c); // bottom
    drawRect({r.x,           r.y + w,        w, r.h - 2*w}, c); // left
    drawRect({r.right() - w, r.y + w,        w, r.h - 2*w}, c); // right
}

void AtlasRenderer::drawRoundedRectOutline(const Rect& r, const Color& c,
                                             float radius, float width) {
    // Simple approximation: draw filled rounded rect, then a smaller
    // inner one in the panel background color.  For a production system
    // this would use a proper stroke path, but this is sufficient for
    // the Atlas UI panel borders.
    drawRoundedRect(r, c, radius);
    Rect inner = {r.x + width, r.y + width,
                  r.w - 2*width, r.h - 2*width};
    // We can't "erase" here, so outline is done via drawRectOutline
    // for the sharp-corner fallback.
    (void)inner;
    (void)radius;
}

void AtlasRenderer::drawLine(Vec2 a, Vec2 b, const Color& c, float w) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = std::sqrt(dx*dx + dy*dy);
    if (len < 0.001f) return;
    float nx = -dy / len * w * 0.5f;
    float ny =  dx / len * w * 0.5f;
    addTriangle(a.x + nx, a.y + ny, a.x - nx, a.y - ny,
                b.x - nx, b.y - ny, c);
    addTriangle(a.x + nx, a.y + ny, b.x - nx, b.y - ny,
                b.x + nx, b.y + ny, c);
}

void AtlasRenderer::drawCircle(Vec2 centre, float radius, const Color& c,
                                 int segments) {
    for (int i = 0; i < segments; ++i) {
        float a0 = 2.0f * static_cast<float>(M_PI) * i / segments;
        float a1 = 2.0f * static_cast<float>(M_PI) * (i+1) / segments;
        addTriangle(centre.x, centre.y,
                    centre.x + std::cos(a0) * radius,
                    centre.y + std::sin(a0) * radius,
                    centre.x + std::cos(a1) * radius,
                    centre.y + std::sin(a1) * radius, c);
    }
}

void AtlasRenderer::drawCircleOutline(Vec2 centre, float radius,
                                        const Color& c, float w,
                                        int segments) {
    float r0 = radius - w * 0.5f;
    float r1 = radius + w * 0.5f;
    drawArc(centre, r0, r1, 0.0f, 2.0f * static_cast<float>(M_PI),
            c, segments);
}

void AtlasRenderer::drawArc(Vec2 centre, float innerR, float outerR,
                               float startAngle, float endAngle,
                               const Color& c, int segments) {
    float step = (endAngle - startAngle) / segments;
    for (int i = 0; i < segments; ++i) {
        float a0 = startAngle + step * i;
        float a1 = startAngle + step * (i + 1);
        float cos0 = std::cos(a0), sin0 = std::sin(a0);
        float cos1 = std::cos(a1), sin1 = std::sin(a1);

        float ix0 = centre.x + cos0 * innerR, iy0 = centre.y + sin0 * innerR;
        float ox0 = centre.x + cos0 * outerR, oy0 = centre.y + sin0 * outerR;
        float ix1 = centre.x + cos1 * innerR, iy1 = centre.y + sin1 * innerR;
        float ox1 = centre.x + cos1 * outerR, oy1 = centre.y + sin1 * outerR;

        addTriangle(ix0, iy0, ox0, oy0, ox1, oy1, c);
        addTriangle(ix0, iy0, ox1, oy1, ix1, iy1, c);
    }
}

void AtlasRenderer::drawProgressBar(const Rect& r, float fraction,
                                      const Color& fg, const Color& bg) {
    drawRect(r, bg);
    float fill = std::max(0.0f, std::min(1.0f, fraction));
    if (fill > 0.0f) {
        drawRect({r.x, r.y, r.w * fill, r.h}, fg);
    }
}

// ── Text rendering ──────────────────────────────────────────────────

float AtlasRenderer::drawText(const std::string& text, Vec2 pos,
                                const Color& c, float scale) {
    // Flush non-textured geometry first, then switch to textured mode
    flush();

    glUseProgram(m_shaderProgram);
    // Re-set projection (flush clears state)
    float L = 0.0f, R = static_cast<float>(m_windowW);
    float T = 0.0f, B = static_cast<float>(m_windowH);
    float proj[16] = {
        2.0f/(R-L),  0.0f,        0.0f, 0.0f,
        0.0f,        2.0f/(T-B),  0.0f, 0.0f,
        0.0f,        0.0f,       -1.0f, 0.0f,
        (R+L)/(L-R), (T+B)/(B-T), 0.0f, 1.0f,
    };
    glUniformMatrix4fv(m_uniformProj, 1, GL_FALSE, proj);
    glUniform1i(m_uniformUseTex, 1);
    glUniform1i(m_uniformTex, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);

    float atlasW = static_cast<float>(kFontCharCount * kFontGlyphW);
    float atlasH = static_cast<float>(kFontGlyphH);
    float gw = kFontGlyphW * scale;
    float gh = kFontGlyphH * scale;
    float cx = pos.x;

    for (char ch : text) {
        int idx = static_cast<int>(ch) - kFontFirstChar;
        if (idx < 0 || idx >= kFontCharCount) { cx += gw; continue; }

        float u0 = (idx * kFontGlyphW) / atlasW;
        float u1 = ((idx + 1) * kFontGlyphW) / atlasW;
        float v0 = 0.0f;
        float v1 = 1.0f;

        addQuad(cx, pos.y, cx + gw, pos.y + gh, u0, v0, u1, v1, c);
        cx += gw;
    }

    // Flush textured batch
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_vertices.size() * sizeof(UIVertex)),
                 m_vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Reset to non-textured mode for subsequent draws
    glUniform1i(m_uniformUseTex, 0);
    glUseProgram(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    m_vertices.clear();

    return cx - pos.x;
}

float AtlasRenderer::measureText(const std::string& text, float scale) const {
    return static_cast<float>(text.size()) * kFontGlyphW * scale;
}

// ── Scissor / clip ──────────────────────────────────────────────────

void AtlasRenderer::pushClip(const Rect& r) {
    flush();
    m_clipStack.push_back(r);
    glEnable(GL_SCISSOR_TEST);
    // OpenGL scissor has origin at bottom-left
    glScissor(static_cast<GLint>(r.x),
              static_cast<GLint>(m_windowH - r.bottom()),
              static_cast<GLsizei>(r.w),
              static_cast<GLsizei>(r.h));
}

void AtlasRenderer::popClip() {
    flush();
    if (!m_clipStack.empty()) m_clipStack.pop_back();
    if (m_clipStack.empty()) {
        glDisable(GL_SCISSOR_TEST);
    } else {
        const Rect& r = m_clipStack.back();
        glScissor(static_cast<GLint>(r.x),
                  static_cast<GLint>(m_windowH - r.bottom()),
                  static_cast<GLsizei>(r.w),
                  static_cast<GLsizei>(r.h));
    }
}

} // namespace atlas
