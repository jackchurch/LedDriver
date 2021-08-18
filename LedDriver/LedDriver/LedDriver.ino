// Code and tutorial comes from:
// https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html

// File reading code comes from:
// https://techtutorialsx.com/2019/02/23/esp32-arduino-list-all-files-in-the-spiffs-file-system/

// Required Library for WebSocketsServer: 
// https://github.com/Links2004/arduinoWebSockets

#include <ESPmDNS.h>
#include <HTTP_Method.h>
#include <SPIFFS.h>
#include <Uri.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>

const char* ssid = "SSID NAME";
const char* password = "SSID PASSWORD";
String hostName = "led-spotlight";
const char* dnsName =
    "led-spotlight";  // Go to this in your web browser instead of an IP address.

WebServer server(80);            // Webserver creation on port TCP 80.
WebSocketsServer webSocket(81);  // Web Sockets Server creation on port TCP 81.

File fsUploadFile;  // File variable to store the recieved file temporarily.

#define LED_RED 26  // specify the pins with an RGB LED connected
#define LED_GREEN 27
#define LED_BLUE 25

// JC OTHER CODE ↓
// Red, green, and blue pins for PWM control
const int redPin = 26;    //
const int greenPin = 27;  //
const int bluePin = 25;   //

// Setting PWM frequency, channels and bit resolution
const int freq = 5000;
const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 2;
// Bit resolution 2^8 = 256
const int resolution = 8;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTsime = 2000;
// JC OTHER CODE ↑

