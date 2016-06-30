/*** MeteoroLógica ***
 *
 * ## Deek-Robot Data Logging Shield V1.0
 *    SD-Card Reader (Arduino SD-Library compatible, chipSelect=10)
 *    https://www.arduino.cc/en/Reference/SD
 *    RTC DS1307 IC with battery (DS1307RTC Library compatible)
 *    http://playground.arduino.cc/code/time
 *
 * ## SD Card Module
 *        GND --> Ground (negativo)
 *        3.3V --> 3.3V           Warning (3.3V)!!!
 *        5V  --> Não conectar
 *        SDCS --> digital 4
 *        MOSI --> digital 11
 *        SCK  --> digital 13
 *        MISO --> digital 12
 *
 * ## DHT22 Basic Temperature-Humidity Sensor
 *    https://github.com/adafruit/DHT-sensor-library
 *        [1] VCC  --> 5V
 *        [2] DATA --> pino digital 2
 *        [3] NC   --> Não Conectar
 *        [4] GND  --> Ground (negativo)
 *
 * ## BMP180 Barometric Pressure/Temperature/Altitude Sensor
 *    https://github.com/adafruit/Adafruit\_BMP085\_Unified
 *        SDA --> analog 4
 *        SCL --> analog 5
 *        VIN --> 3.3V DC      Warning (3.3V)!!!
 *        GND --> Ground
 *
 * ## HC-SR04 Ultrasonic Sensor
 *    https://github.com/PaulStoffregen/NewPing
 *    https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home
 *        VCC  --> 5V
 *        Trig --> digital 12
 *        Echo --> digital 11
 *        GND  --> Ground (negativo)
 *
 * ## ML8511 Ultra Violet Sensor
 *    https://github.com/kl-git/ML8511
 *        EN  --> 3.3
 *        OUT --> analog A0
 *        GND --> Ground (negativo)
 *        3.3 --> 3.3V & analog A1    Warning (3.3V)!!!
 *
 */

/*** Bibliotecas Include ***/

#include <DHT_U.h>
#include <Adafruit_BMP085_U.h> // BMP180
#include <NewPing.h>           // HC-SR04
#include <ML8511.h>
#include <SPI.h>               // Serial Peripheral Interface Bus
#include <SD.h>                // Secure Digital (SD card format)
#include <Wire.h>              // I2C library

/*** Define Constantes ***/

#define DHTPIN 2
#define DHTTYPE DHT22

#define TRIGGER_PIN  12  // Gatilho - Emite um pulso
#define ECHO_PIN     11  // Eco - Recebe o pulso
#define MAX_DISTANCE 200 // Distância máxima em cm.

#define CS 4             // Pino digital Módulo SD Card

/*** Inicializa Objetos e Variáveis ***/

// DHT22 sensor
DHT_Unified dht(DHTPIN, DHTTYPE);

// BMP180 sensor
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// HC-SR04 sensor
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// ML8511 sensor
ML8511 uvSensor;

/*** Configuração Inicial ***/
void setup(void) {
  // Comunicação Serial
  Serial.begin(115200);   // Baud rate (velocidade de comunicação)
  while(!Serial);
  delay(1000);

  // Start DHT22 sensor
  dht.begin();

  // Start BMP180 sensor
  bmp.begin();

  // Start ML8511 sensor
  uvSensor.begin();

  // Start SD Card Module
  SD.begin(CS);
}

/*** Função Principal ***/
void loop(void) {
  String dataString = "";

  read_DHT22();     // Lê Temperatura e Umidade
  read_BMP180();    // Lê Temperatura e Pressão
  read_ML8511(dataString);    // Lê UV
  read_HCSR04(dataString);    // Lê Distância

  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    Serial.println(dataString);
  }
  else {
    Serial.println("Error opening datalog.txt");
  }

  Serial.println("--> END <--");

  delay(5000);      // Frequencia das Leituras
}

/*** DHT22 Umidade e Temperatura ***/

void read_DHT22(void) {
  float humidity = 0.0;
  float temperature = 0.0;
  sensors_event_t event;

  dht.humidity().getEvent(&event);
  humidity = event.relative_humidity;
  if (isnan(humidity)) {
    Serial.println("Error:DHT22_H_Sensor");
  }
  else {
    Serial.print("DHT22_H:");
    Serial.println(humidity);           // em %
  }

  dht.temperature().getEvent(&event);
  temperature = event.temperature;
  if (isnan(temperature)) {
    Serial.println("Error:DHT22_T_Sensor");
  }
  else {
    Serial.print("DHT22_T:");
    Serial.println(temperature);        // em ºC
  }
}

/*** BMP180 Temperatura e Pressão Barométrica ***/

void read_BMP180(void) {
  float temperature = 0.0;
  float pressure = 0.0;

  sensors_event_t eventBMP;
  bmp.getEvent(&eventBMP);
  pressure = eventBMP.pressure;
  bmp.getTemperature(&temperature);

  if (isnan(temperature)) {
    Serial.println("Error:BMP180_T_Sensor");
  }
  else {
    Serial.print("BMP180_T:");
    Serial.println(temperature);        // em Celsius
  }

  if (isnan(pressure)) {
    Serial.println("Error:BMP180_P_Sensor");
  }
  else {
    Serial.print("BMP180_P:");
    Serial.println(pressure);           // em hPa
  }
}

void read_HCSR04 (String dataString) {
  unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  float distance = uS / US_ROUNDTRIP_CM; // Convert ping time to distance in cm and print result (0 = outside set distance range)

  if (isnan(distance)) {
    Serial.print("Error:HC-SR04_D_Sensor");
  }
  else {
    Serial.print("HC-SR04_D:");
    Serial.println(distance);             // em Cm 
    dataString += String(distance) + ",";
  }
}

void read_ML8511 (String dataString) {
  float uv = uvSensor.readSensor();
  if (isnan(uv)) {
    Serial.println("Error:ML8511_UV_Sensor");
  }
  else {
    Serial.print("ML8511_UV:");
    Serial.println(uv);
    //uvSensor.debugSensor();
    dataString += String(uv) + ",";
  }
}

