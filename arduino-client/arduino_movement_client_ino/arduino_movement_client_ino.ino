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
RF24 radio(10, 9);
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
#define nodeID 3
#define masterNodeId 0

String nodeInfo;
int lastVal = LOW;

void setup() {
  // set up for batt voltage measurement
  // REFS1 REFS0          --> 0 1, AVcc internal ref. -Selects AVcc external reference
  // MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG)         -Selects channel 14, bandgap voltage, to measure
  ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0); 
  delay(50);  // Let mux settle a little to get a more stable A/D conversion

  pinMode(2, INPUT_PULLUP);
  Serial.begin(115200);
  printf_begin();

  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  
  Serial.println("------------------------------------");
  Serial.print("Movement sensor, node ");
  Serial.println(nodeID);
  Serial.println("------------------------------------");
  nodeInfo = String("Movement sensor ");

  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin();
  sendInfo();
}

int getBattVolts() { // Returns actual value of Vcc (x 100)

     const long InternalReferenceVoltage = 1080L;  // Adjust this value to your boards specific internal BG voltage x1000
        // Start a conversion
     ADCSRA |= _BV( ADSC );
        // Wait for it to complete
     while( ( (ADCSRA & (1<<ADSC)) != 0 ) );
        // Scale the value
        // 100L is my fudge factor to match my multimeter R2
     int results = (((InternalReferenceVoltage * 1024L) / ADC) + 100L) / 10L;
     return results;
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
  // mesh.update();

  int sensorVal = digitalRead(2);

  if (lastVal != sensorVal) {
      bool payloadValue = sensorVal == HIGH;
      
      //Serial.print("Sensor detected event, event ");
      
      if (!mesh.write(&payloadValue, 'S', sizeof(payloadValue), masterNodeId)) {
        Serial.println(F("send fail"));
        if ( ! mesh.checkConnection() ) {
          Serial.println(F("renewing Address"));
          mesh.renewAddress();
        } else {
          Serial.println(F("send fail, Test OK"));
        }
      } else {
        //Serial.println(F("sent OK.")); 
      }
      lastVal = sensorVal;
  }
  delay(200);
}
