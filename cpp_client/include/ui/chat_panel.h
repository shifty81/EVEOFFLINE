#ifndef CHAT_PANEL_H
#define CHAT_PANEL_H

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace UI {

// Chat message entry
struct ChatMessage {
    std::string sender_name;
    std::string content;
    std::string timestamp;  // e.g. "12:34"
    bool is_system_message = false;

    // Sender relation coloring
    enum class SenderType { Self, Other, System, Hostile, Friendly };
    SenderType sender_type = SenderType::Other;

    ChatMessage() = default;
    ChatMessage(const std::string& sender, const std::string& msg,
                const std::string& time, SenderType type = SenderType::Other,
                bool sys = false)
        : sender_name(sender), content(msg), timestamp(time),
          is_system_message(sys), sender_type(type) {}
};

// Chat channel info
struct ChatChannelInfo {
    std::string channel_id;
    std::string channel_name;
    std::string channel_type;  // "local", "corp", "fleet", "private"
    int member_count = 0;

    ChatChannelInfo() = default;
    ChatChannelInfo(const std::string& id, const std::string& name,
                    const std::string& type, int members = 0)
        : channel_id(id), channel_name(name), channel_type(type),
          member_count(members) {}
};

// Callback types
using SendMessageCallback = std::function<void(const std::string& channel_id, const std::string& message)>;
using SwitchChannelCallback = std::function<void(const std::string& channel_id)>;

class ChatPanel {
public:
    ChatPanel();
    ~ChatPanel() = default;

    // Render the chat panel
    void Render();

    // Render just the panel contents (no Begin/End) — used by docking manager
    void RenderContents();

    // Set channel list
    void SetChannels(const std::vector<ChatChannelInfo>& channels);

    // Add a message to the specified channel
    void AddMessage(const std::string& channel_id, const ChatMessage& message);

    // Set active channel
    void SetActiveChannel(const std::string& channel_id);

    // Update member count for a channel
    void UpdateMemberCount(const std::string& channel_id, int count);

    // Visibility
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    const std::vector<ChatChannelInfo>& GetChannels() const { return m_channels; }
    const std::map<std::string, std::vector<ChatMessage>>& GetAllMessages() const { return m_messages; }
    const std::string& GetActiveChannel() const { return m_activeChannelId; }

    // Callbacks
    void SetSendMessageCallback(SendMessageCallback callback) { m_onSendMessage = callback; }
    void SetSwitchChannelCallback(SwitchChannelCallback callback) { m_onSwitchChannel = callback; }

private:
    bool m_visible = true;

    // Channel data
    std::vector<ChatChannelInfo> m_channels;
    std::string m_activeChannelId;

    // Messages per channel
    std::map<std::string, std::vector<ChatMessage>> m_messages;
    static constexpr int MAX_MESSAGES_PER_CHANNEL = 200;

    // Input state
    char m_inputBuffer[512] = {};
    bool m_scrollToBottom = false;

    // Callbacks
    SendMessageCallback m_onSendMessage;
    SwitchChannelCallback m_onSwitchChannel;

    // Helper functions
    void RenderChannelTabs();
    void RenderMemberBar();
    void RenderMessageArea();
    void RenderInputBar();
    void SendCurrentMessage();
};

} // namespace UI

#endif // CHAT_PANEL_H
