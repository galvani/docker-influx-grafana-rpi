/**
 * Depends on RF24, RF24Network, RF24Mesh
 * Depends on the following Arduino libraries:
 *  * Adafruit Unified Sensor Library: https://github.com/adafruit/Adafruit_Sensor
 *  * DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
 **/

#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <printf.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


//########### USER CONFIG ###########

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(7, 8);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/**
 * User Configuration:
 * nodeID - A unique identifier for each radio. Allows addressing to change dynamically
 * with physical changes to the mesh. (numbers 1-255 allowed)
 *
 * otherNodeID - A unique identifier for the 'other' radio
 *
 **/
#define nodeID 4
#define masterNodeId 0

#define DHTPIN           4         // Pin which is connected to the DHT sensor
// #define DHTTYPE           DHT22     // DHT 11
#define DHTTYPE           DHT11     // DHT 11

//#################################
DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayTime = 45000;
uint32_t delaySensor;
uint32_t infoFrequency = 100;
uint32_t infoRunLeft = infoFrequency + 1;

// Structure of our payload
struct payload_t
{
 float temperature;
 uint32_t humidity;
};

String nodeInfo;

void setup() {
  Serial.begin(115200);
  printf_begin();

  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  
  // Initialize device.
  dht.begin();
  Serial.println("DHTxx Unified Sensor Example");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  nodeInfo += String(sensor.name);
  nodeInfo += String(", ") + String(sensor.min_value) + String("-");
  nodeInfo += String(sensor.max_value) + String("*C ~") + String(sensor.resolution) + String("*C, ");
  
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
  nodeInfo += String(", ") + String(sensor.min_value) + String("-");
  // Set delay between sensor readings based on sensor details.
  delaySensor = sensor.min_delay / 1000;
  
  nodeInfo += String(sensor.max_value) + String("% ~") + String(sensor.resolution) + String("%, delay:") + String(delayTime/1000);

  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin();
  Serial.println(String("Update frequency is ...") + String(delayTime/1000) + String(" seconds..."));
  sendInfo();
}

void sendInfo() {
  char charBuf[nodeInfo.length()+1];
  nodeInfo.toCharArray(charBuf, nodeInfo.length()+1);

  bool sent = false;

  while (!sent) {
    Serial.print(F("Sending node info... ")); 
    Serial.print(sizeof(charBuf));
    Serial.print(F(" bytes... ")); 
    // Send to the master node every ~delayTime miliseconds
    if (!mesh.write(&charBuf, 'I', sizeof(charBuf), masterNodeId)) {
        Serial.println(F("failed."));
        delay(5000);
        if ( ! mesh.checkConnection() ) {
          Serial.println(F("renewing address..."));
          mesh.renewAddress();
        } else {
          Serial.println(F("fail, Test OK"));
          sent = true;
        }
      } else {
        Serial.println(F("Send OK."));
        sent = true;
        delay(1);
      }
    }
    delay(100);
}

void loop() {
  mesh.update();

  payload_t payload;

  // Get temperature event and print its value.
  sensors_event_t eventH;
  sensors_event_t eventT;
  dht.temperature().getEvent(&eventT);
  
  if (isnan(eventT.temperature)) {
    Serial.println("Error reading temperature!");
    Serial.println(eventT.temperature);
  }
  else {
    payload.temperature = eventT.temperature;
    Serial.print("Temperature: ");
    Serial.print(eventT.temperature);
    Serial.print(" *C, ");
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&eventH);
  if (isnan(eventH.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    payload.humidity = eventH.relative_humidity;
    Serial.print("Humidity: ");
    Serial.print(eventH.relative_humidity);
    Serial.print("% ...");
  }

  
  // Send to the master node every ~delayTime miliseconds
  if (!mesh.write(&payload, 'H', sizeof(payload), masterNodeId)) {
      Serial.println(F("Send fail"));
      if ( ! mesh.checkConnection() ) {
        Serial.println(F("Renewing Address"));
        mesh.renewAddress();
      } else {
        Serial.println(F("Send fail, Test OK"));
      }
    } else {
      Serial.println(F("Send OK.")); 
    }

  infoRunLeft--;

  if (infoRunLeft==0) {
    sendInfo();
    infoRunLeft = infoFrequency;
  }

  delay(delayTime);
}
