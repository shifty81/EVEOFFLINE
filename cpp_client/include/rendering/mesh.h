#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace atlas {

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
    
    /**
     * Draw with instancing
     * @param instanceCount Number of instances to draw
     */
    void drawInstanced(unsigned int instanceCount) const;
    
    /**
     * Get VAO for instanced rendering setup
     * Allows external setup of instance attribute pointers
     */
    unsigned int getVAO() const { return m_VAO; }
    
    /**
     * Get index count
     */
    size_t getIndexCount() const { return m_indices.size(); }

private:
    void setup();

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    unsigned int m_VAO, m_VBO, m_EBO;
};

} // namespace atlas
