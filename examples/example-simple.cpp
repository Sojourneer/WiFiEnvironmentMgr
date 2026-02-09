#include <Arduino.h>
#include <WiFiEnvironmentMgr.h>
#include <Ticker.h>

/*
  WiFiEnvironmentMgr Simple Example
  
  This example demonstrates how to use WiFiEnvironmentMgr to:
  1. Connect to known WiFi networks defined in environments.json
  2. Automatically fall back to AP mode if no known networks are found (requires AP.json)
  3. Configure static IP addresses per MAC address
  4. Set device hostname
  
  Required files in data/ folder:
  - environments.json: Configuration for known WiFi networks
  
  Optional file for AP mode fallback:
  - AP.json: Configuration for fallback AP mode
                If this file is not present, AP mode will not be attempted
  
  Define your filesystem in build flags:
  -DUSE_SPIFFS or -DUSE_LITTLEFS
*/

// Create an instance of the WiFiEnvironmentMgr
WiFiEnvironmentMgr environment;

// WiFi event handlers
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi() { 
  // ConnectWifi() will:
  // 1. Scan for available networks
  // 2. Try to connect to known networks from environments.json
  // 3. If unsuccessful and AP.json exists, fall back to AP mode
  // 4. If AP.json does not exist, no AP mode is configured
  bool connected = environment.ConnectWifi();
  
  if(connected) {
    if(WiFi.getMode() == WIFI_AP) {
      Serial.println("Running in AP Mode");
      Serial.printf("AP SSID: %s\n", environment.ssid);
      Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
      // In AP mode, you might want to start a web server for configuration
    } else {
      Serial.println("Connected to WiFi network");
      
      // Configure environment based on connected SSID
      String ssid = WiFi.SSID();
      environment.set_environment(ssid.c_str(), WiFi.macAddress().c_str());
      
      // Configure static IP if specified in the environment
      if(environment.local_ip.isSet()) {
        WiFi.config(environment.local_ip, environment.gateway, environment.subnet);
      }
      
      // Set the hostname if specified
      if(environment.host) {  
        Serial.printf("Setting hostname to %s\n", environment.host);
        WiFi.hostname((const char *)environment.host);
      }
    }
  } else {
    Serial.println("Failed to establish any WiFi connection");
  }
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi network.");
  Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("Subnet: %s\n", WiFi.subnetMask().toString().c_str());
  
  if(environment.host) {
    Serial.printf("Hostname: %s\n", environment.host);
  }
}

void onSSIDSelected(const char *ssid)
{
  Serial.printf("onSSIDSelected(%s)\n", ssid);

  environment.set_environment(ssid, WiFi.macAddress().c_str());
  Serial.printf("IP:%s\n",environment.local_ip.toString().c_str());

  if(environment.local_ip.isSet()) {
    WiFi.config(environment.local_ip, environment.gateway, environment.subnet);
  }
  
  if(environment.host) {  
    Serial.printf("Setting host to %s\n", environment.host);
    WiFi.hostname((const char *)environment.host);
    #ifdef ENABLE_MDNS
      Serial.println("MDNS enabled");
      MDNS.begin((const char *)environment.host);
    #endif
  }
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  Serial.printf("Reason: %d\n", event.reason);
  
  // Attempt to reconnect after 2 seconds
  wifiReconnectTimer.once(2, connectToWifi);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nWiFiEnvironmentMgr Example");
  Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());

  // Initialize the file system (SPIFFS or LittleFS)
  #ifdef USE_SPIFFS
    if(!SPIFFS.begin()){
      while(true) {
        Serial.println("ERROR: Failed to mount SPIFFS");
        delay(5000);
      }
    }
    Serial.println("SPIFFS mounted successfully");
  #elif defined(USE_LITTLEFS)
    if(!LittleFS.begin()){
      while(true) {
        Serial.println("ERROR: Failed to mount LittleFS");
        delay(5000);
      }
    }
    Serial.println("LittleFS mounted successfully");
  #endif

  // Set up the SSID selection callback
  // This is called when a known network is found and connected
  environment.wifiMulti.onSSIDSelected(onSSIDSelected);
  
  // Set up WiFi event handlers for station mode
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  
  // Initiate WiFi connection (with AP fallback)
  connectToWifi();
}

void loop() {
  // Your application code here
  
  // Check WiFi status
  if(environment.device_wifi_status == DEVICE_STA_READY) {
    // Connected to a WiFi network in station mode
    // Do your normal work here
  } 
  else if(environment.device_wifi_status == DEVICE_SOFT_AP_READY) {
    // Running as an Access Point
    // You might handle configuration requests here
  }
  else if(environment.device_wifi_status == DEVICE_NO_WIFI) {
    // No WiFi available - handle this case
    Serial.println("No WiFi connectivity");
    delay(5000);
  }
}
