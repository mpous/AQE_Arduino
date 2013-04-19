// # Air Quality Egg Shield to Cosm - v0.0.1
//
// copyright (c) 2012 sam mulube
// released under the terms of the MIT license (see README.md for details)

#include <SPI.h>
#include <Ethernet.h>
#include <stdint.h>
#include <DHT.h>
#include "Wire.h"
#include "EggBus.h"

EggBus eggBus;

#define DHTPIN 17 // analog pin 3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Change these settings to match your feed and api key
#define FEEDID "126468"
#define APIKEY "COSM_APIKEY"
#define USERAGENT "AQE" // user agent is the project name

// Upload frequency in milliseconds
#define FREQUENCY 20000

// MAC address for your Ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

char website[] PROGMEM = "api.cosm.com";

IPAddress ip(10,0,1,20);
IPAddress server(216,52,233,121);      // numeric IP for api.cosm.com
EthernetClient client;
uint32_t timer;

uint8_t egg_bus_address;
float i_scaler = 0.0;
uint32_t r0 = 0;
uint32_t measured_value = 0;

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000; //delay between updates to Cosm.com


void setup() {
  Serial.begin(9600);
  Serial.println(F("Air Quality Egg Shield to Cosm - v0.0.1"));
  Serial.println(F("======================================="));

  Serial.println(F("Initializing network"));
  Serial.println(F("---------------------------------------"));

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) 
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    // DHCP failed, so use a fixed IP address:
    Ethernet.begin(mac, ip);
  }

  Serial.println(F("---------------------------------------"));
}

void loop() {

  if (client.available()) 
  {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) 
  {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if((millis() - lastConnectionTime > postingInterval))
  {
    sendData();
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}


void sendData()
{
  // if there's a successful connection:
  if (client.connect(server, 80)) 
  {
    Serial.println("connecting at Cosm...");
    // send the HTTP PUT request:
    client.print("PUT /v2/feeds/");
    client.print(FEEDID);
    client.println(".csv HTTP/1.1");
    client.println("Host: api.cosm.com");
    client.print("X-ApiKey: ");
    client.println(APIKEY);
    client.print("User-Agent: ");
    client.println(USERAGENT);
    client.print("Content-Length: ");

    // calculate the length of the sensor reading in bytes:
    // 8 bytes for "sensor1," + number of digits of the data:
    float temp = getTemperature();
    int thisLength = 64;
    client.println(thisLength);

    // last pieces of the HTTP PUT request:
    client.println("Content-Type: text/csv");
    client.println("Connection: close");
    client.println();
      
      //AQE
      eggBus.init();
    
      // Capture the gas sensors from the egg bus
      // Note we're not capturing the units or resistance of the sensors here
      while((egg_bus_address = eggBus.next())) {
        uint8_t numSensors = eggBus.getNumSensors();
        for(uint8_t ii = 0; ii < numSensors; ii++) 
        {
          //CSV print
          client.print(eggBus.getSensorType(ii));
          client.print(", ");
          client.println(eggBus.getSensorValue(ii), 4);
          
          //Serial print
          Serial.print(eggBus.getSensorType(ii));
          Serial.print(", ");
          Serial.println(eggBus.getSensorValue(ii), 4);
        }
      }
      
       // here's the actual content of the PUT request:
      client.print("temperature, ");
      client.println(temp);
      client.print("humidity, ");
      client.println(getHumidity());
      client.print("random, ");
      client.println("23");
      
      Serial.print("temperature: ");
      Serial.println(temp);
      Serial.print("humidity: ");
      Serial.println(getHumidity());
      Serial.print("random, ");
      Serial.println("23");
  } 
  else 
  {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println();
    Serial.println("disconnecting.");
    
    client.stop();
  }
   // note the time that the connection was made or attempted:
  lastConnectionTime = millis();

}


//--------- DHT22 humidity ---------
float getHumidity(){
  float h = dht.readHumidity();
  if (isnan(h)){
    //failed to get reading from DHT
    delay(2500);
    h = dht.readHumidity();
    if(isnan(h)){
      return -1;
    }
  }
  else return h;
}

//--------- DHT22 temperature ---------
float getTemperature()
{
  float t = dht.readTemperature();
  if (isnan(t)){
    //failed to get reading from DHT
    delay(2500);
    t = dht.readTemperature();
    if(isnan(t)){
      return -1;
    }
  }
  return t;
}
