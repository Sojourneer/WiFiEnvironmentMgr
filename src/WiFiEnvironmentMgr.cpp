#include "WiFiEnvironmentMgr.h"
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
extern ESP8266WiFiMulti wifiMulti; 

void WiFiEnvironmentMgr::load(const char *fn)
{
    File file = SPIFFS.open(fn, "r");
    if(!file){
        while(true) {
            Serial.println("Failed to open file for reading");
            delay(5000);
        };
    }
    String content = file.readString();

    Serial.printf("Config file '%s' Content:", fn);
    Serial.print(content);
    file.close();

    DeserializationError err = deserializeJson(doc, content);
    if(err == err.Ok)
        Serial.println("deserialized");
    else {
        while(true) {
            Serial.printf("Deserialization error: %s", err.c_str());
            delay(5000);
        }
    }
}

void WiFiEnvironmentMgr::set_environment(const char *ssid, const char *mac)
{
    if(!isLoaded) {
        load("/environments.json");
        isLoaded = true;
    }
    JsonObject root = doc.as<JsonObject>();

    for (JsonPair kv : root) {
        const char *json_ssid = kv.key().c_str();
        if(strcmp(json_ssid, ssid) != 0)
            continue;
        config = kv.value().as<JsonObject>();

        wifi_password = config["wifi_password"];

        if(config["local_ip"])
            local_ip.fromString(config["local_ip"].as<const char *>());
        else
            local_ip.clear();

        if(config["gateway"])
            gateway.fromString(config["gateway"].as<const char *>());
        else
            gateway.clear();

        if(config["subnet"])
            subnet.fromString(config["subnet"].as<const char *>());
        else
            subnet.clear();

        JsonObject mac_ip = config["mac_ip"];
        if(!mac_ip.isNull()) {
            String mac = WiFi.macAddress();

            JsonObject mac_entry = mac_ip[mac];
            if(! mac_entry.isNull()) {
                JsonString ip = mac_entry["ip"];
                if(!ip.isNull()) {
                    Serial.printf("Setting IP from MAC: %s -> %s [%s]\n",
                        mac.c_str(),
                        ip.c_str(),
                        mac_entry["comment"].as<const char *>());
                    local_ip.fromString(ip.c_str());
                }
            }
        }

        return;
    }

};

void WiFiEnvironmentMgr::addAPs()
{
    if(!isLoaded) {
        load("/environments.json");
        isLoaded = true;
    }
    JsonObject root = doc.as<JsonObject>();

    for (JsonPair kv : root) {
        const char *json_ssid = kv.key().c_str();
        JsonObject config = kv.value().as<JsonObject>();

        wifiMulti.addAP(json_ssid, config["wifi_password"]);
    }
};
