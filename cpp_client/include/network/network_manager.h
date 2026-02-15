#pragma once

#include "network/tcp_client.h"
#include "network/protocol_handler.h"
#include <functional>
#include <memory>
#include <map>
#include <string>

namespace atlas {

// Response callback types
struct InventoryResponse {
    bool success;
    std::string message;
    std::string itemId;
    int quantity;
};

struct FittingResponse {
    bool success;
    std::string message;
    std::string moduleId;
    std::string slotType;
    int slotIndex;
};

struct MarketResponse {
    bool success;
    std::string message;
    std::string itemId;
    int quantity;
    double price;
    double totalCost;
};

struct StationResponse {
    bool success;
    std::string message;
    std::string stationId;
    float repairCost;
    float shieldHp;
    float armorHp;
    float hullHp;
};

struct ScannerResponse {
    std::string scannerId;
    int anomaliesFound;
    std::string resultsJson;  // Raw JSON array of scan results
};

/**
 * High-level network manager
 * Combines TCP client and protocol handler for easy game integration
 */
class NetworkManager {
public:
    // Message handler for specific message types
    using TypedMessageHandler = std::function<void(const std::string& dataJson)>;
    
    // Callback types for gameplay operations
    using InventoryCallback = std::function<void(const InventoryResponse&)>;
    using FittingCallback = std::function<void(const FittingResponse&)>;
    using MarketCallback = std::function<void(const MarketResponse&)>;
    using StationCallback = std::function<void(const StationResponse&)>;
    using ScannerCallback = std::function<void(const ScannerResponse&)>;
    using ErrorCallback = std::function<void(const std::string& message)>;

    NetworkManager();
    ~NetworkManager();

    /**
     * Connect to game server
     * @param host Server hostname or IP
     * @param port Server port
     * @param playerId Player ID (can be generated)
     * @param characterName Character name
     * @return true if connection successful
     */
    bool connect(const std::string& host, int port, 
                 const std::string& playerId, const std::string& characterName);

    /**
     * Disconnect from server
     */
    void disconnect();

    /**
     * Check if connected
     */
    bool isConnected() const;

    /**
     * Update network (process messages)
     * Should be called every frame
     */
    void update();

    /**
     * Register handler for specific message type
     * @param type Message type (e.g., "STATE_UPDATE", "SPAWN_ENTITY")
     * @param handler Handler function
     */
    void registerHandler(const std::string& type, TypedMessageHandler handler);

    /**
     * Send movement input
     */
    void sendMove(float vx, float vy, float vz);

    /**
     * Send chat message
     */
    void sendChat(const std::string& message);
    
    /**
     * Inventory management
     */
    void sendInventoryTransfer(const std::string& itemId, int quantity, bool fromCargo, bool toCargo);
    void sendInventoryJettison(const std::string& itemId, int quantity);
    
    /**
     * Target lock/unlock
     */
    void sendTargetLock(const std::string& targetId);
    void sendTargetUnlock(const std::string& targetId);

    /**
     * Module fitting
     */
    void sendModuleFit(const std::string& moduleId, const std::string& slotType, int slotIndex);
    void sendModuleUnfit(const std::string& slotType, int slotIndex);
    void sendModuleActivate(int slotIndex, const std::string& targetId = "");
    
    /**
     * Market operations
     */
    void sendMarketBuy(const std::string& itemId, int quantity, double price);
    void sendMarketSell(const std::string& itemId, int quantity, double price);
    void sendMarketQuery(const std::string& itemId);
    
    /**
     * Station operations
     */
    void sendDockRequest(const std::string& stationId);
    void sendUndockRequest();
    void sendRepairRequest();

    /**
     * Scanner / Exploration operations
     */
    void sendScanStart(const std::string& systemId);
    void sendScanStop();
    void sendAnomalyListRequest(const std::string& systemId);
    
    /**
     * Set response callbacks for gameplay operations
     * These callbacks are invoked when the server responds to requests
     */
    void setInventoryCallback(InventoryCallback callback) { m_inventoryCallback = callback; }
    void setFittingCallback(FittingCallback callback) { m_fittingCallback = callback; }
    void setMarketCallback(MarketCallback callback) { m_marketCallback = callback; }
    void setStationCallback(StationCallback callback) { m_stationCallback = callback; }
    void setScannerCallback(ScannerCallback callback) { m_scannerCallback = callback; }
    void setErrorCallback(ErrorCallback callback) { m_errorCallback = callback; }

    /**
     * Get connection state string
     */
    std::string getConnectionState() const;

private:
    void onRawMessage(const std::string& message);
    void onProtocolMessage(const std::string& type, const std::string& dataJson);
    
    // Response handlers
    void handleInventoryResponse(const std::string& type, const std::string& dataJson);
    void handleFittingResponse(const std::string& type, const std::string& dataJson);
    void handleMarketResponse(const std::string& type, const std::string& dataJson);
    void handleStationResponse(const std::string& type, const std::string& dataJson);
    void handleScannerResponse(const std::string& type, const std::string& dataJson);
    void handleErrorResponse(const std::string& dataJson);

    std::unique_ptr<TCPClient> m_tcpClient;
    std::unique_ptr<ProtocolHandler> m_protocolHandler;
    
    // Message handlers by type
    std::map<std::string, TypedMessageHandler> m_handlers;
    
    // Response callbacks
    InventoryCallback m_inventoryCallback;
    FittingCallback m_fittingCallback;
    MarketCallback m_marketCallback;
    StationCallback m_stationCallback;
    ScannerCallback m_scannerCallback;
    ErrorCallback m_errorCallback;
    
    // Connection info
    std::string m_playerId;
    std::string m_characterName;
    bool m_authenticated;
    
    enum class State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        AUTHENTICATED
    };
    State m_state;
};

} // namespace atlas
