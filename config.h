// Replace the next variables with your SSID/Password combination
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWD";

// MQTT
const char* mqtt_server = "MQTT_SRVER_ADRDRESS";
const char* mqtt_user = "MQTT_USER";
const char* mqtt_password = "MQTT_PASSWD";
const int   mqtt_port = 1883;
const String mqtt_device_name = String("warlux_") + String((uint16_t)(ESP.getEfuseMac() >> 32));
