# WiFiEnvironmentMgr
Library providing lookup by SSID of configuration information from a JSON file stored in SPIFFS. 

## environments.json sample
~~~
{
    "mySSID":{
        "wifi_password":"???",
        "mqtt":{"broker":"192.168.1.73", "port":1883,"user":"???", "password":"???"}
    },
    "myOtherSSID":{
        "wifi_password":"???",
        "gateway":"192.168.2.1", "subnet":"255.255.255.0",
        "mac_ip": {
            "C8:2B:96:F2:FD:C6":{"ip":"192.168.2.50","comment":"devkit"},
            "E8:DB:84:5B:9F:4F":{"ip":"192.168.2.51","comment":"PIR"},
            "D8:BC:38:B5:E0:A4":{"ip":"192.168.2.52","comment":"PIR"}
        },
        "mqtt":{"broker":"192.168.2.13", "port":1883, "user":"?", "password":"?"}
    }
}
~~~
