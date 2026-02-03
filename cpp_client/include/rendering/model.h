#pragma once

#include <string>
#include <vector>
#include <memory>

namespace eve {

class Mesh;

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

private:
    std::vector<std::unique_ptr<Mesh>> m_meshes;
};

} // namespace eve
