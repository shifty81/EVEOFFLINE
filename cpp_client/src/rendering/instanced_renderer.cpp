#include "rendering/instanced_renderer.h"
#include "rendering/mesh.h"
#include "rendering/shader.h"
#include <glad/glad.h>
#include <iostream>

namespace eve {

// ============================================================================
// InstanceBatch Implementation
// ============================================================================

InstanceBatch::InstanceBatch(std::shared_ptr<Mesh> mesh, unsigned int maxInstances)
    : m_mesh(mesh)
    , m_maxInstances(maxInstances)
    , m_instanceVBO(0)
    , m_bufferDirty(true)
{
    m_instances.reserve(maxInstances);
    setupInstanceBuffer();
}

InstanceBatch::~InstanceBatch() {
    cleanupBuffers();
}

void InstanceBatch::setupInstanceBuffer() {
    if (!m_mesh) return;
    
    // Create instance buffer
    glGenBuffers(1, &m_instanceVBO);
    
    // Bind mesh VAO to configure instance attributes
    unsigned int vao = m_mesh->getVAO();
    glBindVertexArray(vao);
    
    // Bind instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    
    // Allocate buffer (will be filled later)
    glBufferData(GL_ARRAY_BUFFER, m_maxInstances * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
    
    // Configure instance attributes
    // Transform matrix (4 vec4s = 16 floats)
    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 
                             (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(4 + i);
        glVertexAttribDivisor(4 + i, 1); // Advance once per instance
    }
    
    // Color (vec4)
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 
                         (void*)offsetof(InstanceData, color));
    glEnableVertexAttribArray(8);
    glVertexAttribDivisor(8, 1);
    
    // Custom floats
    glVertexAttribPointer(9, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 
                         (void*)offsetof(InstanceData, customFloat1));
    glEnableVertexAttribArray(9);
    glVertexAttribDivisor(9, 1);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstanceBatch::cleanupBuffers() {
    if (m_instanceVBO != 0) {
        glDeleteBuffers(1, &m_instanceVBO);
        m_instanceVBO = 0;
    }
}

int InstanceBatch::addInstance(const InstanceData& data) {
    if (isFull()) {
        return -1;
    }
    
    m_instances.push_back(data);
    m_bufferDirty = true;
    return static_cast<int>(m_instances.size() - 1);
}

bool InstanceBatch::updateInstance(unsigned int index, const InstanceData& data) {
    if (index >= m_instances.size()) {
        return false;
    }
    
    m_instances[index] = data;
    m_bufferDirty = true;
    return true;
}

void InstanceBatch::removeInstance(unsigned int index) {
    if (index >= m_instances.size()) {
        return;
    }
    
    // Swap with last and pop
    if (index != m_instances.size() - 1) {
        m_instances[index] = m_instances.back();
    }
    m_instances.pop_back();
    m_bufferDirty = true;
}

void InstanceBatch::clear() {
    m_instances.clear();
    m_bufferDirty = true;
}

void InstanceBatch::updateGPUBuffer() {
    if (!m_bufferDirty || m_instances.empty()) {
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_instances.size() * sizeof(InstanceData), m_instances.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    m_bufferDirty = false;
}

void InstanceBatch::render(Shader* shader) {
    if (m_instances.empty() || !m_mesh) {
        return;
    }
    
    // Update GPU buffer if dirty
    updateGPUBuffer();
    
    // Draw instanced
    m_mesh->drawInstanced(static_cast<unsigned int>(m_instances.size()));
}

// ============================================================================
// InstancedRenderer Implementation
// ============================================================================

InstancedRenderer::InstancedRenderer()
    : m_nextInstanceId(1)
{
    m_stats.reset();
}

InstancedRenderer::~InstancedRenderer() {
    clearAll();
}

bool InstancedRenderer::registerMesh(const std::string& meshId, std::shared_ptr<Mesh> mesh, unsigned int maxInstances) {
    if (m_batches.find(meshId) != m_batches.end()) {
        std::cerr << "[InstancedRenderer] Mesh ID already registered: " << meshId << std::endl;
        return false;
    }
    
    if (!mesh) {
        std::cerr << "[InstancedRenderer] Cannot register null mesh" << std::endl;
        return false;
    }
    
    m_batches[meshId] = std::make_unique<InstanceBatch>(mesh, maxInstances);
    m_stats.totalMeshes++;
    m_stats.totalBatches++;
    
    return true;
}

void InstancedRenderer::unregisterMesh(const std::string& meshId) {
    auto it = m_batches.find(meshId);
    if (it != m_batches.end()) {
        // Remove all instances of this mesh
        auto instanceIt = m_instanceLocations.begin();
        while (instanceIt != m_instanceLocations.end()) {
            if (instanceIt->second.meshId == meshId) {
                instanceIt = m_instanceLocations.erase(instanceIt);
            } else {
                ++instanceIt;
            }
        }
        
        m_batches.erase(it);
        m_stats.totalMeshes--;
        m_stats.totalBatches--;
    }
}

int InstancedRenderer::addInstance(const std::string& meshId, const InstanceData& data) {
    auto it = m_batches.find(meshId);
    if (it == m_batches.end()) {
        std::cerr << "[InstancedRenderer] Mesh not registered: " << meshId << std::endl;
        return -1;
    }
    
    int batchIndex = it->second->addInstance(data);
    if (batchIndex < 0) {
        std::cerr << "[InstancedRenderer] Batch full for mesh: " << meshId << std::endl;
        return -1;
    }
    
    int instanceId = generateInstanceId();
    m_instanceLocations[instanceId] = { meshId, static_cast<unsigned int>(batchIndex) };
    m_stats.totalInstances++;
    
    return instanceId;
}

bool InstancedRenderer::updateInstance(int instanceId, const InstanceData& data) {
    auto it = m_instanceLocations.find(instanceId);
    if (it == m_instanceLocations.end()) {
        return false;
    }
    
    const InstanceLocation& loc = it->second;
    auto batchIt = m_batches.find(loc.meshId);
    if (batchIt == m_batches.end()) {
        return false;
    }
    
    return batchIt->second->updateInstance(loc.batchIndex, data);
}

void InstancedRenderer::removeInstance(int instanceId) {
    auto it = m_instanceLocations.find(instanceId);
    if (it == m_instanceLocations.end()) {
        return;
    }
    
    const InstanceLocation& loc = it->second;
    auto batchIt = m_batches.find(loc.meshId);
    if (batchIt != m_batches.end()) {
        batchIt->second->removeInstance(loc.batchIndex);
        m_stats.totalInstances--;
    }
    
    m_instanceLocations.erase(it);
}

void InstancedRenderer::updateBuffers() {
    for (auto& pair : m_batches) {
        pair.second->updateGPUBuffer();
    }
}

void InstancedRenderer::renderAll(Shader* shader) {
    if (!shader) return;
    
    m_stats.drawCalls = 0;
    
    for (auto& pair : m_batches) {
        if (pair.second->getInstanceCount() > 0) {
            pair.second->render(shader);
            m_stats.drawCalls++;
        }
    }
}

void InstancedRenderer::renderMesh(const std::string& meshId, Shader* shader) {
    if (!shader) return;
    
    auto it = m_batches.find(meshId);
    if (it != m_batches.end() && it->second->getInstanceCount() > 0) {
        it->second->render(shader);
    }
}

void InstancedRenderer::clearInstances() {
    for (auto& pair : m_batches) {
        pair.second->clear();
    }
    m_instanceLocations.clear();
    m_stats.totalInstances = 0;
}

void InstancedRenderer::clearAll() {
    m_batches.clear();
    m_instanceLocations.clear();
    m_stats.reset();
}

} // namespace eve
