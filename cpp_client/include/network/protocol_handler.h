#pragma once

#include <string>
#include <functional>

namespace atlas {

/**
 * Protocol handler for game messages (JSON-based)
 * Compatible with Python server protocol
 */
class ProtocolHandler {
public:
    using MessageHandler = std::function<void(const std::string& type, const std::string& data)>;

    ProtocolHandler();

    /**
     * Parse incoming message
     */
    void handleMessage(const std::string& message);

    /**
     * Create outgoing message
     * @param type Message type
     * @param dataJson JSON string for data field (empty for no data)
     */
    std::string createMessage(const std::string& type, const std::string& dataJson);

    /**
     * Helper methods for common messages
     */
    std::string createConnectMessage(const std::string& playerId, const std::string& characterName);
    std::string createMoveMessage(float vx, float vy, float vz);
    std::string createChatMessage(const std::string& message);
    
    /**
     * Inventory management messages
     */
    std::string createInventoryTransferMessage(const std::string& itemId, int quantity, 
                                               bool fromCargo, bool toCargo);
    std::string createInventoryJettisonMessage(const std::string& itemId, int quantity);
    
    /**
     * Target lock/unlock messages
     */
    std::string createTargetLockMessage(const std::string& targetId);
    std::string createTargetUnlockMessage(const std::string& targetId);

    /**
     * Module fitting messages
     */
    std::string createModuleFitMessage(const std::string& moduleId, const std::string& slotType, int slotIndex);
    std::string createModuleUnfitMessage(const std::string& slotType, int slotIndex);
    std::string createModuleActivateMessage(int slotIndex, const std::string& targetId = "");
    
    /**
     * Market messages
     */
    std::string createMarketBuyMessage(const std::string& itemId, int quantity, double price);
    std::string createMarketSellMessage(const std::string& itemId, int quantity, double price);
    std::string createMarketQueryMessage(const std::string& itemId);
    
    /**
     * Station docking and repair messages
     */
    std::string createDockRequestMessage(const std::string& stationId);
    std::string createUndockRequestMessage();
    std::string createRepairRequestMessage();
    
    /**
     * Response message type helpers
     * These methods check if a message is a specific type of response
     */
    static bool isSuccessResponse(const std::string& type);
    static bool isErrorResponse(const std::string& type);
    static bool isInventoryResponse(const std::string& type);
    static bool isFittingResponse(const std::string& type);
    static bool isMarketResponse(const std::string& type);
    static bool isStationResponse(const std::string& type);

    /**
     * Set message handler
     */
    void setMessageHandler(MessageHandler handler) { m_messageHandler = handler; }

private:
    MessageHandler m_messageHandler;
};

} // namespace atlas
