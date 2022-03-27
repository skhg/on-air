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
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>
#include "./glyphs.h"
#include <DS3232RTC.h>

#define LED_HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define LED_COMPONENT_MODULES 4
#define LED_DIN 13  // nodemcu v3 pin D7
#define LED_CS 2  // nodemcu v3 pin D4
#define LED_CLK 14  // nodemcu v3 pin D5
#define  MARQUEE_STRING_MAX_LENGTH 255

const String STATIC_CONTENT_INDEX_LOCATION =
"http://jackhiggins.ie/on-air/";

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
const int HTTP_PAYLOAD_TOO_LARGE = 413;

const String METHOD_NOT_ALLOWED_MESSAGE = "Method Not Allowed";

const int SENSOR_READ_INTERVAL_MILLIS = 10000;
const int RANDOM_PIXEL_INTERVAL_MILLIS = 10;
const int CLOCK_SEPARATOR_INTERVAL_MILLIS = 1000;
const int _ledBrightness = 1;  // Max 15

const textEffect_t SCROLL_EFFECT = PA_SCROLL_LEFT;
const textPosition_t SCROLL_ALIGN = PA_LEFT;
const uint16_t SCROLL_PAUSE = 0;
const uint8_t SCROLL_MS_BETWEEN_FRAMES = 50;

DS3232RTC RTC;
MD_Parola LED_DISPLAY = MD_Parola(LED_HARDWARE_TYPE, LED_CS, LED_COMPONENT_MODULES);
WiFiClient WIFI_CLIENT;
HTTPClient HTTP_CLIENT;
ESP8266WebServer HTTP_SERVER(80);
WebSocketsServer WEB_SOCKET_SERVER(81);

/**
 * States and event types
 */
enum MODES {
  OFF,
  RANDOM_PIXELS,
  CLOCK,
  MARQUEE
};

float _clockTemperature = 0.0;
uint64_t _currentMillis = millis();
uint64_t _sensorReadMillis = millis();
uint64_t _randomPixelMillis = millis();

MODES _activeMode = CLOCK;
boolean _modeChanged = true;

bool _zoomAlertActive = true;
bool _zoomCallInProgress = false;

char _marqueeMessage[MARQUEE_STRING_MAX_LENGTH] = { "Hello world!" };

/**
 * Application functions
 */

void sendToWebSocketClients(String webSocketMessage) {
  WEB_SOCKET_SERVER.broadcastTXT(webSocketMessage);
}

void readSensors() {
  _currentMillis = millis();

  if (_currentMillis - _sensorReadMillis >
        SENSOR_READ_INTERVAL_MILLIS) {
    _sensorReadMillis = _currentMillis;

    // read sensors and set the values as global vars
    _clockTemperature = RTC.temperature() / 4.;

    sendToWebSocketClients(stateJson());
  }
}

void renderScreen() {
  if (_zoomAlertActive && _zoomCallInProgress) {
    renderOnAirSign();
    return;
  }

  switch (_activeMode) {
    case OFF: renderBlankScreen(); break;
    case RANDOM_PIXELS: renderRandomPixels(); break;
    case CLOCK: renderClock(); break;
    case MARQUEE: renderMarquee(); break;
    default: break;
  }

  _modeChanged = false;
}

void renderMarquee() {
  if (_modeChanged) {
    clearScreen();
    LED_DISPLAY.displayText(_marqueeMessage, SCROLL_ALIGN, SCROLL_MS_BETWEEN_FRAMES, SCROLL_PAUSE, SCROLL_EFFECT, SCROLL_EFFECT);
  }

  if (LED_DISPLAY.displayAnimate()) {
    LED_DISPLAY.displayReset();
  }
}

void renderClock() {
  time_t t = now();
  int currentHour = hour(t);
  int currentMinute = minute(t);
  int currentSecond = second(t);

  int digit1 = currentHour / 10;
  int digit2 = currentHour % 10;

  int digit3 = currentMinute / 10;
  int digit4 = currentMinute % 10;

  print8x8(3, getGlyphForInteger(digit1));
  print8x8(2, getGlyphForInteger(digit2), currentSecond % 2 == 0 ? GLYPH_CLOCK_SEPARATOR : GLYPH_EMPTY);
  print8x8(1, getGlyphForInteger(digit3));
  print8x8(0, getGlyphForInteger(digit4));
}

void renderBlankScreen() {
  clearScreen();
}

