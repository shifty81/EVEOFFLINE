#pragma once
#include "../ui/EditorPanel.h"
#include <string>

namespace atlas::editor {

enum class BuildTarget {
    Client,
    Server,
};

enum class BuildMode {
    Debug,
    Development,
    Release,
};

struct PackageSettings {
    BuildTarget target = BuildTarget::Client;
    BuildMode mode = BuildMode::Debug;
    bool singleExe = false;
    bool includeMods = false;
    bool stripEditorData = true;
    std::string outputPath = "./build/output";
};

class GamePackagerPanel : public EditorPanel {
public:
    const char* Name() const override { return "Game Packager"; }
    void Draw() override;

    const PackageSettings& Settings() const { return m_settings; }

private:
    PackageSettings m_settings;
};

}
