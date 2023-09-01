#include <Adafruit_HTU21DF.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//******************************************************************************************
// WiFi settings
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_SSID_PASSWORD";

// MQTT settings
const char* mqtt_server = "MQTT_SERVER_HOSTNAME_OR_IP";
const int mqtt_port = 1883;
const char* mqtt_username = "MQTT_USERNAME";
const char* mqtt_password = "MQTT_PASSWORD";

// OTA settings
const char* ota_hostname = "FanBuddy";
const char* ota_password = "YOUR_OTA_PASSWORD_HERE";

//******************************************************************************************

const char* mqtt_topic_temperature = "FanBuddy/htu21/temperature";
const char* mqtt_topic_humidity = "FanBuddy/htu21/humidity";

const char* mqtt_control_topic = "FanBuddy/control/fan";
const char* mqtt_topic_fan_state = "FanBuddy/fan/state";
const char* mqtt_status_topic = "FanBuddy/state";


// Network settings
#define HOSTNAME "FanBuddy"

// Fan control
const int fanPin = 14;

bool fanState = false;
const int maxPayloadLength = 128;

Adafruit_HTU21DF htu21;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  setupOTA();
  setupMDNS();

  pinMode(fanPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  WiFi.hostname(HOSTNAME);
  connectToWiFi();

  // Initialize sensor
  htu21.begin();

  // Set MQTT server and callback
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  //client.setWill(mqtt_status_topic, "Offline", true, 1);
  client.subscribe(mqtt_control_topic);
}

void loop() {
  ArduinoOTA.handle();

  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  float temperature = htu21.readTemperature();
  float humidity = htu21.readHumidity();

  String temperatureStr = String(temperature, 2);
  String humidityStr = String(humidity, 2);

  // Send sensor data via MQTT
  // client.publish(mqtt_topic, ("Temperature: " + temperatureStr + " °C | Humidity: " + humidityStr + " %").c_str());
  client.publish(mqtt_topic_temperature, temperatureStr.c_str());
  client.publish(mqtt_topic_humidity, humidityStr.c_str());
  client.publish(mqtt_topic_fan_state, fanState ? "ON" : "OFF");

  delay(5000); // Wait for 5 seconds before reading and sending again
}

void setupOTA() {
  ArduinoOTA.setHostname(ota_hostname);
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.begin();
}

void setupMDNS() {
  if (!MDNS.begin(ota_hostname)) {
    Serial.println("Error setting up MDNS responder!");
  }
}

void connectToWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(ota_hostname, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_control_topic);
      client.publish(mqtt_status_topic, "Online", true);  // Publish online status
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String receivedTopic = String(topic);

  // Check if the payload length exceeds the maximum allowed length
  if (length > maxPayloadLength) {
    Serial.println("Payload length exceeds the maximum allowed length. Ignoring message.");
    return;
  }

  // Ensure receivedTopic is not empty
  if (receivedTopic.length() == 0) {
    Serial.println("Empty topic received. Ignoring message.");
    return;
  }

  // Convert payload to a string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Check receivedTopic against known control topics
  if (receivedTopic.equals(mqtt_control_topic)) {
    if (message.equals("setFanOn")) {
      fanState = true;
      client.publish(mqtt_topic_fan_state, "ON"); // Publish the new state
    } else if (message.equals("setFanOff")) {
      fanState = false;
      client.publish(mqtt_topic_fan_state, "OFF"); // Publish the new state
    } else {
      Serial.println("Unknown control message received.");
    }
  } else {
    Serial.println("Unknown topic received.");
  }
}