#include "ui/layout_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdio>

// Minimal JSON serialization/deserialization without external dependencies.
// Uses simple string building for writing and basic parsing for reading.
// Sufficient for flat panel-layout objects.

namespace UI {

// ============================================================================
// Construction
// ============================================================================

LayoutManager::LayoutManager()
    : m_layoutDir("ui_layouts")
{}

// ============================================================================
// Directory configuration
// ============================================================================

void LayoutManager::SetLayoutDirectory(const std::string& dir) {
    m_layoutDir = dir;
    // Strip trailing slash for consistency
    while (!m_layoutDir.empty() && (m_layoutDir.back() == '/' || m_layoutDir.back() == '\\')) {
        m_layoutDir.pop_back();
    }
}

std::string LayoutManager::PresetPath(const std::string& name) const {
    return m_layoutDir + "/" + name + ".json";
}

// ============================================================================
// JSON Serialization (static, no file I/O)
// ============================================================================

std::string LayoutManager::SerializeToJson(
    const std::string& name,
    const std::unordered_map<std::string, PanelLayout>& panels)
{
    std::ostringstream os;
    os << "{\n";
    os << "  \"name\": \"" << name << "\",\n";
    os << "  \"panels\": [\n";

    size_t idx = 0;
    for (const auto& [id, pl] : panels) {
        os << "    {\n";
        os << "      \"id\": \"" << pl.id << "\",\n";

        // Use snprintf for portable float formatting
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.1f", static_cast<double>(pl.x));
        os << "      \"x\": " << buf << ",\n";
        std::snprintf(buf, sizeof(buf), "%.1f", static_cast<double>(pl.y));
        os << "      \"y\": " << buf << ",\n";
        std::snprintf(buf, sizeof(buf), "%.1f", static_cast<double>(pl.w));
        os << "      \"w\": " << buf << ",\n";
        std::snprintf(buf, sizeof(buf), "%.1f", static_cast<double>(pl.h));
        os << "      \"h\": " << buf << ",\n";
        os << "      \"visible\": " << (pl.visible ? "true" : "false") << ",\n";
        os << "      \"minimized\": " << (pl.minimized ? "true" : "false") << ",\n";
        std::snprintf(buf, sizeof(buf), "%.2f", static_cast<double>(pl.opacity));
        os << "      \"opacity\": " << buf << "\n";
        os << "    }";
        if (++idx < panels.size()) os << ",";
        os << "\n";
    }

    os << "  ]\n";
    os << "}\n";
    return os.str();
}

// ============================================================================
// Minimal JSON Parsing Helpers
// ============================================================================

namespace {

// Skip whitespace
void skipWS(const std::string& s, size_t& i) {
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
        ++i;
}

// Expect and consume a character
bool expect(const std::string& s, size_t& i, char c) {
    skipWS(s, i);
    if (i < s.size() && s[i] == c) { ++i; return true; }
    return false;
}

// Parse a JSON string value (already past the opening '"')
std::string parseString(const std::string& s, size_t& i) {
    skipWS(s, i);
    if (i >= s.size() || s[i] != '"') return "";
    ++i; // skip opening quote
    std::string result;
    while (i < s.size() && s[i] != '"') {
        if (s[i] == '\\' && i + 1 < s.size()) {
            ++i;
        }
        result += s[i++];
    }
    if (i < s.size()) ++i; // skip closing quote
    return result;
}

// Parse a number
float parseNumber(const std::string& s, size_t& i) {
    skipWS(s, i);
    size_t start = i;
    if (i < s.size() && s[i] == '-') ++i;
    while (i < s.size() && (std::isdigit(static_cast<unsigned char>(s[i])) || s[i] == '.')) ++i;
    return std::stof(s.substr(start, i - start));
}

// Parse a boolean
bool parseBool(const std::string& s, size_t& i) {
    skipWS(s, i);
    if (s.compare(i, 4, "true") == 0)  { i += 4; return true; }
    if (s.compare(i, 5, "false") == 0) { i += 5; return false; }
    return false;
}

} // anonymous namespace

// ============================================================================
// JSON Deserialization (static, no file I/O)
// ============================================================================

bool LayoutManager::DeserializeFromJson(
    const std::string& json,
    std::string& outName,
    std::unordered_map<std::string, PanelLayout>& outPanels)
{
    outPanels.clear();
    outName.clear();

    size_t i = 0;
    if (!expect(json, i, '{')) return false;

    // Parse top-level keys
    while (i < json.size()) {
        skipWS(json, i);
        if (i >= json.size() || json[i] == '}') break;

        std::string key = parseString(json, i);
        if (!expect(json, i, ':')) return false;

        if (key == "name") {
            outName = parseString(json, i);
        } else if (key == "panels") {
            if (!expect(json, i, '[')) return false;

            while (i < json.size()) {
                skipWS(json, i);
                if (i >= json.size() || json[i] == ']') break;

                if (!expect(json, i, '{')) return false;

                PanelLayout pl;
                // Parse panel object keys
                while (i < json.size()) {
                    skipWS(json, i);
                    if (i >= json.size() || json[i] == '}') break;

                    std::string pkey = parseString(json, i);
                    if (!expect(json, i, ':')) return false;

                    if (pkey == "id")        pl.id = parseString(json, i);
                    else if (pkey == "x")     pl.x = parseNumber(json, i);
                    else if (pkey == "y")     pl.y = parseNumber(json, i);
                    else if (pkey == "w")     pl.w = parseNumber(json, i);
                    else if (pkey == "h")     pl.h = parseNumber(json, i);
                    else if (pkey == "visible")   pl.visible = parseBool(json, i);
                    else if (pkey == "minimized") pl.minimized = parseBool(json, i);
                    else if (pkey == "opacity")   pl.opacity = parseNumber(json, i);

                    skipWS(json, i);
                    if (i < json.size() && json[i] == ',') ++i;
                }
                expect(json, i, '}'); // closing brace of panel object

                if (!pl.id.empty()) {
                    outPanels[pl.id] = pl;
                }

                skipWS(json, i);
                if (i < json.size() && json[i] == ',') ++i;
            }
            expect(json, i, ']'); // closing bracket of panels array
        }

        skipWS(json, i);
        if (i < json.size() && json[i] == ',') ++i;
    }

    return !outName.empty();
}

// ============================================================================
// File I/O
// ============================================================================

bool LayoutManager::SaveLayout(
    const std::string& name,
    const std::unordered_map<std::string, PanelLayout>& panels)
{
    std::string json = SerializeToJson(name, panels);
    std::string path = PresetPath(name);

    std::ofstream ofs(path);
    if (!ofs.is_open()) {
        std::cerr << "[LayoutManager] Failed to write layout: " << path << std::endl;
        return false;
    }
    ofs << json;
    ofs.close();
    std::cout << "[LayoutManager] Saved layout '" << name << "' to " << path << std::endl;
    return true;
}

bool LayoutManager::LoadLayout(
    const std::string& name,
    std::unordered_map<std::string, PanelLayout>& panels)
{
    std::string path = PresetPath(name);

    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "[LayoutManager] Layout not found: " << path << std::endl;
        return false;
    }

