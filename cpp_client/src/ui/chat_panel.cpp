#include "ui/chat_panel.h"
#include "ui/ui_manager.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>

namespace UI {

namespace {

ImVec4 GetSenderColor(ChatMessage::SenderType type) {
    switch (type) {
        case ChatMessage::SenderType::Self:
            return ImVec4(EVEColors::ACCENT_SECONDARY[0], EVEColors::ACCENT_SECONDARY[1],
                          EVEColors::ACCENT_SECONDARY[2], 1.0f);
        case ChatMessage::SenderType::System:
            return ImVec4(EVEColors::WARNING[0], EVEColors::WARNING[1],
                          EVEColors::WARNING[2], 1.0f);
        case ChatMessage::SenderType::Hostile:
            return ImVec4(EVEColors::TARGET_HOSTILE[0], EVEColors::TARGET_HOSTILE[1],
                          EVEColors::TARGET_HOSTILE[2], 1.0f);
        case ChatMessage::SenderType::Friendly:
            return ImVec4(EVEColors::TARGET_FRIENDLY[0], EVEColors::TARGET_FRIENDLY[1],
                          EVEColors::TARGET_FRIENDLY[2], 1.0f);
        case ChatMessage::SenderType::Other:
        default:
            return ImVec4(EVEColors::TEXT_PRIMARY[0], EVEColors::TEXT_PRIMARY[1],
                          EVEColors::TEXT_PRIMARY[2], 1.0f);
    }
}

} // anonymous namespace

ChatPanel::ChatPanel()
    : m_visible(false)
    , m_scrollToBottom(false)
{
    memset(m_inputBuffer, 0, sizeof(m_inputBuffer));

    // Default channels
    m_channels.push_back(ChatChannelInfo("local", "Local", "local", 1));
    m_channels.push_back(ChatChannelInfo("corp", "Corp", "corp", 0));
    m_channels.push_back(ChatChannelInfo("fleet", "Fleet", "fleet", 0));
    m_activeChannelId = "local";

    // Initial system message
    AddMessage("local", ChatMessage("System", "Channel joined.",
               "00:00", ChatMessage::SenderType::System, true));
}

void ChatPanel::Render() {
    if (!m_visible) return;

    ImGui::SetNextWindowSize(ImVec2(380, 300), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(60, 420), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (!ImGui::Begin("Chat", &m_visible, flags)) {
        ImGui::End();
        return;
    }

    RenderContents();
    ImGui::End();
}

void ChatPanel::RenderContents() {
    RenderChannelTabs();
    RenderMemberBar();
    RenderMessageArea();
    RenderInputBar();
}

void ChatPanel::SetChannels(const std::vector<ChatChannelInfo>& channels) {
    m_channels = channels;
    if (!m_channels.empty() && m_activeChannelId.empty()) {
        m_activeChannelId = m_channels[0].channel_id;
    }
}

void ChatPanel::AddMessage(const std::string& channel_id, const ChatMessage& message) {
    auto& msgs = m_messages[channel_id];
    msgs.push_back(message);
    if (static_cast<int>(msgs.size()) > MAX_MESSAGES_PER_CHANNEL) {
        msgs.erase(msgs.begin());
    }
    if (channel_id == m_activeChannelId) {
        m_scrollToBottom = true;
    }
}

void ChatPanel::SetActiveChannel(const std::string& channel_id) {
    m_activeChannelId = channel_id;
    m_scrollToBottom = true;
}

void ChatPanel::UpdateMemberCount(const std::string& channel_id, int count) {
    for (auto& ch : m_channels) {
        if (ch.channel_id == channel_id) {
            ch.member_count = count;
            break;
        }
    }
}

void ChatPanel::RenderChannelTabs() {
    for (size_t i = 0; i < m_channels.size(); ++i) {
        const auto& ch = m_channels[i];
        bool isActive = (ch.channel_id == m_activeChannelId);

        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(
                EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
                EVEColors::ACCENT_DIM[2], 0.8f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(
                EVEColors::BG_HEADER[0], EVEColors::BG_HEADER[1],
                EVEColors::BG_HEADER[2], 0.8f));
        }

        if (ImGui::Button(ch.channel_name.c_str(), ImVec2(80, 24))) {
            m_activeChannelId = ch.channel_id;
            m_scrollToBottom = true;
            if (m_onSwitchChannel) {
                m_onSwitchChannel(ch.channel_id);
            }
        }
        ImGui::PopStyleColor();

        if (i + 1 < m_channels.size()) {
            ImGui::SameLine();
        }
    }
}

void ChatPanel::RenderMemberBar() {
    int memberCount = 0;
    for (const auto& ch : m_channels) {
        if (ch.channel_id == m_activeChannelId) {
            memberCount = ch.member_count;
            break;
        }
    }

    ImGui::TextColored(ImVec4(
        EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
        EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
        "Channel members: %d", memberCount);
    ImGui::Separator();
}

void ChatPanel::RenderMessageArea() {
    // Reserve space for input bar at bottom
    float inputHeight = 30.0f;
    float availableHeight = ImGui::GetContentRegionAvail().y - inputHeight - 8.0f;
    if (availableHeight < 50.0f) availableHeight = 50.0f;

    ImGui::BeginChild("ChatMessages", ImVec2(0, availableHeight), true,
                      ImGuiWindowFlags_HorizontalScrollbar);

    auto it = m_messages.find(m_activeChannelId);
    if (it != m_messages.end()) {
        for (const auto& msg : it->second) {
            // Timestamp
            ImGui::TextColored(ImVec4(
                EVEColors::TEXT_DISABLED[0], EVEColors::TEXT_DISABLED[1],
                EVEColors::TEXT_DISABLED[2], 1.0f),
                "%s", msg.timestamp.c_str());
            ImGui::SameLine();

            // Sender name with color
            ImVec4 senderColor = GetSenderColor(msg.sender_type);
            ImGui::TextColored(senderColor, "%s:", msg.sender_name.c_str());
            ImGui::SameLine();

            // Message content
            ImGui::TextColored(ImVec4(
                EVEColors::TEXT_PRIMARY[0], EVEColors::TEXT_PRIMARY[1],
                EVEColors::TEXT_PRIMARY[2], 0.9f),
                "%s", msg.content.c_str());
        }
    }

    if (m_scrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        m_scrollToBottom = false;
    }

    ImGui::EndChild();
}

void ChatPanel::RenderInputBar() {
    ImGui::Spacing();

    // Input text field with send on Enter
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 60.0f);
    bool enterPressed = ImGui::InputText("##ChatInput", m_inputBuffer,
                                          sizeof(m_inputBuffer),
                                          ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();

    bool sendClicked = ImGui::Button("Send", ImVec2(55, 0));

    if (enterPressed || sendClicked) {
        SendCurrentMessage();
        // Re-focus the input text field (previous widget) after sending
        ImGui::SetKeyboardFocusHere(-1);
    }
}

void ChatPanel::SendCurrentMessage() {
    if (strlen(m_inputBuffer) == 0) return;

    std::string message(m_inputBuffer);

    // Fire callback
    if (m_onSendMessage) {
        m_onSendMessage(m_activeChannelId, message);
    }

    // Add message locally as self
    AddMessage(m_activeChannelId, ChatMessage("You", message, "",
               ChatMessage::SenderType::Self));

    // Clear input
    memset(m_inputBuffer, 0, sizeof(m_inputBuffer));
}

} // namespace UI
