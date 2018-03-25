#include <ESP8266WiFi.h>

const char* ssid = "espIAQServer";
const char* password = "12345678";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Set esp8266 as Acess Point
  WiFi.mode(WIFI_AP);
  //Set esp8266 as softAP
  Serial.println("Configuring access point...");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  //Get softAp IP address
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  //Start server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // put your main code here, to run repeatedly:
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    Serial.println("No client");
    delay(1000);
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    Serial.println("Waiting data from client");
    delay(1000);
  }

  // Read the first line of the request
  String msg = client.readStringUntil('\n');
  if( msg == "getTimeNow" ){
    Serial.println("Message obtained from client: " + msg);
    Serial.println("Sending time request to rpi3");
    Serial.println(msg);

    while(!Serial.available()){
      Serial.println("Waiting time response from rpi3");
      delay(1000);    
    }
    String rsp = Serial.readStringUntil('\n');
    Serial.println("Time received from rpi3: " + rsp);
    Serial.println("Writing time to client");
    client.println(rsp);
  }
  else{
    Serial.println("Received sensor data from client: "+ msg);
    Serial.println("Sending sensor data to rpi3 server");
    Serial.println("sendData");
    while(!Serial.available()){}
    String rspRpi = Serial.readStringUntil('\n');
    if(rspRpi == "sendDataOK"){
      Serial.println(msg);
      client.print("RECEIVED\n");
    }
  } 
  delay(1);
  Serial.println("Client disconnected");
     
}

