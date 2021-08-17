#include <HTTP_Method.h>
#include <Uri.h>
#include <WebServer.h>

#include <ESPmDNS.h>

#include <WiFi.h>


const char* ssid = "Test";
const char* password = "TofuDinner";
String hostName = "Spotlight";
const char* dnsName = "spotlight";


//Webserver creation. 
WebServer server(80); 
//Function protoype for HTTP handlers (next 2 lines)
void handleRoot();
void handleNotFound();


void handleRoot() {
  server.send(200, "text/plain", "The server is doing okay. ");
}

void handleNotFound() {
  server.send(404, "text/plain", "404, The host is drunk. ");
}


void setup() {
  Serial.begin(115200);
  Serial.println("\n");

  // Connect to wifi network. 
  Serial.print("Connecting to");
  WiFi.setHostname(hostName.c_str());
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Print IP address. 
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address:");
  Serial.println(WiFi.localIP());

  // MDNS setup. 
  if(!MDNS.begin(dnsName)) {
    Serial.println("Error starting MDNS. ");
  } else {
    Serial.println("MDNS started. ");
  }

  //Start HTTP server
  server.on("/", handleRoot); //Call the handleroot function when a client requests the URI: "/". 
  server.onNotFound(handleNotFound); //When a client requests unknown URI. 
  server.begin();
  Serial.println("HTTP server has begun. ");

}

void loop() {
  server.handleClient(); //Listen for HTTP requests
}




