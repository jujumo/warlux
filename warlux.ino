#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "config.h"

const int        ID_LED = 2;

//                           1 min
const int TIME_TO_SLEEP_uS = 60 * 1.e06; // Time ESP32 will go to pause (1 minute)

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);


//Function that gives the reason by which ESP32 has been awaken from sleep
String get_wakeup_string(esp_sleep_wakeup_cause_t wakeup_reason_code)
{
  switch(wakeup_reason_code)
  {
    case ESP_SLEEP_WAKEUP_UNDEFINED : return "reset was not caused by exit from deep sleep";
    case ESP_SLEEP_WAKEUP_EXT0    : return "trigger"; 
    case ESP_SLEEP_WAKEUP_EXT1    : return "trigger"; 
    case ESP_SLEEP_WAKEUP_TIMER   : return "timer"; 
    case ESP_SLEEP_WAKEUP_TOUCHPAD: return "touchpad"; 
    case ESP_SLEEP_WAKEUP_ULP     : return "ULP program";
    default                       : return "undefined";
  }
}


String get_mqtt_state_str(int state_code)
{
  switch(state_code)
  {
    case MQTT_CONNECTION_TIMEOUT      : return "timeout";
    case MQTT_CONNECT_FAILED          : return "failed";
    case MQTT_DISCONNECTED            : return "disconnected";
    case MQTT_CONNECTED               : return "connected"; // 0
    case MQTT_CONNECT_BAD_PROTOCOL    : return "bad protocol";
    case MQTT_CONNECT_BAD_CLIENT_ID   : return "bad client id";
    case MQTT_CONNECT_UNAVAILABLE     : return "unavailable";
    case MQTT_CONNECT_BAD_CREDENTIALS : return "bad credentials";
    case MQTT_CONNECT_UNAUTHORIZED    : return "unauthorized";
    default : return String("Unknown : ") + String(state_code);
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  Serial.print("WiFi: connecting to " + String(ssid) + " ");
  //Serial.println(ssid);

  WiFi.begin(ssid, password);

  // todo: give up after an number of attempt
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" DONE");
  Serial.print("WiFi: connected with IP address ");
  Serial.println(WiFi.localIP());
}


void setup_mqtt() 
{
  mqtt_client.setServer(mqtt_server, mqtt_port); 
  Serial.print("MQTT: connecting to " + String(mqtt_server) + " ");
  mqtt_client.connect(mqtt_device_name.c_str(), mqtt_user, mqtt_password);
  // todo: give up after an number of attempt
  while (!mqtt_client.connected()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" DONE");
}

void send_mqtt(const String& mqtt_topic, const String& mqtt_payload) {
  Serial.println("MQTT: publish\t" + mqtt_topic + " : " + mqtt_payload);
  mqtt_client.publish(mqtt_topic.c_str(), mqtt_payload.c_str());
}

void setup()
{
  // Setup hardware
  pinMode(ID_LED,OUTPUT);
  // We set our ESP32 to wake up every 5 seconds
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_uS);
  // setup print
  Serial.begin(115200);
  digitalWrite(ID_LED, HIGH);
  delay(1000); 

  //Print the wakeup reason for ESP32
  esp_sleep_wakeup_cause_t wakeup_code = esp_sleep_get_wakeup_cause();
  const String wakeup_string = get_wakeup_string(wakeup_code);
  Serial.println("ESP: awaken because : " + wakeup_string);

  // set up wifi + mqtt 
  setup_wifi();
  setup_mqtt();
  
  send_mqtt("motion/warlux/" + mqtt_device_name, "on");
  if (wakeup_code == ESP_SLEEP_WAKEUP_TIMER) {
    // wake up y timer. Should not happen ...
    send_mqtt("status/warlux/" + mqtt_device_name , "on"); 
  }

  digitalWrite(ID_LED, LOW);
  Serial.println("ESP: go to sleep.");
  // Go to sleep now
  esp_deep_sleep_start();
}


void loop() {}
