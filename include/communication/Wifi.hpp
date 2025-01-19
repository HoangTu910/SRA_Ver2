#include <WiFi.h>

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

    //setter
    void setWifiSsid(char *wifiSsid);
    void setWifiPassword(char *wifiPassword);

    //getter
    char *getWifiSsid();
    char *getWifiPassword();

    //configuration
    bool connect();
    void disconnect();
    bool isConnected() const;
};


