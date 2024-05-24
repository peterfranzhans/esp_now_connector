#include "esp_now_connector.h"
#include "WiFi.h"
#include <esp_wifi.h>

std::vector<esp_now_peer_info_t> peerList;

static std::vector<DataReceivedCallbackFunction> dataReceivedCallbacks;
static std::vector<DataReceivedJsonCallbackFunction> dataReceivedJsonCallbacks;

static bool encryptionPossible = false;

void EspNowConnection::init(const uint8_t privateMasterKey[16])
{
    if (!WiFi.isConnected()){ //initialize wifi mode if there's no connection
        WiFi.mode(WIFI_STA);
    }

    if (esp_now_init() != 0) //initialize esp now
    {
        Serial.println("ESP-NOW initialization failed!");
        ESP.restart();
    }

    if(privateMasterKey){ //if private master key is given
        encryptionPossible = true;
        esp_now_set_pmk(privateMasterKey); //set private master key for encryption
    }

    //register callback functions
    esp_now_register_send_cb(_onDataSent);
    esp_now_register_recv_cb(_onDataReceived);
}

void EspNowConnection::addPeer(const uint8_t macAddr[6], const uint8_t localMasterKey[16])
{
    esp_now_peer_info_t newPeer;

    //set mac address of new peer
    memset(&newPeer, 0, sizeof(newPeer));
    memcpy(newPeer.peer_addr, macAddr, 6);

    //if a local master key is given, the connection will be encrypted
    if (encryptionPossible && localMasterKey)
    {
        memcpy(newPeer.lmk, localMasterKey, 16);
        newPeer.encrypt = true;
    }

    esp_err_t result = esp_now_add_peer(&newPeer); //Add new Peer

    if (result != ESP_OK)
    {
        printMacAddr(newPeer.peer_addr);
        Serial.println(" - Failed to add new Peer!");
        _errorMessage(result);
    }
    else
    {
        printMacAddr(newPeer.peer_addr);
        if (encryptionPossible && localMasterKey)
            Serial.println(" - New Peer added successfully! (encrypted connection)");
        else
            Serial.println(" - New Peer added successfully!");
    }
}

void EspNowConnection::deletePeer(const uint8_t macAddr[6])
{
    esp_err_t result = esp_now_del_peer(macAddr); //delete peer

    if(result != ESP_OK){
        _errorMessage(result);
    }
}

void EspNowConnection::send(const uint8_t macAddr[6], uint8_t *data, int len)
{
    esp_err_t result = esp_now_send(macAddr, data, len); //send data

    if(result != ESP_OK){
        _errorMessage(result);
    }
}

void EspNowConnection::sendJson(const uint8_t macAddr[6], JsonDocument data)
{
    //serialize Json
    size_t jsonSize = measureJson(data);
    uint8_t jsonData[jsonSize];
    serializeJson(data, jsonData, jsonSize);
    send(macAddr, jsonData, jsonSize); //send Json
}

void EspNowConnection::printMacAddr(const uint8_t macAddr[6])
{
    for (int i = 0; i < 6; i++)
    {
        Serial.print(macAddr[i], HEX);
        if (i != 5)
            Serial.print(":");
    }
}

void EspNowConnection::registerDataReceivedCallback(DataReceivedCallbackFunction dataReceivedCallback)
{
    dataReceivedCallbacks.push_back(dataReceivedCallback);
}

void EspNowConnection::registerDataReceivedJsonCallback(DataReceivedJsonCallbackFunction dataReceivedJsonCallback)
{
    dataReceivedJsonCallbacks.push_back(dataReceivedJsonCallback);
}

void EspNowConnection::_onDataSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus)
{
    if (!sendStatus)
    {
        EspNowConnection::printMacAddr(mac_addr);
        Serial.println(" - Data sent successfully!");
    }
    else
    {
        EspNowConnection::printMacAddr(mac_addr);
        Serial.println(" - Sending the data failed!");
    }
}

void EspNowConnection::_onDataReceived(const uint8_t *macAddr, const uint8_t *data, int len)
{
    //call all regsitered callback functions

    for (DataReceivedCallbackFunction dataReceivedCallback : dataReceivedCallbacks)
    {
        dataReceivedCallback(macAddr, data, len);
    }

    JsonDocument receivedData;
    deserializeJson(receivedData, data, len);
    for (DataReceivedJsonCallbackFunction dataReceivedJsonCallback : dataReceivedJsonCallbacks)
    {
        dataReceivedJsonCallback(macAddr, receivedData);
    }
}

void EspNowConnection::_errorMessage(esp_err_t err)
{
    switch (err)
    {
    case ESP_ERR_ESPNOW_NOT_INIT:
        Serial.println("ERROR: ESPNOW is not initialized");
        break;
    case ESP_ERR_ESPNOW_ARG:
        Serial.println("ERROR: Invalid argument");
        break;
    case ESP_ERR_ESPNOW_NO_MEM:
        Serial.println("ERROR: Out of memory");
        break;
    case ESP_ERR_ESPNOW_FULL:
        Serial.println("ERROR: ESPNOW peer list is full");
        break;
    case ESP_ERR_ESPNOW_NOT_FOUND:
        Serial.println("ERROR: ESPNOW peer is not found");
        break;
    case ESP_ERR_ESPNOW_INTERNAL:
        Serial.println("ERROR: Internal error");
        break;
    case ESP_ERR_ESPNOW_EXIST:
        Serial.println("ERROR: ESPNOW peer has existed");
        break;
    case ESP_ERR_ESPNOW_IF:
        Serial.println("ERROR: Interface error");
        break;
    }
}
