#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <string>
#include <vector>
#include <functional>

namespace UI {

// Notification severity / type
enum class NotificationType {
    Info,       // General information (blue)
    Success,    // Successful action (green)
    Warning,    // Warning / attention needed (amber)
    Danger,     // Critical / combat alert (red)
    Combat      // Combat-specific (red flash)
};

// Single notification entry
struct Notification {
    std::string id;
    std::string title;
    std::string message;
    NotificationType type = NotificationType::Info;
    float duration = 5.0f;      // seconds to display (0 = until dismissed)
    float elapsed = 0.0f;       // time shown so far
    bool dismissed = false;

    Notification() = default;
    Notification(const std::string& notif_id, const std::string& t, const std::string& msg,
                 NotificationType nt = NotificationType::Info, float dur = 5.0f)
        : id(notif_id), title(t), message(msg), type(nt), duration(dur) {}
};

class NotificationManager {
public:
    NotificationManager();
    ~NotificationManager() = default;

    // Render all active toast notifications (call each frame)
    void Render();

    // Add a new notification
    void AddNotification(const std::string& title, const std::string& message,
                         NotificationType type = NotificationType::Info,
                         float duration = 5.0f);

    // Convenience methods for common notification types
    void ShowInfo(const std::string& message);
    void ShowSuccess(const std::string& message);
    void ShowWarning(const std::string& message);
    void ShowDanger(const std::string& message);
    void ShowCombatAlert(const std::string& message);

    // Dismiss a specific notification
    void Dismiss(const std::string& id);

    // Dismiss all notifications
    void DismissAll();

    // Get active notification count
    int GetActiveCount() const;

    // Visibility (can hide all notifications)
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }

private:
    bool m_visible = true;
    std::vector<Notification> m_notifications;
    int m_nextId = 0;
    static constexpr int MAX_VISIBLE = 5;        // Max toast notifications on screen at once
    static constexpr float TOAST_WIDTH = 300.0f;
    static constexpr float TOAST_HEIGHT = 60.0f;
    static constexpr float TOAST_SPACING = 5.0f;
    static constexpr float TOAST_MARGIN_RIGHT = 10.0f;
    static constexpr float TOAST_MARGIN_TOP = 10.0f;

    void RenderToast(Notification& notif, int index);
    void CleanupExpired();
};

} // namespace UI

#endif // NOTIFICATION_MANAGER_H
