/**
 * Copyright 2022 Jack Higgins : https://github.com/skhg
 * All components of this project are licensed under the MIT License.
 * See the LICENSE file for details.
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <home_wifi.h>
#include <ArduinoJson.h>
#include <ESPAsyncTCP.h>
#include <WebSocketsServer.h>

const String STATIC_CONTENT_INDEX_LOCATION =
"http://jackhiggins.ie/";

const String HOST_NAME = "on-air";

const String EMPTY_STRING = "";

const String CONTENT_TYPE_TEXT_PLAIN = "text/plain";
const String CONTENT_TYPE_APPLICATION_JSON = "application/json";
const String CONTENT_TYPE_TEXT_HTML = "text/html";

const int HTTP_OK = 200;
const int HTTP_NO_CONTENT = 204;
const int HTTP_NOT_FOUND = 404;
const int HTTP_METHOD_NOT_ALLOWED = 405;
const int HTTP_BAD_REQUEST = 400;

const String METHOD_NOT_ALLOWED_MESSAGE = "Method Not Allowed";

const int SENSOR_READ_INTERVAL_MILLIS = 10000;

WiFiClient WIFI_CLIENT;
HTTPClient HTTP_CLIENT;
ESP8266WebServer HTTP_SERVER(80);
WebSocketsServer WEB_SOCKET_SERVER(81);

uint64_t _currentMillis = millis();
uint64_t _sensorReadMillis = millis();

void sendToWebSocketClients(String webSocketMessage) {
  WEB_SOCKET_SERVER.broadcastTXT(webSocketMessage);
}

void readSensors() {
  _currentMillis = millis();

  if (_currentMillis - _sensorReadMillis >
        SENSOR_READ_INTERVAL_MILLIS) {
    _sensorReadMillis = _currentMillis;

    // read sensors and set the values as global vars

    sendToWebSocketClients(sensorValuesToJsonString());
  }
}

String sensorValuesToJsonString() {
  String content;
  StaticJsonDocument<JSON_OBJECT_SIZE(4)> sensorsJson;
  sensorsJson["field-name"] = 0;
  serializeJson(sensorsJson, content);
  return content;
}

void httpRootEventHandler() {
  if (HTTP_SERVER.method() != HTTP_GET) {
    HTTP_SERVER.send(HTTP_METHOD_NOT_ALLOWED, CONTENT_TYPE_TEXT_PLAIN,
      METHOD_NOT_ALLOWED_MESSAGE);
    return;
  }

  // Fetch content from source and forward it
  HTTP_CLIENT.begin(WIFI_CLIENT, STATIC_CONTENT_INDEX_LOCATION);
  HTTP_CLIENT.GET();

  HTTP_SERVER.sendHeader("Cache-Control", "no-cache");
  HTTP_SERVER.send(HTTP_OK, "text/html", HTTP_CLIENT.getString());
}

void httpNotFoundEventHandler() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += HTTP_SERVER.uri();
  message += "\nMethod: ";
  message += (HTTP_SERVER.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += HTTP_SERVER.args();
  message += "\n";
  for (uint8_t i = 0; i < HTTP_SERVER.args(); i++) {
    message += " " + HTTP_SERVER.argName(i) + ": " + HTTP_SERVER.arg(i) + "\n";
  }
  HTTP_SERVER.send(HTTP_NOT_FOUND, CONTENT_TYPE_TEXT_PLAIN, message);
}

void statusHttpEventHandler() {
  if (HTTP_SERVER.method() != HTTP_GET) {
    HTTP_SERVER.send(HTTP_METHOD_NOT_ALLOWED, CONTENT_TYPE_TEXT_PLAIN,
      METHOD_NOT_ALLOWED_MESSAGE);
  } else {
    String content;

    StaticJsonDocument<JSON_OBJECT_SIZE(8) + 1000> statusJson;
    statusJson["status-field"] = 0;
    serializeJson(statusJson, content);

    HTTP_SERVER.send(HTTP_OK, CONTENT_TYPE_APPLICATION_JSON, content);
  }
}

void webSocketEventHandler(uint8_t num, WStype_t type, uint8_t * payload,
  size_t length) {
  IPAddress ip = WEB_SOCKET_SERVER.remoteIP(num);

  switch (type) {
    case WStype_DISCONNECTED: {
      Serial.println("WebSocket client disconnected.");
      break;
    }
    case WStype_CONNECTED: {
      Serial.print("WebSocket client at ");
      Serial.print(ip);
      Serial.println(" connected.");
      break;
    }
  }
}

void setup(void) {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.hostname(HOST_NAME);

  Serial.println(EMPTY_STRING);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(EMPTY_STRING);
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("Access at http://");
  Serial.print(WiFi.localIP());
  Serial.print("/ or http://");
  Serial.print(HOST_NAME);
  Serial.println("/");

  HTTP_SERVER.on("/", httpRootEventHandler);

  HTTP_SERVER.on("/api/status", statusHttpEventHandler);

  HTTP_SERVER.onNotFound(httpNotFoundEventHandler);

  HTTP_SERVER.begin();
  Serial.println("HTTP server started");

  WEB_SOCKET_SERVER.begin();

  // Disconnect after a single failed heartbeat
  WEB_SOCKET_SERVER.enableHeartbeat(1000, 1000, 1);
  WEB_SOCKET_SERVER.onEvent(webSocketEventHandler);
}

void loop(void) {
  HTTP_SERVER.handleClient();
  readSensors();
}