void renderOnAirSign() {
  print8x8(3, GLYPH_ON_AIR_BLOCK_0);
  print8x8(2, GLYPH_ON_AIR_BLOCK_1);
  print8x8(1, GLYPH_ON_AIR_BLOCK_2);
  print8x8(0, GLYPH_ON_AIR_BLOCK_3);
}

void renderRandomPixels() {
  _currentMillis = millis();

  if (_currentMillis - _randomPixelMillis <= RANDOM_PIXEL_INTERVAL_MILLIS) {
    return;
  }

  _randomPixelMillis = _currentMillis;

  int row = random(8);
  int column = random(8 * LED_COMPONENT_MODULES);
  int state = random(2);

  LED_DISPLAY.getGraphicObject()->setPoint(row, column, state);
}

void print8x8(int screenId, const byte pixels1[]) {
  print8x8(screenId, pixels1, GLYPH_EMPTY);
}

void print8x8(int screenId, const byte pixels1[], const byte pixels2[]) {
  for (int i = 0; i < 8; i++) {
    byte combo = B00000000;

    for (int j=0; j < 8; j++) {
      boolean outcome = bitRead(pixels1[i], j) || bitRead(pixels2[i], j);
      bitWrite(combo, j, outcome);
    }

    LED_DISPLAY.getGraphicObject()->setRow(screenId, screenId, i, combo);
  }
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
    HTTP_SERVER.send(HTTP_OK, CONTENT_TYPE_APPLICATION_JSON, stateJson());
  }
}

String stateJson() {
  String jsonContent;

  StaticJsonDocument<512> statusJson;
  statusJson["mode"] = modeToString(_activeMode);
  statusJson["message"] = _marqueeMessage;
  statusJson["temperature"] = _clockTemperature;

  JsonObject zoomJson = statusJson.createNestedObject("zoom");
  zoomJson["alert-active"] = _zoomAlertActive;
  zoomJson["call-in-progress"] = _zoomCallInProgress;

  serializeJson(statusJson, jsonContent);

  return jsonContent;
}

void messageHttpEventHandler() {
  if (HTTP_SERVER.method() == HTTP_PUT) {
    if (HTTP_SERVER.hasArg("plain") == false) {
      HTTP_SERVER.send(HTTP_BAD_REQUEST, CONTENT_TYPE_TEXT_PLAIN,
        "Missing body");
    } else {
      StaticJsonDocument<512> newMessageJson;
      DeserializationError err = deserializeJson(newMessageJson, HTTP_SERVER.arg("plain"));

      if (err) {
        Serial.println("Deserialising new message failed!");
        Serial.println(err.c_str());
        HTTP_SERVER.send(HTTP_BAD_REQUEST, CONTENT_TYPE_TEXT_PLAIN,
        "Invalid message");
        return;
      }

      String possibleNewMessage = newMessageJson["value"];
      if (possibleNewMessage.length() > MARQUEE_STRING_MAX_LENGTH) {
        HTTP_SERVER.send(HTTP_PAYLOAD_TOO_LARGE, CONTENT_TYPE_TEXT_PLAIN,
        "Message too long. Must be 255 characters or less.");
        return;
      }

      strcpy(_marqueeMessage, newMessageJson["value"]);

      _modeChanged = true;
      HTTP_SERVER.send(HTTP_NO_CONTENT, CONTENT_TYPE_TEXT_PLAIN, EMPTY_STRING);
      sendToWebSocketClients(stateJson());
    }
  } else if (HTTP_SERVER.method() == HTTP_DELETE) {
    _activeMode = OFF;
    HTTP_SERVER.send(HTTP_NO_CONTENT, CONTENT_TYPE_TEXT_PLAIN, EMPTY_STRING);
    sendToWebSocketClients(stateJson());
  } else {
    HTTP_SERVER.send(HTTP_METHOD_NOT_ALLOWED, CONTENT_TYPE_TEXT_PLAIN,
      METHOD_NOT_ALLOWED_MESSAGE);
  }
}

