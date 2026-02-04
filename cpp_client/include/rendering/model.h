#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <glm/glm.hpp>

namespace eve {

class Mesh;

/**
 * Faction color scheme
 */
struct FactionColors {
    glm::vec4 primary;
    glm::vec4 secondary;
    glm::vec4 accent;
};

/**
 * 3D Model class
 */
class Model {
public:
    Model();
    ~Model();

    /**
     * Load model from file
     */
    bool loadFromFile(const std::string& path);

    /**
     * Create procedural ship model
     */
    static std::unique_ptr<Model> createShipModel(const std::string& shipType, const std::string& faction);

    /**
     * Draw the model
     */
    void draw() const;

    /**
     * Add a mesh to the model
     */
    void addMesh(std::unique_ptr<Mesh> mesh);

private:
    std::vector<std::unique_ptr<Mesh>> m_meshes;

    // Static helper methods for ship generation
    static bool isFrigate(const std::string& shipType);
    static bool isDestroyer(const std::string& shipType);
    static bool isCruiser(const std::string& shipType);
    static bool isTech2Cruiser(const std::string& shipType);
    static bool isBattlecruiser(const std::string& shipType);
    static bool isBattleship(const std::string& shipType);
    static bool isMiningBarge(const std::string& shipType);

    static FactionColors getFactionColors(const std::string& faction);
    
    static std::unique_ptr<Model> createFrigateModel(const FactionColors& colors);
    static std::unique_ptr<Model> createDestroyerModel(const FactionColors& colors);
    static std::unique_ptr<Model> createCruiserModel(const FactionColors& colors);
    static std::unique_ptr<Model> createTech2CruiserModel(const FactionColors& colors);
    static std::unique_ptr<Model> createBattlecruiserModel(const FactionColors& colors);
    static std::unique_ptr<Model> createBattleshipModel(const FactionColors& colors);
    static std::unique_ptr<Model> createMiningBargeModel(const FactionColors& colors);
    static std::unique_ptr<Model> createGenericModel(const FactionColors& colors);

    // Model cache
    static std::map<std::string, std::shared_ptr<Model>> s_modelCache;
};

} // namespace eve