void setup() {
  //  pinMode(LED_RED, OUTPUT);  // the pins with LEDs connected are outputs
  //  pinMode(LED_GREEN, OUTPUT);
  //  pinMode(LED_BLUE, OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(redChannel, freq, resolution);
  ledcSetup(greenChannel, freq, resolution);
  ledcSetup(blueChannel, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(redPin, redChannel);
  ledcAttachPin(greenPin, greenChannel);
  ledcAttachPin(bluePin, blueChannel);

  Serial.begin(115200);  // Start the Serial communication to send messages to
                         // the computer
  delay(10);
  Serial.println("\r\n");

  startWiFi();  // Start wifi and connect to ssid.

  startSPIFFS();  // Start file service and list all contents.

  startWebSocket();  // Start Web Sockets Ser er.

  startMDNS();  // Start mDNS responder.

  startServer();  // Start a HTTP server with a file read handler and an upload
                  // handler.
}

bool rainbow = false;  // The rainbow effect is turned off on startup

unsigned long prevMillis = millis();
int hue = 0;

int green = 0;
int red = 255;
int blue = 0;

void loop() {
  webSocket.loop();       // constantly check for websocket events
  server.handleClient();  // run the server

  int msMultiplier = 255;


  while (rainbow) {  // if the rainbow effect is turned on

  //Removed blue
  // Colour is Red
  //255, 0, 0

    while (green <= 254) {
      webSocket.loop();
      if (!rainbow) {
        break;
      }
      green++;
      blue = 0;
      setRainbow(red, green, blue);
    }
    
    //Added green
    //Colour is Yellow
    //255, 255, 0

    while (red >= 1) {
      if (!rainbow) {
        break;
      }
      red--;
      green = 255;
      setRainbow(red, green, blue);
    }

    //Removed red
    //Colour is Green
    //0, 255, 0

    while (blue <= 254) {
      if (!rainbow) {
        break;
      }
      blue++;
      red = 0;
      setRainbow(red, green, blue);
    }

    //Added blue
    //Colour is Light Blue
    //0, 255, 255

    while (green >= 1) {
      if (!rainbow) {
        break;
      }
      green--;
      blue = 255;
      setRainbow(red, green, blue);
    }

    //Removed green
    //Colour is Blue
    //0, 0, 255

    while (red <= 254) {
      if (!rainbow) {
        break;
      }
      red++;
      green = 0;
      setRainbow(red, green, blue);
    }    

    //Added red
    //Colour is Magenta
    //255, 0, 255

    while (blue >= 1) {
      if (!rainbow) {
        break;
      }
      blue--;
      red = 255;
      setRainbow(red, green, blue);
    }

    //Removed blue
    //Colour is Red
    //255, 0, 0

  }
}

void startWiFi() {  // Start a Wi-Fi access point, and try to connect to some
                    // given access points. Then wait for either an AP or STA
                    // connection
                    // Connect to wifi network.

  WiFi.setHostname(hostName.c_str());
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to: ");
  Serial.println(ssid);
  // Print IP address.
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void startSPIFFS() {  // Start the SPIFFS and list all contents
  SPIFFS.begin();     // Start the SPI Flash File System (SPIFFS)
  Serial.println("SPIFFS started. Contents:");
  {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {  // List the file system contents
      Serial.println(file.name());
      file = root.openNextFile();
    }
    Serial.println("\n");
  }
}

void startWebSocket() {  // Start a WebSocket server
  webSocket.begin();     // start the websocket server
  webSocket.onEvent(
      webSocketEvent);  // if there's an incomming websocket message, go to
                        // function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startMDNS() {  // Start the mDNS responder
  // MDNS setup.
  if (!MDNS.begin(dnsName)) {
    Serial.println("Error starting MDNS. ");
  } else {
    Serial.print("MDNS name: ");
    Serial.println(dnsName);
  }
}

void startServer() {  // Start a HTTP server with a file read handler and an
                      // upload handler
  server.on(
      "/edit.html", HTTP_POST,
      []() {  // If a POST request is sent to the /edit.html address,
        server.send(200, "text/plain", "");
      },
      handleFileUpload);  // go to 'handleFileUpload'

  server.onNotFound(handleNotFound);  // if someone requests any other file or
                                      // page, go to function 'handleNotFound'
                                      // and check if the file exists

  server.begin();  // start the HTTP server
  Serial.println("HTTP server started.");
}

void handleNotFound() {  // if the requested file or page doesn't exist, return
                         // a 404 not found error
  if (!handleFileRead(server.uri())) {  // check if the file exists in the flash
                                        // memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
}

bool handleFileRead(
    String path) {  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
    path += "index.html";  // If a folder is requested, send the index file
  String contentType = getContentType(path);  // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) ||
      SPIFFS.exists(path)) {  // If the file exists, either as a compressed
                              // archive, or normal
    if (SPIFFS.exists(pathWithGz))  // If there's a compressed version available
      path += ".gz";                // Use the compressed verion
    File file = SPIFFS.open(path, "r");  // Open the file
    size_t sent =
        server.streamFile(file, contentType);  // Send it to the client
    file.close();                              // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") +
                 path);  // If the file doesn't exist, return false
  return false;
}

void handleFileUpload() {  // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {  // The file server always prefers a compressed
                                  // version of a file
      String pathWithGz = path + ".gz";  // So if an uploaded file is not
                                         // compressed, the existing compressed
      if (SPIFFS.exists(pathWithGz))     // version of that file must be deleted
                                         // (if it exists)
        SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: ");
    Serial.println(path);
    fsUploadFile =
        SPIFFS.open(path, "w");  // Open the file for writing in SPIFFS (create
                                 // if it doesn't exist)
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(
          upload.buf,
          upload.currentSize);  // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {      // If the file was successfully created
      fsUploadFile.close();  // Close the file again
      Serial.print("handleFileUpload Size: ");
      Serial.println(upload.totalSize);
      server.sendHeader(
          "Location",
          "/success.html");  // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload,
                    size_t lenght) {  // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:  // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {  // if a new websocket connection is established
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0],
                    ip[1], ip[2], ip[3], payload);
      rainbow = false;  // Turn rainbow off when a new connection is established
    } break;
    case WStype_TEXT:  // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
      if (payload[0] == '#') {  // we get RGB data
        uint32_t rgb = (uint32_t)strtol((const char*)&payload[1], NULL,
                                        16);  // decode rgb data
        int r = ((rgb >> 20) & 0x3FF);  // 10 bits per color, so R: bits 20-29
        int g = ((rgb >> 10) & 0x3FF);  // G: bits 10-19
        int b = rgb & 0x3FF;            // B: bits  0-9

        // JC OTHER CODE ↓
        ledcWrite(redChannel,
                  r);  // Write the right color to the LED output pins
        ledcWrite(greenChannel, g);
        ledcWrite(blueChannel, b);
        // JC OTHER CODE ↑
      } else if (payload[0] == 'R') {  // the browser sends an R when the
                                       // rainbow effect is enabled
        rainbow = true;
      } else if (payload[0] == 'N') {  // the browser sends an N when the
                                       // rainbow effect is disabled
        rainbow = false;
      }
      break;
  }
}

String formatBytes(size_t bytes) {  // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename) {  // determine the filetype of a given
                                          // filename, based on the extension
  if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}

void setRainbow(int red, int green, int blue) {
  ledcWrite(redChannel, red);
  ledcWrite(greenChannel, green);
  ledcWrite(blueChannel, blue);
  delay(32);
}