void modeHttpEventHandler() {
  if (HTTP_SERVER.method() == HTTP_PUT) {
    if (HTTP_SERVER.hasArg("plain") == false) {
      HTTP_SERVER.send(HTTP_BAD_REQUEST, CONTENT_TYPE_TEXT_PLAIN,
        "Missing body");
    } else {
      StaticJsonDocument<48> newModeJson;
      DeserializationError err = deserializeJson(newModeJson, HTTP_SERVER.arg("plain"));

      if (err) {
        Serial.println("Deserialising new mode failed!");
        Serial.println(err.c_str());
        HTTP_SERVER.send(HTTP_BAD_REQUEST, CONTENT_TYPE_TEXT_PLAIN,
        "Invalid mode");
        return;
      }

      MODES newMode = stringToMode(newModeJson["name"]);
      if (newMode != _activeMode) {
        _activeMode = newMode;
        _modeChanged = true;
        sendToWebSocketClients(stateJson());
      }
      
      HTTP_SERVER.send(HTTP_NO_CONTENT, CONTENT_TYPE_TEXT_PLAIN, EMPTY_STRING);
    }
  } else if (HTTP_SERVER.method() == HTTP_DELETE) {
    _activeMode = OFF;
    HTTP_SERVER.send(HTTP_NO_CONTENT, CONTENT_TYPE_TEXT_PLAIN, EMPTY_STRING);
    sendToWebSocketClients(stateJson());
  } else {
    HTTP_SERVER.send(HTTP_METHOD_NOT_ALLOWED, CONTENT_TYPE_TEXT_PLAIN,
      METHOD_NOT_ALLOWED_MESSAGE);
  }
}

MODES stringToMode(String mode) {
  if (mode == "random-pixels") {
    return RANDOM_PIXELS;
  } else if (mode == "clock") {
    return CLOCK;
  } else if (mode == "marquee") {
    return MARQUEE;
  } else {
    return OFF;
  }
}

String modeToString(MODES mode) {
  switch (mode) {
    case OFF: return "off";
    case RANDOM_PIXELS: return "random-pixels";
    case CLOCK: return "clock";
    case MARQUEE: return "marquee";
    default: return "Unknown";
  }
}

void zoomAlertHttpEventHandler() {
  if (HTTP_SERVER.method() == HTTP_PUT) {
    _zoomAlertActive = true;
    HTTP_SERVER.send(HTTP_NO_CONTENT, CONTENT_TYPE_TEXT_PLAIN, EMPTY_STRING);
    sendToWebSocketClients(stateJson());
  } else if (HTTP_SERVER.method() == HTTP_DELETE) {
    _zoomAlertActive = false;
    HTTP_SERVER.send(HTTP_NO_CONTENT, CONTENT_TYPE_TEXT_PLAIN, EMPTY_STRING);
    sendToWebSocketClients(stateJson());
  } else {
    HTTP_SERVER.send(HTTP_METHOD_NOT_ALLOWED, CONTENT_TYPE_TEXT_PLAIN,
      METHOD_NOT_ALLOWED_MESSAGE);
  }
}

void zoomCallHttpEventHandler() {
  if (HTTP_SERVER.method() == HTTP_PUT) {
    _zoomCallInProgress = true;
    HTTP_SERVER.send(HTTP_NO_CONTENT, CONTENT_TYPE_TEXT_PLAIN, EMPTY_STRING);
    sendToWebSocketClients(stateJson());
  } else if (HTTP_SERVER.method() == HTTP_DELETE) {
    _zoomCallInProgress = false;
    HTTP_SERVER.send(HTTP_NO_CONTENT, CONTENT_TYPE_TEXT_PLAIN, EMPTY_STRING);
    sendToWebSocketClients(stateJson());
  } else {
    HTTP_SERVER.send(HTTP_METHOD_NOT_ALLOWED, CONTENT_TYPE_TEXT_PLAIN,
      METHOD_NOT_ALLOWED_MESSAGE);
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

void clearScreen() {
  LED_DISPLAY.setIntensity(_ledBrightness);
  LED_DISPLAY.displayClear();
}

/**
 * Essential system functions
 */

void setup(void) {
  randomSeed(analogRead(0));
  Serial.begin(115200);

  setSyncProvider(RTC.get);

  // todo handle RTC initialisation failure

  LED_DISPLAY.begin();
  clearScreen();

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
  HTTP_SERVER.on("/api/mode", modeHttpEventHandler);
  HTTP_SERVER.on("/api/message", messageHttpEventHandler);
  HTTP_SERVER.on("/api/alert/zoom", zoomAlertHttpEventHandler);
  HTTP_SERVER.on("/api/alert/zoom/call", zoomCallHttpEventHandler);

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
  renderScreen();
}
