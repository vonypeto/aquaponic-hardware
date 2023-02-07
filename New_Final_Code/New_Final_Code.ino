#include <OneWire.h>

#include <DallasTemperature.h>
#include <SoftwareSerial.h>

#include <Wire.h>
#include <ArduinoJson.h>

SoftwareSerial nodemcu(5, 6);
 // static JsonBuffer with a capacity of 1000 bytes


#define TdsSensorPin A1
#define VoltageSensor A5

#define VREF 5.0 // analog reference voltage(Volt) of the ADC
#define SCOUNT 30 // sum of sample point
int analogBuffer[SCOUNT]; // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25, voltageValue = 0;


// Temperature State
#define ONE_WIRE_BUS 2 //pin for sensor


// Ph Leveling State
float calibration_value = 21.34 - 0.7;
int phval = 0;
unsigned long int avgval;
int buffer_arr[10], temp;
float ph_act;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors( & oneWire);

unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;
void setup() {
  Serial.begin(9600);

  nodemcu.begin(9600);
  delay(1000);

  Serial.println("Program started");
}




float calculatePH() {
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(A0);
   // delay(30);
  }
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }
  avgval = 0;
  for (int i = 2; i < 8; i++)
    avgval += buffer_arr[i];
  float volt = (float) avgval * 5.0 / 1024 / 6;
  ph_act = -5.70 * volt + calibration_value;

  Serial.print("pH Level: ");
  Serial.print(ph_act);
  Serial.println();

  return ph_act;
}

float getTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if (tempC != DEVICE_DISCONNECTED_C) {

    Serial.println();
    Serial.print("Celsius: ");
    Serial.print(tempC);
    Serial.print(" C");
    Serial.print(" |");
    Serial.print(" |");
    Serial.print(" Fahrenheit: ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.print(" F");
    Serial.println();

  } else {
    Serial.println("Error: Could not read temperature data");
  }
  return tempC;
}
void loop() {
  if ((millis() - lastTime) > timerDelay) {

  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();

  //Obtain Temp and Hum data
 
  tdsValue = analogRead(TdsSensorPin);

  Serial.print("TDS ");
  Serial.println(tdsValue);

  //Assign collected data to JSON Object
  float voltage = getVoltageValue();
  float pH = calculatePH();
 float tds = tdsValue;
  float tempC = getTemperature();
  Serial.println(voltage);
  data["battery"]= voltage;
  data["pH"] = pH;
  data["temperature"] = tempC; 
  data["tds"]=tds;

  data.printTo(nodemcu);
  jsonBuffer.clear();

  lastTime = millis();
  }
}










float getVoltageValue(){
    
   
  float  voltageData = analogRead(VoltageSensor);
 Serial.println("OLD Voltage" );
  Serial.println(voltageValue);
 
 voltageValue = (voltageData * 5.0) / 1024.0;
 // voltageValue = map(voltageData, 0, 1023, 0, 5);

  Serial.println("Voltage" );
  Serial.println(voltageValue);



  return voltageValue;
  }


float getTdsValue() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float) VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient; //temperature compensation
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
    //Serial.print("voltage:");
    //Serial.print(averageVoltage,2);
    //Serial.print("V   ");
    Serial.print("TDS Value:");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");

  }
  return tdsValue;
}
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}
