#ifndef esp_now_connector_h
#define esp_now_connector_h

#include <Arduino.h>
#include <vector>
#include <esp_now.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>

typedef void (*DataReceivedCallbackFunction)(const uint8_t *sender, const uint8_t *data, int len);

typedef void (*DataReceivedJsonCallbackFunction)(const uint8_t *sender, JsonDocument data);

class EspNowConnection
{
public:
    /// @brief Initialize ESPNOW function
    /// @param privateMasterKey private master key for encryption (optional)
    void init(const uint8_t privateMasterKey[16] = nullptr);

    /// @brief Add a peer to peer list
    /// @param macAddr peer MAC address
    /// @param localMasterKey local master key for connection
    void addPeer(const uint8_t macAddr[6], const uint8_t localMasterKey[16] = nullptr);

    /// @brief Delete a peer from peer list
    /// @param macAddr peer MAC address
    void deletePeer(const uint8_t macAddr[6]);

    /// @brief Send ESPNOW data
    /// @param macAddr peer MAC address
    /// @param data data to send
    /// @param len length of data
    void send(const uint8_t macAddr[6], uint8_t *data, int len);

    /// @brief sen Json data with ESPNOW
    /// @param macAddr peer MAC address
    /// @param data Json data
    void sendJson(const uint8_t macAddr[6], JsonDocument data);

    /// @brief print MAC address
    /// @param macAddr MAC address
    static void printMacAddr(const uint8_t macAddr[6]);

    /// @brief Register callback function of receiving ESPNOW data
    /// @param dataReceivedCallback callback function of receiving ESPNOW data
    void registerDataReceivedCallback(DataReceivedCallbackFunction dataReceivedCallback);

    /// @brief Register callback function of receiving ESPNOW Json data
    /// @param dataReceivedJsonCallback callback function of receiving ESPNOW Json data
    void registerDataReceivedJsonCallback(DataReceivedJsonCallbackFunction dataReceivedJsonCallback);

private:
    /// @brief Callback function of sending ESPNOW data
    /// @param mac_addr 
    /// @param sendStatus 
    static void _onDataSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus);

    /// @brief Callback function of receiving ESPNOW data, calls all registered callback functions
    /// @param macAddr 
    /// @param data 
    /// @param len 
    static void _onDataReceived(const uint8_t *macAddr, const uint8_t *data, int len);

    /// @brief prints ESPNOW erros messages
    /// @param err error message
    static void _errorMessage(esp_err_t err);
};

#endif
