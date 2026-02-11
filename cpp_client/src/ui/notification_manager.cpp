#include "ui/notification_manager.h"
#include "ui/ui_manager.h"
#include <imgui.h>
#include <algorithm>
#include <string>

namespace UI {

NotificationManager::NotificationManager() = default;

void NotificationManager::Render() {
    if (!m_visible) return;

    CleanupExpired();

    ImGuiIO& io = ImGui::GetIO();
    int displayed = 0;

    for (size_t i = 0; i < m_notifications.size() && displayed < MAX_VISIBLE; ++i) {
        auto& notif = m_notifications[i];
        if (notif.dismissed) continue;

        // Update elapsed time
        notif.elapsed += io.DeltaTime;

        RenderToast(notif, displayed);
        displayed++;
    }
}

void NotificationManager::AddNotification(const std::string& title,
                                            const std::string& message,
                                            NotificationType type,
                                            float duration) {
    Notification notif;
    notif.id = "notif_" + std::to_string(m_nextId++);
    notif.title = title;
    notif.message = message;
    notif.type = type;
    notif.duration = duration;
    m_notifications.push_back(notif);
}

void NotificationManager::ShowInfo(const std::string& message) {
    AddNotification("Info", message, NotificationType::Info);
}

void NotificationManager::ShowSuccess(const std::string& message) {
    AddNotification("Success", message, NotificationType::Success);
}

void NotificationManager::ShowWarning(const std::string& message) {
    AddNotification("Warning", message, NotificationType::Warning);
}

void NotificationManager::ShowDanger(const std::string& message) {
    AddNotification("Alert", message, NotificationType::Danger, 8.0f);
}

void NotificationManager::ShowCombatAlert(const std::string& message) {
    AddNotification("Combat", message, NotificationType::Combat, 6.0f);
}

void NotificationManager::Dismiss(const std::string& id) {
    for (auto& n : m_notifications) {
        if (n.id == id) {
            n.dismissed = true;
            break;
        }
    }
}

void NotificationManager::DismissAll() {
    for (auto& n : m_notifications) {
        n.dismissed = true;
    }
}

int NotificationManager::GetActiveCount() const {
    int count = 0;
    for (const auto& n : m_notifications) {
        if (!n.dismissed && (n.duration <= 0.0f || n.elapsed < n.duration)) {
            count++;
        }
    }
    return count;
}

void NotificationManager::RenderToast(Notification& notif, int index) {
    ImGuiIO& io = ImGui::GetIO();

    // Position: top-right corner, stacked vertically
    float posX = io.DisplaySize.x - TOAST_WIDTH - TOAST_MARGIN_RIGHT;
    float posY = TOAST_MARGIN_TOP + static_cast<float>(index) * (TOAST_HEIGHT + TOAST_SPACING);

    // Fade out in the last second
    float alpha = 1.0f;
    if (notif.duration > 0.0f) {
        float remaining = notif.duration - notif.elapsed;
        if (remaining < 1.0f && remaining > 0.0f) {
            alpha = remaining;
        }
    }

    // Accent color based on type
    ImVec4 accentColor;
    switch (notif.type) {
        case NotificationType::Success:
            accentColor = ImVec4(EVEColors::SUCCESS[0], EVEColors::SUCCESS[1],
                                  EVEColors::SUCCESS[2], alpha);
            break;
        case NotificationType::Warning:
            accentColor = ImVec4(EVEColors::WARNING[0], EVEColors::WARNING[1],
                                  EVEColors::WARNING[2], alpha);
            break;
        case NotificationType::Danger:
        case NotificationType::Combat:
            accentColor = ImVec4(EVEColors::DANGER[0], EVEColors::DANGER[1],
                                  EVEColors::DANGER[2], alpha);
            break;
        case NotificationType::Info:
        default:
            accentColor = ImVec4(EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
                                  EVEColors::ACCENT_PRIMARY[2], alpha);
            break;
    }

    ImGui::SetNextWindowPos(ImVec2(posX, posY));
    ImGui::SetNextWindowSize(ImVec2(TOAST_WIDTH, TOAST_HEIGHT));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 6));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(
        EVEColors::BG_TOOLTIP[0], EVEColors::BG_TOOLTIP[1],
        EVEColors::BG_TOOLTIP[2], EVEColors::BG_TOOLTIP[3] * alpha));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(
        accentColor.x, accentColor.y, accentColor.z, 0.6f * alpha));

    std::string windowId = "##toast_" + notif.id;
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav;

    ImGui::Begin(windowId.c_str(), nullptr, flags);

    // Title line with color
    ImGui::TextColored(accentColor, "%s", notif.title.c_str());

    // Dismiss button
    ImGui::SameLine(TOAST_WIDTH - 35.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(
        EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
        EVEColors::TEXT_SECONDARY[2], alpha));
    std::string dismissId = "x##dismiss_" + notif.id;
    if (ImGui::SmallButton(dismissId.c_str())) {
        notif.dismissed = true;
    }
    ImGui::PopStyleColor();

    // Message
    ImGui::TextColored(ImVec4(
        EVEColors::TEXT_PRIMARY[0], EVEColors::TEXT_PRIMARY[1],
        EVEColors::TEXT_PRIMARY[2], 0.9f * alpha),
        "%s", notif.message.c_str());

    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

void NotificationManager::CleanupExpired() {
    m_notifications.erase(
        std::remove_if(m_notifications.begin(), m_notifications.end(),
            [](const Notification& n) {
                return n.dismissed ||
                       (n.duration > 0.0f && n.elapsed >= n.duration);
            }),
        m_notifications.end());
}

} // namespace UI