    std::ostringstream buf;
    buf << ifs.rdbuf();
    std::string json = buf.str();

    std::string loadedName;
    if (!DeserializeFromJson(json, loadedName, panels)) {
        std::cerr << "[LayoutManager] Failed to parse layout: " << path << std::endl;
        return false;
    }

    std::cout << "[LayoutManager] Loaded layout '" << loadedName << "' from " << path << std::endl;
    return true;
}

std::vector<std::string> LayoutManager::GetAvailablePresets() const {
    // Return the known built-in names; file-system scanning would require
    // platform-specific code.  For now, check which preset files exist.
    std::vector<std::string> names;
    const char* builtins[] = {"default", "combat", "mining", "custom"};
    for (const char* n : builtins) {
        std::ifstream f(PresetPath(n));
        if (f.is_open()) {
            names.emplace_back(n);
        }
    }
    return names;
}

bool LayoutManager::DeletePreset(const std::string& name) {
    std::string path = PresetPath(name);
    return std::remove(path.c_str()) == 0;
}

// ============================================================================
// Built-in Presets
// ============================================================================

void LayoutManager::CreateDefaultPresets(int windowW, int windowH) {
    float W = static_cast<float>(windowW);
    float H = static_cast<float>(windowH);

    // Helper lambda to build a PanelLayout
    auto pl = [](const std::string& id, float x, float y, float w, float h,
                 bool vis, float op = 0.92f) -> PanelLayout {
        PanelLayout p;
        p.id = id;
        p.x = x; p.y = y; p.w = w; p.h = h;
        p.visible = vis;
        p.opacity = op;
        return p;
    };

    // ---- Default layout ----
    {
        std::unordered_map<std::string, PanelLayout> panels;
        panels["overview"]      = pl("overview",      W - 390, 50,  380, 400, true);
        panels["inventory"]     = pl("inventory",     50,      300, 350, 400, false);
        panels["fitting"]       = pl("fitting",       420,     300, 400, 450, false);
        panels["mission"]       = pl("mission",       50,      50,  400, 350, false);
        panels["market"]        = pl("market",        420,     50,  450, 500, false);
        panels["dscan"]         = pl("dscan",         W - 360, 460, 350, 300, false);
        panels["chat"]          = pl("chat",          60,      420, 380, 300, false);
        panels["drones"]        = pl("drones",        60,      300, 320, 400, false);
        panels["probe_scanner"] = pl("probe_scanner", 420,     50,  400, 350, false);
        SaveLayout("default", panels);
    }

    // ---- Combat layout — overview + dscan visible, compact ----
    {
        std::unordered_map<std::string, PanelLayout> panels;
        panels["overview"]      = pl("overview",      W - 340, 40,  330, 360, true,  0.85f);
        panels["inventory"]     = pl("inventory",     50,      300, 350, 400, false);
        panels["fitting"]       = pl("fitting",       420,     300, 400, 450, false);
        panels["mission"]       = pl("mission",       50,      50,  400, 350, false);
        panels["market"]        = pl("market",        420,     50,  450, 500, false);
        panels["dscan"]         = pl("dscan",         W - 340, 410, 330, 250, true,  0.80f);
        panels["chat"]          = pl("chat",          60,      H - 220, 350, 200, true, 0.70f);
        panels["drones"]        = pl("drones",        60,      300, 280, 350, true,  0.80f);
        panels["probe_scanner"] = pl("probe_scanner", 420,     50,  400, 350, false);
        SaveLayout("combat", panels);
    }

    // ---- Mining layout — inventory visible, overview, drones ----
    {
        std::unordered_map<std::string, PanelLayout> panels;
        panels["overview"]      = pl("overview",      W - 390, 50,  380, 350, true);
        panels["inventory"]     = pl("inventory",     50,      50,  380, 450, true);
        panels["fitting"]       = pl("fitting",       420,     300, 400, 450, false);
        panels["mission"]       = pl("mission",       50,      50,  400, 350, false);
        panels["market"]        = pl("market",        420,     50,  450, 500, false);
        panels["dscan"]         = pl("dscan",         W - 360, 460, 350, 300, false);
        panels["chat"]          = pl("chat",          60,      520, 380, 200, true, 0.75f);
        panels["drones"]        = pl("drones",        50,      510, 320, 200, true, 0.85f);
        panels["probe_scanner"] = pl("probe_scanner", 420,     50,  400, 350, false);
        SaveLayout("mining", panels);
    }
}

} // namespace UI
