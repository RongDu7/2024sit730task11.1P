#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>

const char* ssid = "Dori";
const char* password = "qazwsxedc";

// MQTT Broker settings
const char* mqtt_server = "mqtt-dashboard.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "sensor/data";

// BH1750 Light Sensor
BH1750 lightMeter;

// DHT11 Temperature and Humidity Sensor
#define DHTPIN 3      // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11 (DHT11)
DHT dht(DHTPIN, DHTTYPE);

// LED Pins
#define RED_LED_PIN 3
#define GREEN_LED_PIN 4

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic, "Arduino connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  // Initialize BH1750
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 initialized"));
  } else {
    Serial.println(F("Error initializing BH1750"));
  }
  
  // Initialize DHT11
  dht.begin();
  
  // Initialize LEDs
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Read light intensity from BH1750
  float lux = lightMeter.readLightLevel();
  Serial.print(F("Light: "));
  Serial.print(lux);
  Serial.println(F(" lx"));
  
  // Read temperature and humidity from DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor"));
  } else {
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.println(F("°C"));
    
    // Check temperature and control red LED
    if (t > 30) {
      digitalWrite(RED_LED_PIN, HIGH);
      delay(500); // LED on for 500 milliseconds
      digitalWrite(RED_LED_PIN, LOW);
      delay(500); // LED off for 500 milliseconds
    } else {
      digitalWrite(RED_LED_PIN, LOW);
    }
    
    // Check light intensity and control green LED
    if (lux > 3000) {
      digitalWrite(GREEN_LED_PIN, LOW);
    } else {
      digitalWrite(GREEN_LED_PIN, HIGH);
    }
    
    // Publish sensor data to MQTT
    String payload = String(t) + "," + String(h) + "," + String(lux);
    Serial.print("Publishing data to MQTT: ");
    Serial.println(payload);
    
    if (client.publish(mqtt_topic, payload.c_str())) {
      Serial.println("Data published successfully");
    } else {
      Serial.println("Failed to publish data");
    }
  }
  
  delay(2000);
}
