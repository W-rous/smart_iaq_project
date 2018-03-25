#include <ESP8266WiFi.h>

//Server softAP credentials
const char* ssid     = "espIAQServer";
const char* password = "12345678";

//Server IP address
byte server[] = {192,168,4,1};
//Server Port No.
const int httpPort = 80;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(10);

  /*Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);*/

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }

  //Serial.println("");
  //Serial.println("WiFi connected");  
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!Serial.available()){
    //Serial.println("Waiting data from sensor node");
    delay(1000);
    return; 
  }
  String msg = Serial.readStringUntil('\n');
  if(msg == "getTime"){
    /*Serial.println("Message obtained from sensor node: " + msg); 
    Serial.println("Sending time request to server");
    Serial.print("connecting to server");*/

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect(server, httpPort)) {
      //Serial.println("connection failed");
      //Serial.println("Please try again");
      return;
    } 
    client.print("getTimeNow\n");

    // Read all the lines of the reply from server and print them to Serial
    while(!client.available()){
      //Serial.println("Waiting data response from the server"); 
      delay(1000);  
    }
    String rsp = client.readStringUntil('\n');
    //Serial.println("Current time is: " + rsp);
    Serial.println(rsp);
  }
  else if(msg == "sendData"){
    //Serial.println("Message obtained from sensor node: " + msg);
    Serial.println("sendDataOK");
    while(!Serial.available()){}; 
    String data = Serial.readStringUntil('\n');
    //Serial.println("Data received: " + data);
    //Serial.println("Sending data transmission to server");
    //Serial.print("connecting to server");

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect(server, httpPort)) {
      //Serial.println("connection failed");
      //Serial.println("Please try again");
      return;
    } 
    client.println(data);

    // Read all the lines of the reply from server and print them to Serial
    while(!client.available()){
      //Serial.println("Waiting data response from the server"); 
      delay(1000); 
    }
    String rsp = client.readStringUntil('\n');
    if(rsp == "RECEIVED"){
      //Serial.println("Server responded: " + rsp);
      Serial.println(rsp);
    }
    else{return;}   
  }
  else if(msg == "sleepNow"){
    Serial.println("sleepNowOK");
    while(!Serial.available()){}; 
    String sleepTime = Serial.readStringUntil('\n');
    int sleepTimeInt = sleepTime.toInt();
    ESP.deepSleep(sleepTimeInt * 1000000, WAKE_RF_DEFAULT);
  }
  else{
    //Serial.println("Unrecognized data");
    //Serial.println("please try again");
  }
  
  
  
}
