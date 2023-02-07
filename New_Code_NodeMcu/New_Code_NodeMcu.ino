//ThatsEngineering
//Sending Data from Arduino to NodeMCU Via Serial Communication
//NodeMCU code
#include <ESP8266WiFi.h> // include the WiFi library
 //Include Lib for Arduino to Nodemcu
#include <SoftwareSerial.h>

#include <ESP8266HTTPClient.h>

#include <ArduinoJson.h>

#include <WiFiClientSecure.h>

#include <NTPClient.h>

#include <WiFiUdp.h>

//D6 = Rx & D5 = Tx
SoftwareSerial nodemcu(D6, D5);
WiFiClientSecure client;
WiFiServer server(80); // create a server object and specify the port (80 is default for HTTP)

const char * website = "https://worldtimeapi.org/api/timezone/Asia/Manila";

const char * ssid = "MAMA"; // replace with your WiFi network name
const char * password = "mamamama"; // replace with your WiFi password
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;
const char * serverName = "aquaponic.onrender.com";
const char * ss = "https://aquaponic.onrender.com/get_data";
String sensorReadings;
float sensorReadingsArr[3];
#define relayPin D8

const long utcOffsetInSeconds = 3600;

char daysOfTheWeek[7][12] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds);

void setup() {
  // Initialize Serial port
  //This is the Pin the LED is connected to
  Serial.begin(9600);
  nodemcu.begin(9600);

  pinMode(relayPin, OUTPUT);
  WiFi.begin(ssid, password); // connect to the WiFi network
  server.begin(); // start the server

  while (WiFi.status() != WL_CONNECTED) { // wait until the connection is successful
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // print the IP address to the serial monitor
  while (!Serial) continue;

  client.setFingerprint("6E E0 C4 58 F0 A5 DA 04 83 59 55 44 0F 5C 7F 05 8F 8C 98 F0");

}

void makePostRequest(float temperature, float ph, float tds,String led_status, float battery) {
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {

      HTTPClient http;

      http.begin(client, "https://aquaponic.onrender.com/api/send");

      http.addHeader("Content-Type", "application/json");
      
      String jsonPayload = "{\"battery_percentage\":\"" + String(battery) + "\",\"temperature\":\"" + String(temperature) + "\",\"ph_leveling\":\"" + String(ph) + "\",\"tds\":\"" + String(tds) + "\",\"led_status\":\"" + led_status + "\"}";

      int httpResponseCode = http.POST(jsonPayload);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      // Free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

void makeHTTPRequest() {

  HTTPClient https;

  https.begin(client, "https://aquaponic.onrender.com/api/get_data?result=6&start=0");

  int httpCode = https.GET();

  Serial.print("HTTP CODE: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String payload = https.getString();
    Serial.print("PAYLOAD: ");
    Serial.println(payload);
    StaticJsonBuffer < 1000 > jsonBuffer;
    JsonObject & data = jsonBuffer.parseObject(payload);

    String hum = data["battery_percentage"];
    Serial.println(hum);
  }

  https.end();
}

void test() {
 
}

void loop() {

  WiFiClient client2 = server.available();

  StaticJsonBuffer < 1000 > jsonBuffer; // static JsonBuffer with a capacity of 1000 bytes
  JsonObject & data = jsonBuffer.parseObject(nodemcu);

  if (data == JsonObject::invalid()) {
    //Serial.println("Invalid Json Object");
    jsonBuffer.clear();
    return;
  }
  timeClient.update();
  String isOpen = "false";
  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");

  Serial.print(timeClient.getHours() + 7);
  if (timeClient.getHours() + 7 <= 20 && timeClient.getHours() + 7 >= 17) {
    digitalWrite(D8, HIGH);
    Serial.println("20 OPEN");
    isOpen = "true";
  } else {

    digitalWrite(D8, LOW);
    Serial.println(" CLOSED");
    isOpen = "false";
  }
  Serial.println("JSON Object Recieved");
  Serial.print("Recieved ph:  ");
  float ph = data["pH"];
  Serial.println(ph);
  Serial.print("Recieved Temperature:  ");
  float temp = data["temperature"];
  Serial.println(temp);
  Serial.print("Recieved TDS:  ");
  float tds = data["tds"];
  Serial.println(tds);
   Serial.print("Recieved Battery:  ");
  float battery = data["battery"];
  Serial.println(battery);
  Serial.println("-----------------------------------------");
makePostRequest( temp,  ph,  tds,isOpen,battery);
}
