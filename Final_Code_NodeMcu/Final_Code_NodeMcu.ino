#include <ESP8266WiFi.h> // include the WiFi library
#include<SoftwareSerial.h>
#include <ArduinoJson.h>
//SoftwareSerial nodemcu (D5,D6);



//D6 = Rx & D5 = Tx
SoftwareSerial nodemcu(D6, D5);


const char* ssid = "MAMA"; // replace with your WiFi network name
const char* password = "mamamama"; // replace with your WiFi password

WiFiServer server(80); // create a server object and specify the port (80 is default for HTTP)

void setup() {
  Serial.begin(9600); // start serial communication at 115200 baud
  nodemcu.begin(9600);
  WiFi.begin(ssid, password); // connect to the WiFi network
  while (WiFi.status() != WL_CONNECTED) { // wait until the connection is successful
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  server.begin(); // start the server
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // print the IP address to the serial monitor
}
void loop() {
  WiFiClient client = server.available(); // listen for incoming client connections
StaticJsonBuffer<1000> jsonBuffer;  // static JsonBuffer with a capacity of 1000 bytes
  JsonObject& data = jsonBuffer.parseObject(nodemcu);

  if (data == JsonObject::invalid()) {
    //Serial.println("Invalid Json Object");
    jsonBuffer.clear();
    return;
  }

 Serial.println("JSON Object Recieved");
  Serial.print("Recieved Humidity:  ");
  float ph = data["pH"];
  Serial.println(ph);
  Serial.print("Recieved Temperature:  ");
  float temp = data["temperature"];
  Serial.println(temp);
    Serial.print("Recieved TDS:  ");
  float tds = data["tds"];
  Serial.println(tds);
  Serial.println("-----------------------------------------");

  
  
  if (client) { // if a client connects
    while (client.connected()) { // while the client is connected
      if (client.available()) { // if there is data available to read
        String request = client.readStringUntil('\r'); // read the request
        client.flush(); // flush the data from the client
        
        // check if the request is a GET request with the path "/data"
        if (request.indexOf("GET /data") != -1) {
          client.println("HTTP/1.1 200 OK"); // send a response
          client.println("Content-Type: text/html");
          client.println(""); // do not forget this one
          client.println("<h1>Hello from the NodeMCU!</h1>"); // send a message to the client
          break;
        }
      }
    }
    client.stop(); // close the client connection
  }
}
