#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace eve {

/**
 * Mesh class - holds vertex data
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 color;
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    void draw() const;

private:
    void setup();

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    unsigned int m_VAO, m_VBO, m_EBO;
};

} // namespace eve
