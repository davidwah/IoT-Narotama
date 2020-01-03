#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>

#include "DHT.h"

#define DHTPIN 5          // Pin Sensor DHT11
#define DHTTYPE DHT11     // Tipe Sensor DHT11

DHT dht(DHTPIN, DHTTYPE);

#define ssid "SSID"           // SSID
#define password "PASSWORD"   // Password WiFi

#define mqtt_server "IP_Address"   // Broker MQTT
//#define mqtt_user "user"
//#define mqtt_password "password"

#define humidity_topic "sensor/humidity"                        // Topik MQTT Humidity
#define temperature_celsius_topic "sensor/temp_celsius"         // Topik MQTT Temperature Celsius
#define temperature_fahrenheit_topic "sensor/temp_fahrenheit"   // Topik MQTT Temperature Fahrenheit

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    String clientName;  
    clientName += "esp8266-";
    uint8_t mac[6];
    WiFi.macAddress(mac);
    clientName += macToStr(mac);
    clientName += "-";
    clientName += String(micros() & 0xff, 16);
    Serial.print("Connecting to ");
    Serial.print(mqtt_server);
    Serial.print(" as ");
    Serial.println(clientName);



  if (client.connect((char*) clientName.c_str())) {
    //if (client.connect((char*) clientName.c_str()), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
      delay(2000);

      float h = dht.readHumidity();
      float t = dht.readTemperature();
      float f = dht.readTemperature(true);
      
      if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }
      
      float hif = dht.computeHeatIndex(f, h);
      float hic = dht.computeHeatIndex(t, h, false);
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" *C ");
      Serial.print(f);
      Serial.print(" *F\t");
      Serial.print("Heat index: ");
      Serial.print(hic);
      Serial.print(" *C ");
      Serial.print(hif);
      Serial.println(" *F");


      Serial.print("Temperature in Celsius:");
      Serial.println(String(t).c_str());
      client.publish(temperature_celsius_topic, String(t).c_str(), true);

      Serial.print("Temperature in Fahrenheit:");
      Serial.println(String(f).c_str());
      client.publish(temperature_fahrenheit_topic, String(f).c_str(), true);


      Serial.print("Humidity:");
      Serial.println(String(h).c_str());
      client.publish(humidity_topic, String(h).c_str(), true);
}
