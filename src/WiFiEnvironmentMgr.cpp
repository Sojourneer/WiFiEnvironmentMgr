#include "WiFiEnvironmentMgr.h"
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>

const char *config_filename = "/environments.json";

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
        load(config_filename);
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
        load(config_filename);
        isLoaded = true;
    }
    JsonObject root = doc.as<JsonObject>();

    for (JsonPair kv : root) {
        const char *json_ssid = kv.key().c_str();
        JsonObject config = kv.value().as<JsonObject>();

        wifiMulti.addAP(json_ssid, config["wifi_password"]);
    }
};

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

    //TODO Do we need it?  If so, use the MQTT_ClientId instead
    #ifdef ENABLE_MDNS
    uint8_t shortname [18];
    sprintf((char *)shortname, "PIR%d", WiFi.localIP()[3]);
    WiFi.hostname((const char *)shortname);
    MDNS.begin((const char *)shortname);
    #endif

    return true;
  } 
  
  Serial.println("");
  Serial.println("Connection failed. Fallback to SoftAP");
  //UPDATE_INDICATOR(255, 0, 0);

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
