#include "systems/chat_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <iostream>
#include <algorithm>

namespace eve {
namespace systems {

ChatSystem::ChatSystem(ecs::World* world) : System(world) {}

void ChatSystem::update(float delta_time) {
    for (auto* entity : world_->getAllEntities()) {
        auto* channel = entity->getComponent<components::ChatChannel>();
        if (!channel) continue;

        // Trim message history if exceeding max_history
        if (static_cast<int>(channel->messages.size()) > channel->max_history) {
            int excess = static_cast<int>(channel->messages.size()) - channel->max_history;
            channel->messages.erase(channel->messages.begin(),
                                    channel->messages.begin() + excess);
        }
    }
}

bool ChatSystem::joinChannel(const std::string& channel_entity_id,
                             const std::string& player_id,
                             const std::string& player_name) {
    auto* entity = world_->getEntity(channel_entity_id);
    if (!entity) return false;

    auto* channel = entity->getComponent<components::ChatChannel>();
    if (!channel) return false;

    // Check if already a member
    for (const auto& m : channel->members) {
        if (m.player_id == player_id) return false;
    }

    // Check max_members
    if (channel->max_members > 0 &&
        static_cast<int>(channel->members.size()) >= channel->max_members) {
        return false;
    }

    // Add member
    components::ChatChannel::ChannelMember member;
    member.player_id = player_id;
    member.player_name = player_name;
    member.role = "member";
    member.is_muted = false;
    channel->members.push_back(member);

    // Send system join message
    components::ChatChannel::ChatMessage msg;
    msg.message_id = "msg_" + std::to_string(++message_counter_);
    msg.sender_id = "system";
    msg.sender_name = "System";
    msg.content = player_name + " has joined the channel";
    msg.is_system_message = true;
    channel->messages.push_back(msg);

    return true;
}

bool ChatSystem::leaveChannel(const std::string& channel_entity_id,
                              const std::string& player_id) {
    auto* entity = world_->getEntity(channel_entity_id);
    if (!entity) return false;

    auto* channel = entity->getComponent<components::ChatChannel>();
    if (!channel) return false;

    // Find and remove member
    std::string player_name;
    auto it = std::find_if(channel->members.begin(), channel->members.end(),
                           [&](const components::ChatChannel::ChannelMember& m) {
                               return m.player_id == player_id;
                           });
    if (it == channel->members.end()) return false;

    player_name = it->player_name;
    channel->members.erase(it);

    // Send system leave message
    components::ChatChannel::ChatMessage msg;
    msg.message_id = "msg_" + std::to_string(++message_counter_);
    msg.sender_id = "system";
    msg.sender_name = "System";
    msg.content = player_name + " has left the channel";
    msg.is_system_message = true;
    channel->messages.push_back(msg);

    return true;
}

bool ChatSystem::sendMessage(const std::string& channel_entity_id,
                             const std::string& sender_id,
                             const std::string& sender_name,
                             const std::string& content) {
    auto* entity = world_->getEntity(channel_entity_id);
    if (!entity) return false;

    auto* channel = entity->getComponent<components::ChatChannel>();
    if (!channel) return false;

    // Verify sender is a member
    auto it = std::find_if(channel->members.begin(), channel->members.end(),
                           [&](const components::ChatChannel::ChannelMember& m) {
                               return m.player_id == sender_id;
                           });
    if (it == channel->members.end()) return false;

    // Check if muted
    if (it->is_muted) return false;

    // Create and add message
    components::ChatChannel::ChatMessage msg;
    msg.message_id = "msg_" + std::to_string(++message_counter_);
    msg.sender_id = sender_id;
    msg.sender_name = sender_name;
    msg.content = content;
    msg.is_system_message = false;
    channel->messages.push_back(msg);

    return true;
}

bool ChatSystem::mutePlayer(const std::string& channel_entity_id,
                            const std::string& moderator_id,
                            const std::string& target_id) {
    auto* entity = world_->getEntity(channel_entity_id);
    if (!entity) return false;

    auto* channel = entity->getComponent<components::ChatChannel>();
    if (!channel) return false;

    // Verify moderator has appropriate role
    auto mod_it = std::find_if(channel->members.begin(), channel->members.end(),
                               [&](const components::ChatChannel::ChannelMember& m) {
                                   return m.player_id == moderator_id;
                               });
    if (mod_it == channel->members.end()) return false;
    if (mod_it->role != "moderator" && mod_it->role != "operator" && mod_it->role != "owner")
        return false;

    // Find target and mute
    auto target_it = std::find_if(channel->members.begin(), channel->members.end(),
                                  [&](const components::ChatChannel::ChannelMember& m) {
                                      return m.player_id == target_id;
                                  });
    if (target_it == channel->members.end()) return false;

    target_it->is_muted = true;
    return true;
}

bool ChatSystem::unmutePlayer(const std::string& channel_entity_id,
                              const std::string& moderator_id,
                              const std::string& target_id) {
    auto* entity = world_->getEntity(channel_entity_id);
    if (!entity) return false;

    auto* channel = entity->getComponent<components::ChatChannel>();
    if (!channel) return false;

    // Verify moderator has appropriate role
    auto mod_it = std::find_if(channel->members.begin(), channel->members.end(),
                               [&](const components::ChatChannel::ChannelMember& m) {
                                   return m.player_id == moderator_id;
                               });
    if (mod_it == channel->members.end()) return false;
    if (mod_it->role != "moderator" && mod_it->role != "operator" && mod_it->role != "owner")
        return false;

    // Find target and unmute
    auto target_it = std::find_if(channel->members.begin(), channel->members.end(),
                                  [&](const components::ChatChannel::ChannelMember& m) {
                                      return m.player_id == target_id;
                                  });
    if (target_it == channel->members.end()) return false;

    target_it->is_muted = false;
    return true;
}

bool ChatSystem::setMotd(const std::string& channel_entity_id,
                         const std::string& setter_id,
                         const std::string& motd) {
    auto* entity = world_->getEntity(channel_entity_id);
    if (!entity) return false;

    auto* channel = entity->getComponent<components::ChatChannel>();
    if (!channel) return false;

    // Verify setter has operator or owner role
    auto it = std::find_if(channel->members.begin(), channel->members.end(),
                           [&](const components::ChatChannel::ChannelMember& m) {
                               return m.player_id == setter_id;
                           });
    if (it == channel->members.end()) return false;
    if (it->role != "operator" && it->role != "owner") return false;

    channel->motd = motd;
    return true;
}

int ChatSystem::getMessageCount(const std::string& channel_entity_id) {
    auto* entity = world_->getEntity(channel_entity_id);
    if (!entity) return 0;

    auto* channel = entity->getComponent<components::ChatChannel>();
    if (!channel) return 0;

    return static_cast<int>(channel->messages.size());
}

int ChatSystem::getMemberCount(const std::string& channel_entity_id) {
    auto* entity = world_->getEntity(channel_entity_id);
    if (!entity) return 0;

    auto* channel = entity->getComponent<components::ChatChannel>();
    if (!channel) return 0;

    return static_cast<int>(channel->members.size());
}

bool ChatSystem::isMember(const std::string& channel_entity_id,
                          const std::string& player_id) {
    auto* entity = world_->getEntity(channel_entity_id);
    if (!entity) return false;

    auto* channel = entity->getComponent<components::ChatChannel>();
    if (!channel) return false;

    for (const auto& m : channel->members) {
        if (m.player_id == player_id) return true;
    }
    return false;
}

} // namespace systems
} // namespace eve
