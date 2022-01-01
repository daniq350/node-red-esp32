#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 4//DHT 22 sensor pin
#define DHTTYPE           DHT22 

const int pResistor = 2; //photoresistor pin

const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";


const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

DHT_Unified dht(DHTPIN, DHTTYPE); //DHT22 sensor object
uint32_t delayMS;

//variables to store sensor values
int pr_value=0;  
float temperature=0;
float humidity=0;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
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
void setup() {
  Serial.begin(115200);

  pinMode(pResistor, INPUT);//photoresistor pin as input
  //---------DHT22 Setup Code-----------
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  //------------------------------------

  //---------WIFI setup code-----------
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //--------MQTT setup Code-----------
  client.setServer(mqtt_server, 1883);
 
  //------------------------------------
}


void loop() {

if (!client.connected()) {
    reconnect();
  }
  client.loop();

  
  pr_value = analogRead(pResistor); //read photoresistor value
  
  if (pr_value > 25){
    //day
    client.publish("esp32/photoresistor", "Day");
  }
  else{
    //Night
    client.publish("esp32/photoresistor", "Night");
  }

delay(delayMS);
 
    sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    temperature=event.temperature;
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
  }
    
   dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    humidity=event.relative_humidity;
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
  }

  //convert sensors values (float) to character string
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/temperature", tempString);

    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/humidity", humString);
  
}
