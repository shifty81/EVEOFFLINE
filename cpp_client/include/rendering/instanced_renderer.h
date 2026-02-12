#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace atlas {

// Forward declarations
class Mesh;
class Shader;

/**
 * Instance data for a single entity
 * Contains transform and per-instance properties
 */
struct InstanceData {
    glm::mat4 transform;        // Model matrix (position, rotation, scale)
    glm::vec4 color;            // Instance color/tint
    float customFloat1;          // Custom data (e.g., health %)
    float customFloat2;          // Custom data (e.g., shield %)
    float _padding1;             // Padding for alignment
    float _padding2;             // Padding for alignment
    
    InstanceData()
        : transform(1.0f)
        , color(1.0f)
        , customFloat1(0.0f)
        , customFloat2(0.0f)
        , _padding1(0.0f)
        , _padding2(0.0f)
    {}
};

/**
 * Batch of instances sharing the same mesh
 * Manages instance buffer and rendering
 */
class InstanceBatch {
public:
    InstanceBatch(std::shared_ptr<Mesh> mesh, unsigned int maxInstances = 1000);
    ~InstanceBatch();
    
    // Prevent copying
    InstanceBatch(const InstanceBatch&) = delete;
    InstanceBatch& operator=(const InstanceBatch&) = delete;
    
    /**
     * Add an instance to this batch
     * @param data Instance data
     * @return Instance index in this batch, or -1 if batch is full
     */
    int addInstance(const InstanceData& data);
    
    /**
     * Update an existing instance
     * @param index Instance index
     * @param data New instance data
     * @return true if successful
     */
    bool updateInstance(unsigned int index, const InstanceData& data);
    
    /**
     * Remove an instance from this batch
     * @param index Instance index to remove
     */
    void removeInstance(unsigned int index);
    
    /**
     * Clear all instances
     */
    void clear();
    
    /**
     * Upload instance data to GPU
     * Call this before rendering if instances have changed
     */
    void updateGPUBuffer();
    
    /**
     * Render all instances in this batch
     * @param shader Shader to use for rendering
     */
    void render(Shader* shader);
    
    /**
     * Get number of active instances
     */
    unsigned int getInstanceCount() const { return static_cast<unsigned int>(m_instances.size()); }
    
    /**
     * Check if batch has room for more instances
     */
    bool isFull() const { return m_instances.size() >= m_maxInstances; }
    
    /**
     * Get max instances capacity
     */
    unsigned int getMaxInstances() const { return m_maxInstances; }
    
    /**
     * Get the mesh used by this batch
     */
    std::shared_ptr<Mesh> getMesh() const { return m_mesh; }

private:
    std::shared_ptr<Mesh> m_mesh;
    unsigned int m_maxInstances;
    std::vector<InstanceData> m_instances;
    
    // OpenGL buffers
    unsigned int m_instanceVBO;  // Instance data buffer
    bool m_bufferDirty;          // True if buffer needs update
    
    void setupInstanceBuffer();
    void cleanupBuffers();
};

/**
 * Instanced Renderer for efficient batch rendering
 * Manages multiple batches of instanced geometry
 */
class InstancedRenderer {
public:
    InstancedRenderer();
    ~InstancedRenderer();
    
    // Prevent copying
    InstancedRenderer(const InstancedRenderer&) = delete;
    InstancedRenderer& operator=(const InstancedRenderer&) = delete;
    
    /**
     * Register a mesh for instanced rendering
     * @param meshId Unique identifier for this mesh
     * @param mesh Mesh to instance
     * @param maxInstances Maximum instances for this mesh type
     * @return true if successful
     */
    bool registerMesh(const std::string& meshId, std::shared_ptr<Mesh> mesh, unsigned int maxInstances = 1000);
    
    /**
     * Unregister a mesh and remove all its instances
     * @param meshId Mesh identifier
     */
    void unregisterMesh(const std::string& meshId);
    
    /**
     * Add an instance of a registered mesh
     * @param meshId Mesh identifier
     * @param data Instance data
     * @return Instance ID for this instance, or -1 on failure
     */
    int addInstance(const std::string& meshId, const InstanceData& data);
    
    /**
     * Update an existing instance
     * @param instanceId Instance ID returned from addInstance
     * @param data New instance data
     * @return true if successful
     */
    bool updateInstance(int instanceId, const InstanceData& data);
    
    /**
     * Remove an instance
     * @param instanceId Instance ID to remove
     */
    void removeInstance(int instanceId);
    
    /**
     * Update all GPU buffers
     * Call this before rendering if any instances have changed
     */
    void updateBuffers();
    
    /**
     * Render all instances
     * @param shader Shader to use for rendering
     */
    void renderAll(Shader* shader);
    
    /**
     * Render instances of a specific mesh
     * @param meshId Mesh identifier
     * @param shader Shader to use
     */
    void renderMesh(const std::string& meshId, Shader* shader);
    
    /**
     * Clear all instances (keeps mesh registrations)
     */
    void clearInstances();
    
    /**
     * Clear everything (instances and mesh registrations)
     */
    void clearAll();
    
    /**
     * Get statistics
     */
    struct Stats {
        unsigned int totalMeshes = 0;
        unsigned int totalInstances = 0;
        unsigned int totalBatches = 0;
        unsigned int drawCalls = 0;
        
        void reset() {
            totalMeshes = 0;
            totalInstances = 0;
            totalBatches = 0;
            drawCalls = 0;
        }
    };
    
    const Stats& getStats() const { return m_stats; }
    void resetStats() { m_stats.reset(); }

private:
    // Map mesh ID to batch
    std::unordered_map<std::string, std::unique_ptr<InstanceBatch>> m_batches;
    
    // Map instance ID to (mesh ID, batch index)
    struct InstanceLocation {
        std::string meshId;
        unsigned int batchIndex;
    };
    std::unordered_map<int, InstanceLocation> m_instanceLocations;
    
    int m_nextInstanceId;
    Stats m_stats;
    
    int generateInstanceId() { return m_nextInstanceId++; }
};

} // namespace atlas
