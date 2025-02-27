#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>


typedef enum  {
    DEVICE_STA_READY = 0,
    DEVICE_SOFT_AP_READY = 1,
    DEVICE_NO_WIFI = 2,
    DEVICE_FAILURE = 3
} device_wifi_status_t;

class WiFiEnvironmentMgr
{
    public:
        WiFiEnvironmentMgr() {  }
        void load(const char *fn);
        void addAPs();
        void set_environment(const char *ssid, const char *mac);
        bool ConnectWifi(void);

    public: // accessors
        ESP8266WiFiMulti        wifiMulti;
        device_wifi_status_t    device_wifi_status;
        const char              *wifi_password;
        IPAddress               local_ip;
        IPAddress               gateway;
        IPAddress               subnet;


    private:
        bool            isLoaded = false;
        JsonDocument    doc;
        JsonObject      json_ssid;
        JsonObject      json_mac;
 
    protected:
        JsonObject config;
};

// Extension with MQTT configuration information
class MqttEnvironmentMgr: public WiFiEnvironmentMgr
{
    public:
        bool hasMQTT_Config() { return !json_mqtt.isNull(); };
        void set_environment(const char *ssid, const char *mac) {
            WiFiEnvironmentMgr::set_environment(ssid, mac);
            json_mqtt = config["mqtt"];
        };

        const char *mqtt_broker()   { return json_mqtt["broker"].as<const char *>(); };
        const int   mqtt_port()     { return json_mqtt["port"].as<const int>(); };
        const char *mqtt_user()     { return json_mqtt["user"].as<const char *>(); };;
        const char *mqtt_password() { return json_mqtt["password"].as<const char *>(); };

    private:
        JsonObject json_mqtt;
};
