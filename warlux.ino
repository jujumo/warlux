#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "config.h"

//RTC_DATA_ATTR int bootCount = 0;

#define LED 2
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex

//                          12h   m    s
const int TIME_TO_SLEEP_S = 12 * 60 * 60;        /* Time ESP32 will go to sleep (in seconds) */

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);


//Function that gives the reason by which ESP32 has been awaken from sleep
String get_wakeup_string(esp_sleep_wakeup_cause_t wakeup_reason_code)
{
  switch(wakeup_reason_code)
  {
    case ESP_SLEEP_WAKEUP_EXT0    : return "external signal using RTC_IO"; 
    case ESP_SLEEP_WAKEUP_EXT1    : return "external signal using RTC_CNTL"; 
    case ESP_SLEEP_WAKEUP_TIMER   : return "timer"; 
    case ESP_SLEEP_WAKEUP_TOUCHPAD: return "ouchpad"; 
    case ESP_SLEEP_WAKEUP_ULP     : return "ULP program";
    default                       : return "not sleeping: " + String(wakeup_reason_code);
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
  //mqtt_client.setCallback(callback);
  
  Serial.print("MQTT: connecting to " + String(mqtt_server) + " ");
  mqtt_client.connect(mqtt_device_name.c_str(), mqtt_user, mqtt_password);
  while (!mqtt_client.connected()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" DONE");

}


void blink()
{
    for (int i=0; i<10; ++i) {
      delay(200);
      digitalWrite(LED, LOW);
      delay(300);
      digitalWrite(LED, HIGH);
    }
}


void setup()
{
  // Setup hardware
  pinMode(LED,OUTPUT);
  digitalWrite(LED, HIGH);
  //Configure GPIO33 as ext0 wake up source for HIGH logic level
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, LOW);
  gpio_pullup_en(GPIO_NUM_33);
  //esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  // We set our ESP32 to wake up every 5 seconds
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_S * uS_TO_S_FACTOR);
  // setup print
  Serial.begin(115200);
  delay(1000); 

  //Print the wakeup reason for ESP32
  esp_sleep_wakeup_cause_t wakeup_code = esp_sleep_get_wakeup_cause();
  String wakeup_string = get_wakeup_string(wakeup_code);
  Serial.println("ESP: awaken because : " + wakeup_string);

  // set up wifi + mqtt 
  setup_wifi();
  setup_mqtt();
  
  // publish status
  const String mqtt_topic = "warlux/" + mqtt_device_name + "/motion";
  const String mqtt_payload = String(wakeup_code);
  Serial.println("MQTT: publish\t" + mqtt_topic + " : " + mqtt_payload);
  mqtt_client.publish(mqtt_topic.c_str(), mqtt_payload.c_str());

  // avoid multiple trigger in a row
  if (wakeup_code == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.print("wait a bit ");
    blink();
    delay(50000);
    blink();
  }
  
  //Go to sleep now
  digitalWrite(LED, LOW);
  Serial.println("ESP: going to sleep now.");
  esp_deep_sleep_start();
}


void loop(){}
