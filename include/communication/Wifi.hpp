#ifndef WIFI_HPP
#define WIFI_HPP

#include <WiFi.h>
#include "setupConfiguration/utils.hpp"
#include "communication/CommunicationNumberHelper.hpp"

#define DELAY_FOR_SETTING_WIFI 10
#define TIMEOUT_FOR_CONNECTING_WIFI 500

class Wifi
{
private:
    char *m_wifiSsid;
    char *m_wifiPassword;
    bool m_isWifiConnected;
public:
    Wifi(char *wifiSsid, char *wifiPassword, bool isWifiConnected = false);
    ~Wifi();

    /**
     * @brief Set the Wifi Ssid object
     */
    void setWifiSsid(char *wifiSsid);
    void setWifiPassword(char *wifiPassword);

    
    /**
     * @brief Get the Wifi Ssid object
     * @return char* 
     */
    char *getWifiSsid();
    char *getWifiPassword();

    
    /**
     * @brief Connect to Wifi network
     */
    bool connect();
    void disconnect();
    bool isConnected() const;

    static std::shared_ptr<Wifi> create(char *wifiSsid, char *wifiPassword, bool isWifiConnected = false);
};

#endif // WIFI_HPP