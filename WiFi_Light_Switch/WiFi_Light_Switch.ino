#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include "config.h"

extern const char* ssid;
extern const char* password;
extern char server_ip[];
extern const char* ota_password;
extern const char* ota_hostname;
extern const int ota_port;
String freemem = "";

int state = HIGH;
int reading;
int previous = LOW;

long timevar = 0;
long debounce = 200;

const int led = 4;
const int relay = 14;
const int button = 12;

ESP8266WebServer server(80);
WiFiClient client;
ESP8266HTTPUpdateServer httpUpdater;


void handleRoot() {
  digitalWrite(led, 1);
  freemem = ESP.getFreeHeap();
  server.send(200, "text/plain", "hello from esp8266! FreeMem: " + freemem + " Remote IP: " + WiFi.localIP());
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

//void eepromload(){ Чтение EEPROM памяти и применение параметров.
//  EEPROM.read(address);
//}

void setup(void) {
  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(button, INPUT);
  digitalWrite(led, 0);
  EEPROM.begin(1024);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.println("");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(10000);
    ESP.restart();
  }
  Serial.println("");
  Serial.print("Status: Connected ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  httpUpdater.setup(&server);
  server.begin();
  if (MDNS.begin(ota_hostname)) {
    Serial.println("MDNS responder started");
  }
  MDNS.addService("http", "tcp", 80);
  // OTA PARAMS:
  //ArduinoOTA.setPort(ota_port);
  ArduinoOTA.setHostname(ota_hostname);
  ArduinoOTA.setPassword(ota_password);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  server.on("/", handleRoot);
  server.on("/reset", []() {
    server.send(200, "text/html", "<html><body><form action='/resetyes'><input type='submit' value='Restart ESP' name=''></form></body></html>");
    delay(1000);
  });
  server.on("/resetyes", []() {
    ESP.reset();
  });
  server.on("/gpio", []() {
    int stat = server.arg("st").toInt();
    int gpionum = server.arg("pin").toInt();
    if (stat == 1) digitalWrite(gpionum, HIGH);
    else if (stat == 0) digitalWrite(gpionum, LOW);
    server.send(200, "text/plain", "GPIO: " + server.arg("pin") + " is now " + digitalRead(gpionum));
    Serial.println("GET GPIO: " + server.arg("pin") + " is now " + server.arg("st"));
    Serial.println("GPIO: " + server.arg("pin") + " is now " + digitalRead(gpionum));
  });
  server.onNotFound(handleNotFound);
  server.on("/light", []() {
    String turnlight = server.arg("turn");
    if (turnlight == "on") {
      state = HIGH;
    } else if (turnlight == "off") {
      state = LOW;
    }
    server.send(200, "text/html", "Light turn " + turnlight);
  });


  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  ArduinoOTA.handle();
//  reading = digitalRead(button);
//  if (reading == HIGH && previous == LOW && millis() - timevar > debounce) {
//    if (state == HIGH) {
//      state = LOW;
//        if (client.connect(server_ip, 80)) {
//          Serial.println("connected to server");
//          // Make a HTTP request:
//          client.println("GET //objects/?object=Lights-Room&op=set&p=Status&v=0 HTTP/1.1");
//          client.println("Host: WiFi-Light-Switch");
//          client.println("Connection: close");
//          client.println();
//        }
//    } else {
//       state = HIGH;
//        if (client.connect(server_ip, 80)) {
//          Serial.println("connected to server");
//          // Make a HTTP request:
//          client.println("GET //objects/?object=Lights-Room&op=set&p=Status&v=1 HTTP/1.1");
//          client.println("Host: WiFi-Light-Switch");
//          client.println("Connection: close");
//          client.println();
//        }
//      timevar = millis();
//    }
//  }
//  digitalWrite(relay, state);
//  previous = reading;
}

//void saveeeprom(int num, int val){
//    EEPROM.write(num, val);
//    EEPROM.commit();
//}
//void loadeeprom(int num, int val){
//    EEPROM.write(num, val);
//    EEPROM.commit();
//}
