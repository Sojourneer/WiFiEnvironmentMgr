#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

class WiFiEnvironmentMgr
{
    public:
        WiFiEnvironmentMgr() {  }
        void load(const char *fn);
        void addAPs();
        void set_environment(const char *ssid, const char *mac);

    public: // accessors
        const char *wifi_password;
        IPAddress   local_ip;
        IPAddress   gateway;
        IPAddress   subnet;


    private:
        bool     isLoaded = false;
        JsonDocument doc;
        JsonObject json_ssid;
        JsonObject json_mac;
 
    protected:
        JsonObject config;
};


class MqttEnvironmentMgr: public WiFiEnvironmentMgr
{
    public:
        bool hasMQTT_Config() { return !json_mqtt.isNull(); };
        JsonObject set_environment(const char *ssid, const char *mac) {
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
