#include "communication/Wifi.hpp"

// Constructor implementation
Wifi::Wifi(char *wifiSsid, char *wifiPassword, bool isWifiConnected)
    : m_wifiSsid(wifiSsid), m_wifiPassword(wifiPassword), m_isWifiConnected(isWifiConnected)
{
}


// Destructor implementation
Wifi::~Wifi()
{
    
}

void Wifi::setWifiSsid(char *wifiSsid)
{
    m_wifiSsid = wifiSsid;
}
void Wifi::setWifiPassword(char *wifiPassword)
{
    m_wifiPassword = wifiPassword;
}
char *Wifi::getWifiSsid()
{
    return m_wifiSsid;
}
char *Wifi::getWifiPassword()
{
    return m_wifiPassword;
}
// Connect to the WiFi network
// Return true if successful, false otherwise
bool Wifi::connect()
{
    delay(DELAY_FOR_SETTING_WIFI);
    PLAT_LOG_D("Connecting to WiFi network %s", m_wifiSsid);
    WiFi.begin(m_wifiSsid, m_wifiPassword);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(TIMEOUT_FOR_CONNECTING_WIFI);
        PLAT_LOG_ED("%s", ".");
    }
    PLAT_LOG_D("%s","");
    PLAT_LOG_D("Connected to WiFi network %s", m_wifiSsid);
    PLAT_LOG_D("IP Address: %s", WiFi.localIP().toString().c_str());
    m_isWifiConnected = true;
    return true;
}

void Wifi::disconnect()
{
    // Disconnect from the WiFi network
}

bool Wifi::isConnected() const
{
    return m_isWifiConnected;
}

std::shared_ptr<Wifi> Wifi::create(char *wifiSsid, char *wifiPassword, bool isWifiConnected)
{
    return std::make_shared<Wifi>(wifiSsid, wifiPassword, isWifiConnected);
}
