#include "rendering/shader.h"
#include <GL/glew.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <climits>
#else
#include <unistd.h>
#include <climits>
#endif

namespace eve {

// Returns the directory containing the running executable.
static std::string getExecutableDir() {
    std::filesystem::path exePath;
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        exePath = buf;
    }
#elif defined(__APPLE__)
    char buf[PATH_MAX];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        exePath = std::filesystem::canonical(buf);
    }
#else
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        exePath = buf;
    }
#endif
    if (!exePath.empty()) {
        return exePath.parent_path().string();
    }
    return "";
}

// Resolve a file path by first checking as-is, then relative to the executable.
static std::string resolvePath(const std::string& filePath) {
    // First, try the path directly (works when CWD is correct)
    if (std::filesystem::exists(filePath)) {
        return filePath;
    }

    // Fall back to resolving relative to the executable directory
    std::string exeDir = getExecutableDir();
    if (!exeDir.empty()) {
        std::filesystem::path exeDirPath(exeDir);
        std::filesystem::path resolved = exeDirPath / filePath;
        if (std::filesystem::exists(resolved)) {
            return resolved.string();
        }

        // For multi-config generators (e.g., Visual Studio) the executable
        // may be in a configuration subdirectory such as bin/Release/ or
        // bin/Debug/ while resources are placed in the parent bin/ directory.
        // Check the parent directory as well.
        resolved = exeDirPath.parent_path() / filePath;
        if (std::filesystem::exists(resolved)) {
            return resolved.string();
        }
    }

    // Return original path (will produce the same "failed to open" error)
    return filePath;
}

Shader::Shader()
    : m_programID(0)
{
}

Shader::~Shader() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
    }
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "Failed to read shader files" << std::endl;
        return false;
    }
    
    return loadFromSource(vertexSource, fragmentSource);
}

bool Shader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader, fragmentShader;
    
    // Compile vertex shader
    if (!compileShader(vertexSource, GL_VERTEX_SHADER, vertexShader)) {
        std::cerr << "Vertex shader compilation failed" << std::endl;
        return false;
    }
    
    // Compile fragment shader
    if (!compileShader(fragmentSource, GL_FRAGMENT_SHADER, fragmentShader)) {
        std::cerr << "Fragment shader compilation failed" << std::endl;
        glDeleteShader(vertexShader);
        return false;
    }
    
    // Link program
    if (!linkProgram(vertexShader, fragmentShader)) {
        std::cerr << "Shader program linking failed" << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    std::cout << "Shader program created successfully (ID: " << m_programID << ")" << std::endl;
    return true;
}

void Shader::use() const {
    if (m_programID != 0) {
        glUseProgram(m_programID);
    }
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(m_programID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(m_programID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_programID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(m_programID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(m_programID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(m_programID, name.c_str()), 1, &value[0]);
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) const {
    glUniformMatrix3fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

bool Shader::compileShader(const std::string& source, unsigned int type, unsigned int& shaderId) {
    shaderId = glCreateShader(type);
    const char* sourceCStr = source.c_str();
    glShaderSource(shaderId, 1, &sourceCStr, nullptr);
    glCompileShader(shaderId);
    
    // Check compilation status
    int success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        int logLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> infoLog(logLength);
        glGetShaderInfoLog(shaderId, logLength, nullptr, infoLog.data());
        std::cerr << "Shader compilation error: " << infoLog.data() << std::endl;
        return false;
    }
    
    return true;
}

bool Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);
    
    // Check linking status
    int success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        int logLength;
        glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> infoLog(logLength);
        glGetProgramInfoLog(m_programID, logLength, nullptr, infoLog.data());
        std::cerr << "Shader program linking error: " << infoLog.data() << std::endl;
        m_programID = 0;
        return false;
    }
    
    return true;
}

std::string Shader::readFile(const std::string& filePath) {
    std::string resolvedFilePath = resolvePath(filePath);
    std::ifstream file(resolvedFilePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath;
        if (resolvedFilePath != filePath) {
            std::cerr << " (also tried: " << resolvedFilePath << ")";
        }
        std::cerr << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace eve
