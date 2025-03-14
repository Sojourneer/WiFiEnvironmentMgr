#include "WiFiEnvironmentMgr.h"
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>

const char *config_filename = "/environments.json";
const char *APconfig_filename = "/AP.json";

void WiFiEnvironmentMgr::load_configs(const char *fn)
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

void WiFiEnvironmentMgr::load_APconfig(const char *fn, JsonDocument apDoc)
{
    File file = SPIFFS.open(APconfig_filename, "r");
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

void WiFiEnvironmentMgr::set_environment(const char *selected_ssid, const char *mac)
{
    if(!isLoaded) {
        load_configs(config_filename);
        isLoaded = true;
    }
    JsonObject root = doc.as<JsonObject>();

    for (JsonPair kv : root) {
        const char *json_ssid = kv.key().c_str();
        if(strcmp(json_ssid, selected_ssid) != 0)
            continue;
            
        ssid = json_ssid;
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

        JsonObject mac_ip = config["per_mac"];
        if(!mac_ip.isNull()) {
            String mac = WiFi.macAddress();

            JsonObject mac_entry = mac_ip[mac];
            if(! mac_entry.isNull()) {
                JsonString ip = mac_entry["ip"];
                if(!ip.isNull()) {
                    Serial.printf("Setting IP from MAC: %s -> %s\n",
                        mac.c_str(),
                        ip.c_str());
                    local_ip.fromString(ip.c_str());
                }
                if(mac_entry["host"].is<JsonString>()) {
                    host = mac_entry["host"];
                    Serial.printf("Setting host to %s\n", host);
                } else {
                    host = nullptr;
                }
            }

        }
        return;
    }

};

void WiFiEnvironmentMgr::addAPs()
{
    if(!isLoaded) {
        load_configs(config_filename);
        isLoaded = true;
    }
    JsonObject root = doc.as<JsonObject>();

    for (JsonPair kv : root) {
        const char *json_ssid = kv.key().c_str();
        JsonObject config = kv.value().as<JsonObject>();

        wifiMulti.addAP(json_ssid, config["wifi_password"]);
    }
};


bool WiFiEnvironmentMgr::set_AP()
{
    load_APconfig(APconfig_filename, apDoc);
    JsonObject apConfig = doc.as<JsonObject>();

    if(apConfig["ssid"])
        ssid = apConfig["ssid"];
    else
        return false;

    if(config["local_ip"])
        local_ip.fromString(apConfig["local_ip"].as<const char *>());
    else
        return false;

    if(apConfig["gateway"])
        gateway.fromString(apConfig["gateway"].as<const char *>());
    else
        return false;

    if(apConfig["subnet"])
        subnet.fromString(config["subnet"].as<const char *>());
    else
        return false;

    return true;
}

// connect to wifi – returns true if WiFi estabished (either AP or STA)
bool WiFiEnvironmentMgr::ConnectWifi(void)
{
  bool state = true;
  int i = 0;

  addAPs();

  Serial.print("Scanning...");
  int n = WiFi.scanNetworks();
  if(n == 0) {
    Serial.println("No networks found");
    state = false;
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.print("Connecting to WiFi ");
    wl_status_t wifistatus;
    while ((wifistatus = wifiMulti.run()) != WL_CONNECTED) {
      delay(10000);
      Serial.print(".");
      Serial.printf("%d ", wifistatus);
      if (i > 10){
        state = false;
        break;
      }
      i++;
    }
  }

  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    String ssid = WiFi.SSID();
    Serial.print(WiFi.SSID());
    Serial.print(", ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    return true;
  } 
  
  Serial.println("");
  Serial.println("Connection failed. Fallback to SoftAP");
  //UPDATE_INDICATOR(255, 0, 0);

  if(set_AP()) {
    //TODO load from AP.json
    if(false == WiFi.softAPConfig(local_ip, gateway, subnet)) {  
        Serial.printf("Fallback to SoftAP '%s' as configured failed\n", ssid);
        goto tryDefaultAP;
    }
    
    JsonObject options = apDoc.as<JsonObject>()["options"];
    if(options.isNull()) {
        if(false == WiFi.softAP(ssid)) {
            goto tryDefaultAP;
        }
        return true;
    } else {
        if(false == WiFi.softAP(ssid,
                options["wifi_password"],
                options["channel"],
                options["hidden"],
                options["max_connections"])) {
            goto tryDefaultAP;
        }
        return true;
    }

    

    return true;
  }

  tryDefaultAP:
  
  IPAddress local_IP(192,168,2,1);
  IPAddress gateway(192,168,2,1);
  IPAddress subnet(255,255,255,0);

  if(false == WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("SoftAP config failed.");
    device_wifi_status = DEVICE_NO_WIFI;
    return false;
  }
  if(false == WiFi.softAP(SOFTAP_SSID, SOFTAP_PSK, SOFTAP_CHANNEL, SOFTAP_HIDDEN, SOFTAP_MAX_CONNECTIONS)) {
    device_wifi_status = DEVICE_NO_WIFI;
    return false;
  }
  
  device_wifi_status = DEVICE_SOFT_AP_READY;

  return true;
}
