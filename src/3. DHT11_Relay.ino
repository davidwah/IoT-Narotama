#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 5      // Pin Sensor DHT11
#define DHTTYPE DHT11 // Tipe Sensor DHT11

#define LED 2     // PIN D4

#define humidity_topic "sensor/humidity"
#define temperature_celsius_topic "sensor/temp_celsius"
#define temperature_fahrenheit_topic "sensor/temp_fahrenheit"


DHT dht(DHTPIN, DHTTYPE);

#define ssid "SSID"           // SSID
#define password "PASSWORD"   // Password WiFi

#define mqtt_server "broker.hivemq.com"   // Broker MQTT
//#define mqtt_user "user"
//#define mqtt_password "password"

int relayPin = 16 ;  // PIN D0

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
   delay(100);
   // koneksi pada jaringan WIFI
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Command from MQTT broker is : [");
  Serial.print(topic);
  Serial.print("   ");
  
  int p =(char)payload[0]-'0';
  // if MQTT=0, Relay mati di pin D1
  if(p==0)
  {
    digitalWrite(LED, LOW);
    digitalWrite(relayPin, LOW);
    Serial.println(" Relay Mati! ");
  } 
  // if MQTT=1, Relay aktif di pin D1
  if(p==1)
  {
    digitalWrite(LED, HIGH);
    digitalWrite(relayPin, HIGH);
    Serial.println(" Relay Aktif! ");
  }
    Serial.println();
}
//end callback

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    
    // Create Name ClientId
    String clientId = "ESP8266";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str()))
//    if (client.connect((char*) clientName.c_str()), mqtt_user, mqtt_password)) {
    {
      Serial.println("connected");
      client.subscribe("aktuator/relay"); //Topik MQTT yang di Subscribe
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // delay 5 detik
      delay(5000);
    }
  }
} //end reconnect()

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
  pinMode(LED,OUTPUT);
  pinMode(relayPin, OUTPUT);
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
