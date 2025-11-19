#include <Arduino.h>
#include <WiFi.h>

#define LED  2
#define SENSOR 32  //3000 dark, 300 direct light
#define OUT1 18
#define OUT2 19

// Replace with your network credentials
const char* ssid     = "ESP32-Access-Point";
const char* password = "1234567890";


double Input;
boolean statusOpen = false;

int darkThreshold = 1000;
int lightThreshold = 300;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

void setup() {

  Serial.begin(115200);
  Serial.println("Starting...");


  pinMode(LED, OUTPUT);
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);

  Input = analogRead(SENSOR);

   if(Input>darkThreshold){
    digitalWrite(OUT1, HIGH);
    delay(500);
    digitalWrite(OUT1, LOW);
    statusOpen=false;
   }
   else{
    digitalWrite(OUT2, HIGH);
    delay(500);
    digitalWrite(OUT2, LOW);
    statusOpen = true;
   }



/*todo
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
*/

  for(int i=0; i<10; i++){
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
    delay(50);
  }

}

void loop() {
   Input = analogRead(SENSOR);
   Serial.println(Input);
   digitalWrite(LED, HIGH);
   delay(100);

   if(statusOpen && Input>darkThreshold){
    digitalWrite(OUT1, HIGH);
    delay(500);
    digitalWrite(OUT1, LOW);
    statusOpen=false;
   }


    else if(!statusOpen && Input<lightThreshold){
    digitalWrite(OUT2, HIGH);
    delay(500);
    digitalWrite(OUT2, LOW);
    statusOpen = true;
   }

   digitalWrite(LED, LOW);
   sleep(5);
  
  
/*
   //Webserver
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");  
  }
*/
}
