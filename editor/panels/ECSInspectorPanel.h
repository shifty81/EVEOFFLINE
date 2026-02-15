#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ecs/ECS.h"

namespace atlas::editor {

class ECSInspectorPanel : public EditorPanel {
public:
    explicit ECSInspectorPanel(ecs::World& world) : m_world(world) {}

    const char* Name() const override { return "ECS Inspector"; }
    void Draw() override;

private:
    ecs::World& m_world;
};

}
