#include "rendering/model.h"
#include "rendering/mesh.h"
#include <iostream>

namespace eve {

Model::Model() {
}

Model::~Model() {
}

bool Model::loadFromFile(const std::string& path) {
    std::cout << "Model loading from file not yet implemented: " << path << std::endl;
    return false;
}

std::unique_ptr<Model> Model::createShipModel(const std::string& shipType, const std::string& faction) {
    std::cout << "Procedural ship model generation not yet implemented" << std::endl;
    return std::make_unique<Model>();
}

void Model::draw() const {
    for (const auto& mesh : m_meshes) {
        mesh->draw();
    }
}

} // namespace eve
