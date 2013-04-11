#include <SPI.h>
#include <Ethernet.h>
#include <TextFinder.h>
#include <YalerEthernetServer.h>
#include <DHT.h>
#include <stdlib.h>
#include "Wire.h"
#include "EggBus.h"

#define API_KEY "OOk5W0LuZF4qId5F2waoTd20DwuSAKxDMDJGLzZ3Yml0Zz0g" // your Cosm API key
#define FEED_ID 123333 // your Cosm feed ID

EggBus eggBus;
uint8_t egg_bus_address;
float i_scaler = 0.0;
uint32_t r0 = 0;
uint32_t measured_value = 0;

#define DHTPIN 17 // analog pin 3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);


char endOfHeaders[] = "\r\n\r\n";

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF }; // CHANGE THIS IF YOU'VE GOT MORE THAN ONE Arduino


EthernetClient client;


//EthernetServer server(80); // port 80
YalerEthernetServer server("try.yaler.net", 80, "gsiot-ypf7-s06d");
//YalerEthernetServer server("try.yaler.net", 80, "gsiot-vfn8-qwc0");

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in econds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000; //delay between updates to Cosm.com


void setup() 
{
  Serial.begin(9600);
  Serial.println("Acquiring IP address...");
  if (Ethernet.begin(mac) == 0) 
  {
    Serial.println("DHCP failed.");
  } 
  else 
  {
    Serial.println(Ethernet.localIP());
    server.begin();
  }
}

void sendResponse(EthernetClient client) 
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<h1>Arduino Yaler Air Quality Egg Web Service</h1>");
  client.println("<a href='/aqe.json'>GET AQE JSON</a><br><br>");
  client.println("<a href='/aqe.csv'>GET AQE CSV for Cosm</a><br><br><br>");
  client.println("Build by <a href='http://marcpous.com'>Marc Pous</a> and Yaler");
}

void sendRedirect(EthernetClient client) {
  client.println("HTTP/1.1 307 Temporary Redirect");
  client.println("Location: /");
  client.println();
}

void sendNotFound(EthernetClient client) {
  client.println("HTTP/1.1 404 Not Found");
  client.println();
}




void sendCSV(EthernetClient client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/csv");
  client.println();


  //Temperature
  float currentTemp = getTemperature();
  if (currentTemp != -1) {
      Serial.print("Temperature,");
      Serial.println(currentTemp, 2);
    }

  client.print("Temperature, ");
  client.println(currentTemp);
  
  //Humidity
  float currHumidity = getHumidity();
  if (currHumidity != -1) {
    Serial.print("Humidity,");
    Serial.println(currHumidity, 2);
  }

  client.print("Humidity, ");
  client.println(currHumidity);
  
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
      Serial.print(",");
      Serial.println(eggBus.getSensorValue(ii), 4);
    }
  }
}


void sendJSON(EthernetClient client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println();

  client.print("{");
  
  client.println("\"version\":\"1.0.0\",");
  client.println("\"datastreams\":[");
    
  //Temperature
  float currentTemp = getTemperature();
  if (currentTemp != -1) {
      Serial.print("Temperature,");
      Serial.println(currentTemp, 2);
    }

  client.print("{\"id\": \"temperature\", \"value\": \"");
  client.print(currentTemp);
  client.println("\"},");
  
  //Humidity
  float currHumidity = getHumidity();
  if (currHumidity != -1) {
    Serial.print("Humidity,");
    Serial.println(currHumidity, 2);
  }

  client.print("{\"id\": \"humidity\", \"value\": \"");
  client.print(currHumidity);
  client.println("\"},");
  
  //AQE
  eggBus.init();

  // Capture the gas sensors from the egg bus
  // Note we're not capturing the units or resistance of the sensors here
  while((egg_bus_address = eggBus.next())) {
    uint8_t numSensors = eggBus.getNumSensors();
    for(uint8_t ii = 0; ii < numSensors; ii++) {
      if (ii > 0) client.println(",");

      //JSON print
      client.print("{\"id\": \"");
      client.print(eggBus.getSensorType(ii));
      client.print("\", \"value\" : \"");
      client.print(eggBus.getSensorValue(ii), 4);
      client.print("\"}");
      
      //Serial print
      Serial.print(eggBus.getSensorType(ii));
      Serial.print(",");
      Serial.println(eggBus.getSensorValue(ii), 4);
    }
  }

  client.println("]");
  client.print("}");
}

void sendSerial()
{
  //Temperature
  float currentTemp = getTemperature();
  if (currentTemp != -1) {
      Serial.print("Temperature,");
      Serial.println(currentTemp, 2);
    }

  //Humidity
  float currHumidity = getHumidity();
  if (currHumidity != -1) {
    Serial.print("Humidity,");
    Serial.println(currHumidity, 2);
  }
  
  //AQE
  eggBus.init();

  // Capture the gas sensors from the egg bus
  // Note we're not capturing the units or resistance of the sensors here
  while((egg_bus_address = eggBus.next())) {
    uint8_t numSensors = eggBus.getNumSensors();
    for(uint8_t ii = 0; ii < numSensors; ii++) {
      //Serial print
      Serial.print(eggBus.getSensorType(ii));
      Serial.print(",");
      Serial.println(eggBus.getSensorValue(ii), 4);
    }
  }

}



void loop() {
    
  sendSerial();
  Serial.println("------------------");
    
  client = server.available();
  
  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  /*
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) 
  {
    sendSerial();
    Serial.print("------------------");
  }
  */

  if (client && client.connected()) 
  {    
    TextFinder finder(client); // helper to read request
    // Receive first line of HTTP request, which looks like this:
    // Request-Line = Method SP Request-URI SP HTTP-Version CRLF
    // see http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html
    char requestMethodBuffer[8];
    char requestUriBuffer[64];
    finder.getString("", " ", requestMethodBuffer, 8); // read to first SP
    finder.getString("", " ", requestUriBuffer, 64); // read on, to second SP
    String requestMethod = String(requestMethodBuffer);
    String requestUri = String(requestUriBuffer);
    Serial.print(requestMethod);
    Serial.print(" ");
    Serial.println(requestUri); 
    // receive HTTP request headers, ignore all except "Content-Length"
    int contentLength = 0;
    if(finder.findUntil("Content-Length", endOfHeaders)) 
    {
      contentLength = finder.getValue();
      finder.find("\n\r\n");
    }
    
   if (requestMethod.equals("GET") && requestUri.equals("/")) 
   {
      sendResponse(client);
   }
   else if(requestMethod.equals("GET") && requestUri.equals("/aqe.json"))
   {
      sendJSON(client);
   }
   else if(requestMethod.equals("GET") && requestUri.equals("/aqe.csv"))
   {
     sendCSV(client);
   }
   /*
   else if (requestMethod.equals("POST") && requestUri.equals("/") && (contentLength > 0)) {
      // Receive HTTP request content, assume "name=value" pairs, ordered as in the html
      for (int i = 0; i <= 13; i++) {
        finder.find("=");
        if (!pinUsedByEthernetShield(i)) {
          int dUnsafe = finder.getValue();
          int d = max(0, min(dUnsafe, 1)); // Force (potentially unsafe) value to legal range
          digitalWrite(i, d);
        }
      }
      sendResponse(client);
    } 
    */
    else if (requestMethod.equals("GET") && requestUri.equals("/index.html")) {
      sendRedirect(client);
    } 
    else {
      sendNotFound(client);
    }
    delay(1); // give the web browser time to receive the data
    client.stop();
  }
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
float getTemperature(){
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

