#pragma once

#include <string>
#include <glm/glm.hpp>

namespace eve {

/**
 * Shader program management
 */
class Shader {
public:
    Shader();
    ~Shader();

    /**
     * Load and compile shader from files
     */
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * Load and compile shader from source code
     */
    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);

    /**
     * Use this shader program
     */
    void use() const;

    /**
     * Utility functions for setting uniforms
     */
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat3(const std::string& name, const glm::mat3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

    /**
     * Get shader program ID
     */
    unsigned int getID() const { return m_programID; }

    /**
     * Check if shader is valid
     */
    bool isValid() const { return m_programID != 0; }

private:
    bool compileShader(const std::string& source, unsigned int type, unsigned int& shaderId);
    bool linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    std::string readFile(const std::string& filePath);

    unsigned int m_programID;
};

} // namespace eve
