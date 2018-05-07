/** RF24Mesh_Example_Node2Node.ino by TMRh20
 *
 * This example sketch shows how to communicate between two (non-master) nodes using
 * RF24Mesh & RF24Network
 **/

// Depends on the following Arduino libraries:
// - Adafruit Unified Sensor Library: https://github.com/adafruit/Adafruit_Sensor
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library

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
#define nodeID 2
#define otherNodeID 0

#define DHTPIN            2         // Pin which is connected to the DHT sensor
#define DHTTYPE           DHT11     // DHT 11

//#################################
DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayTime = 20000;
uint32_t delaySensor;

// Structure of our payload
struct payload_t
{
 float temperature;
 uint32_t humidity;
};

void setup() {
  Serial.begin(115200);
  //printf_begin();
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin();
  
  // Initialize device.
  dht.begin();
  Serial.println("DHTxx Unified Sensor Example");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
  // Set delay between sensor readings based on sensor details.
  delaySensor = sensor.min_delay / 1000;
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
  }
  else {
    payload.temperature = eventT.temperature;
    Serial.print("Temperature: ");
    Serial.print(eventT.temperature);
    Serial.println(" *C");
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
    Serial.println("%");
  }

  
  // Send to the master node every second
  // Send an 'M' type to other Node containing the current millis()
  if (!mesh.write(&payload, 'H', sizeof(payload), otherNodeID)) {
      Serial.println(F("Send fail"));
      if ( ! mesh.checkConnection() ) {
        Serial.println(F("Renewing Address"));
        mesh.renewAddress();
      } else {
        Serial.println(F("Send fail, Test OK"));
      }
    } else {
      Serial.print(F("Send OK.")); 
    }

  delay(delayTime);

//  
//    stringTimer = millis();
//    //Copy the current number of characters to the temporary array
//    memcpy(tmpStr, dataStr, strCtr);
//    //Set the last character to NULL
//    tmpStr[strCtr] = '\0';
//
//    // Send the temp string as an 'S' type message
//    // Send it to otherNodeID (An RF24Mesh address lookup will be performed)
//    //bool ok = mesh.write(tmpStr,'S',strCtr+1,otherNodeID);
//    if (mesh.write(tmpStr, 'S', strCtr + 1, otherNodeID)) {
//      strCtr++;
//      delayTime = 333;
//      //Set the sending length back to 1 once max size is reached
//      if (strCtr == sizeof(dataStr)) {
//        strCtr = 1;
//      }
//      //if(strCtr == 12){ strCtr=1; }
//    }
//  }
}
