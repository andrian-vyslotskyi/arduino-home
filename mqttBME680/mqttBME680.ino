#include <WiFi101.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <WiFi101OTA.h>
#include <ArduinoLowPower.h>
#include "arduino_secrets.h"

#include "Adafruit_BME680.h"
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, SECRET_SSID, SECRET_PASS);
AdafruitIO_Feed *temperature = io.feed("temperature-mkr");
AdafruitIO_Feed *humidity = io.feed("humidity-mkr");
AdafruitIO_Feed *pressure = io.feed("pressure-mkr");
AdafruitIO_Feed *debugLog = io.feed("debug-log");

const char ssid[]   = SECRET_SSID;
const char pass[]   = SECRET_PASS;
const char otaPass[] = OTA_PASS;
const char otaName[] = OTA_NAME;

void setup() {
  Serial.begin(9600);

  setupWiFiConnection();
  WiFiOTA.begin(otaName, otaPass, InternalStorage);
  setupAdafruitIo();
  setupBME();
}

void loop() {
  enableBMEReading();

  WiFiOTA.poll();
  io.run();
  
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  double t = bme.temperature;
  double p = bme.pressure / 100.0;
  double h = bme.humidity;
  temperature->save(t);
  pressure->save(p);
  humidity->save(h);
 
  LowPower.deepSleep(30000);  //sleep for 30s
}

void enableBMEReading() {
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);
}

void setupAdafruitIo() {
  io.connect();

  while(io.status() < AIO_CONNECTED) {
    Serial.println(io.statusText());
    delay(10000);
  }

  Serial.println();
  Serial.println(io.statusText());
}

void setupWiFiConnection() {
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  WiFi.maxLowPowerMode();

  Serial.println("You're connected to the network");
  Serial.println();
}

void setupBME() {
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_1X);
  bme.setHumidityOversampling(BME680_OS_1X);
  bme.setPressureOversampling(BME680_OS_1X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
}
