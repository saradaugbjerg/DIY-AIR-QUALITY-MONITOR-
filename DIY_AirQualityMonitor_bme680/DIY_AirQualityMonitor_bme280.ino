/*
  Example, how to read data from a BME680 (here: GY-BME680 5V/3.3V) and display the values on a HELTEC WIFI Kit 32 display
  by 3KU_Delta (https://github.com/3KUdelta)

  IAQ calculations following Dr. Julie Riggs, The IAQ Rating Index, www.iaquk.org.uk

  Weighting: Temperature, Humidity each one tenth of the rating ==> 6.5 points max each
  Giving gas resistance readings 8 tenths of the rating ==> 52 points max

  CREDITS:
  Basic solution provided by itofficeeu https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/issues/62 because Pin21 and Pin22
  do not work for this purpose.

  /***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor
  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/XXXX
  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************


  General Adafruit Sensor (Arduino Library Manager)
  Adafruit BME680 (https://github.com/adafruit/Adafruit_BME680)

*/

/* ---- Modified by Sara Daugbjerg, for FabLab RUC ---- */ 


#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <FastLED.h>

#define TEMP_CORR (-4)  //Calibration offset - calibrate yourself the temp reading --> humidity will \
                        //be automatically adjusted using the August-Roche-Magnus approximation \
                        //http://bmcnoldy.rsmas.miami.edu/Humidity.html



Adafruit_BME680 bme;  // I2C initializing BME680


float measured_temp;  //raw sensor measurement for temperature
float measured_humi;  //raw sensor measurement for humidity

float DewpointTemperature;
float adjusted_temp;
float adjusted_humi;

float hum_score, gas_score, temp_score;

#define NUM_LEDS 16
#define DATA_PIN D5

CRGB leds[NUM_LEDS];

CRGB color;

void setup() {
  Serial.begin(115200);
  Serial.printf("Starting...\r\n");
  while (!Serial)
    ;
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {

    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1)
      ;
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);  // 320°C for 150 ms

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
}

void loop() {
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");

    return;
  }
  // Print temperature
  measured_temp = bme.temperature;
  // print on serial monitor
   Serial.print("Raw Temp from Sensor: ");
   Serial.print(measured_temp);
   Serial.println("; ");

  // Print humidity
  measured_humi = bme.humidity;
  // print on serial monitor
   Serial.print("Raw humidity from Sensor: ");
   Serial.print(measured_humi);
   Serial.println("; ");

  // Calculate dewpoint
  double a_dp = 17.271;  //constants needed for Dewpoint calculations
  double b_dp = 237.7;
  double tempcalc_dp;
  tempcalc_dp = (a_dp * measured_temp) / (b_dp + measured_temp) + log(measured_humi * 0.01);
  DewpointTemperature = (b_dp * tempcalc_dp) / (a_dp - tempcalc_dp);
  // Serial.print("Dewpoint: ");
  // Serial.print(DewpointTemperature);
  // Serial.println("°C; ");

  adjusted_temp = measured_temp + TEMP_CORR;
  Serial.print("Adjusted temperature: ");
  Serial.print(adjusted_temp);
  Serial.println("; ");

  //August-Roche-Magnus approximation (http://bmcnoldy.rsmas.miami.edu/Humidity.html)
  adjusted_humi = 100 * (exp((17.625 * DewpointTemperature) / (243.04 + DewpointTemperature)) / exp((17.625 * adjusted_temp) / (243.04 + adjusted_temp)));
  Serial.print("Adjusted humidity: ");
  Serial.print(adjusted_humi);
  Serial.println("; ");



  // Print read gas
  uint32_t gas = bme.gas_resistance;
  // print on serial monitor
  // Serial.print("Gas raw: ");
  // Serial.print(gas / 1000.0);
  // Serial.println(" kOhms; ");


  //Calculate humidity score
  if (adjusted_humi >= 40 && adjusted_humi <= 60) hum_score = 6.5;                                                       //ideal condition, full points
  else if ((adjusted_humi >= 30 && adjusted_humi < 40) || (adjusted_humi > 60 && adjusted_humi <= 70)) hum_score = 5.2;  //20% less
  else if ((adjusted_humi >= 20 && adjusted_humi < 30) || (adjusted_humi > 70 && adjusted_humi <= 80)) hum_score = 3.9;  //40% less
  else if ((adjusted_humi >= 10 && adjusted_humi < 20) || (adjusted_humi > 80 && adjusted_humi <= 90)) hum_score = 2.6;  //60% less
  else if (adjusted_humi < 10 && adjusted_humi > 90) hum_score = 1.3;                                                    //80% less


  //Calculate temperature score
  int compare_temp = adjusted_temp;                                     // round to full degree
  if (compare_temp >= 18 && compare_temp <= 22) temp_score = 6.5;       //ideal condition, full points
  else if (compare_temp == 17 || compare_temp == 23) temp_score = 5.2;  //20% less
  else if (compare_temp == 16 || compare_temp == 24) temp_score = 3.9;  //40% less
  else if (compare_temp == 15 || compare_temp == 25) temp_score = 2.6;  //60% less
  else if (compare_temp == 14 || compare_temp == 26) temp_score = 1.3;  //80% less
  else if (compare_temp < 15 || compare_temp > 26) temp_score = 0;      //100% less


  //Calculate gas score on resistance values - best practices
  if (gas >= 430000) gas_score = 52;  //ideal condition, full points
  else if (gas >= 210000 && gas < 430000) gas_score = 43;
  else if (gas >= 100000 && gas < 210000) gas_score = 35;
  else if (gas >= 55000 && gas < 100000) gas_score = 26;
  else if (gas >= 27000 && gas < 55000) gas_score = 18;
  else if (gas >= 13500 && gas > 9000) gas_score = 9;
  else if (gas < 9000) gas_score = 0;

  calc_IAQ(temp_score + hum_score + gas_score);


    // Set the color for all LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = color;
      FastLED.show();
    }
}

String calc_IAQ(float score) {
  String IAQ = "Air quality is ";
  if (score < 25) {
    IAQ += "Hazardous";
    color = CRGB(254, 0, 2);
  } else if (score >= 25 && score <= 38) {
    IAQ += "Unhealthy";
    color = CRGB(254, 186, 1);
  } else if (score > 38 && score <= 51) {
    IAQ += "Moderate";
    color = CRGB(255, 242, 42);
  } else if (score > 51 && score <= 60) {
    IAQ += "Good";
    color = CRGB(120, 255, 10);
  } else if (score > 60 && score <= 65) {
    IAQ += "Excellent";
    color = CRGB(30, 255, 0);
  }
  Serial.println("Total IAQ score: " + String(score));
  Serial.println(IAQ);
  Serial.println("*****************************************************");
  return IAQ;

  delay(3000);
}