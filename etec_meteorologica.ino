/*********************************************
 **********                         **********
 **********      MeteoroLógica      **********
 **********                         **********
 *********************************************
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

#define DEBUG 1          // 1 modo debug, 0 não debug


/*** Inicializa Objetos e Variáveis ***/

// DHT22 sensor
DHT_Unified dht(DHTPIN, DHTTYPE);

// BMP180 sensor
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// HC-SR04 sensor
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// ML8511 sensor
ML8511 uvSensor;

// String para concatenar informação dos sensores e enviar pro Serial ou
// salvar no Data Logger
String dataString = "";


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

  dataString = "";

  read_DHT22();     // Temperatura e Umidade
  read_BMP180();    // Temperatura e Pressão
  read_HCSR04();    // Distância
  read_ML8511();    // Radiação Ultravioleta

  Serial.println(dataString);

  // Abre o arquivo no cartão de memória
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
  else {
    Serial.println("Error:opening_datalog.txt");
  }

  delay(5000);      // Frequencia das Leituras
}


/*** DHT22 Umidade e Temperatura ***/

void read_DHT22 (void) {

  float humidity = 0.0;
  float temperature = 0.0;
  sensors_event_t event;

  dht.humidity().getEvent(&event);
  humidity = event.relative_humidity;
  if (isnan(humidity)) {
    dataString += "DHT22_H:Error,";
  }
  else {
    dataString += "DHT22_H:" + String(humidity) + ",";
  }

  dht.temperature().getEvent(&event);
  temperature = event.temperature;
  if (isnan(temperature)) {
    dataString += "DHT22_T:Error,";
  }
  else {
    dataString += "DHT22_T:" + String(temperature) + ",";
  }
}


/*** BMP180 Temperatura e Pressão Barométrica ***/

void read_BMP180 (void) {

  float temperature = 0.0;
  float pressure = 0.0;

  sensors_event_t eventBMP;
  bmp.getEvent(&eventBMP);
  pressure = eventBMP.pressure;
  bmp.getTemperature(&temperature);

  if (isnan(temperature)) {
    dataString += "BMP180_T:Error,";
  }
  else {
    dataString += "BMP180_T:" + String(temperature) + ",";
  }

  if (isnan(pressure)) {
    dataString += "BMP180_P:Error,";
  }
  else {
    dataString += "BMP180_P:" + String(pressure) + ",";
  }
}


/*** HC-SR04 Sensor de Distância Ultrasónico ***/

void read_HCSR04 (void) {

  unsigned int uS = sonar.ping();        // Emite um pulso e retorna tempo em
                                         // microsegundos (uS)
  float distance = uS / US_ROUNDTRIP_CM; // Converte tempo em distância (cm).
                                         // Se distância maior do que MAX_DISTANCE retorna 0

  if (isnan(distance)) {
    dataString += "HC-SR04_D:Error,";
  }
  else {
    dataString += "HC-SR04_D:" + String(distance) + ",";
  }
}


/*** ML8511 Sensor de Radiação Ultravioleta ***/

void read_ML8511 (void) {

  float uv = uvSensor.readSensor();

  if (isnan(uv)) {
    dataString += "ML8511_UV:Error,";
  }
  else {
    //uvSensor.debugSensor();
    dataString += "ML8511_UV:" + String(uv) + ",";
  }
}
