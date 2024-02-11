//Include required libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
#include <FlowMeter.h>  
FlowMeter *Meter;
int Status_LED=2;
const unsigned long period = 1000;

// WiFi credentials
const char* ssid = "KLAB-FABLAB";         // change SSID
const char* password = "Innovate";    // change password
// Google script ID and required credentials
String GOOGLE_SCRIPT_ID = "AKfycbzhTLK7Ysyc_sWoO_Ma_hMaGVjOnLHIqyW2WT7Q_VjzD3MZK7A";    // change Gscript ID
String DeviceUID="Dev001A";
String Volume="";
String Prev_Volume="";
unsigned long sending_to_server_Interval_time = 6000;    // 6 sec interval to send message
const unsigned long sending_to_server_Interval_time_Confirm = 6000;    // 6 sec interval to send message

void MeterISR() {
    // let our flow meter count the pulses
    Meter->count();
}

void setup() {
  pinMode(Status_LED,OUTPUT);
  Serial.begin(115200);
  delay(1000);
  // connect to WiFi
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
     blinking();
     Serial.println("Connected to the WiFi network");
  }
  
  Meter = new FlowMeter(digitalPinToInterrupt(34), UncalibratedSensor, MeterISR, RISING);

}
void loop() {
  // wait between output updates
    delay(period);
    Meter->tick(period);
    
    Volume=String(Meter->getTotalVolume());
    if( Prev_Volume!=Volume){
       sending_to_server_Interval_time=millis();
       Prev_Volume=Volume;
    }else{}
    
    if((millis() - sending_to_server_Interval_time >= sending_to_server_Interval_time_Confirm)  && Volume>="0.01") {
      
      Serial.println("sending");
   
    if (WiFi.status() == WL_CONNECTED) {
    static bool flag = false;
    blinking();
    
    
    String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+"DeviceUID=" + DeviceUID + "&Volume=" + String(Volume);
    

    Serial.print("POST data to spreadsheet:");
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload); 
        if(payload=="OK"){
        Meter->setTotalVolume(0.0f);
        }
           
    }
    //---------------------------------------------------------------------
    http.end();
  }   
   Serial.println("Currently " + String(Meter->getCurrentFlowrate()) + " l/min, " + String(Volume)+ " l total. Prev_Volume="+String(Prev_Volume));
    
  }
  
} 

void blinking()
{
  digitalWrite(Status_LED,HIGH);
  delay(200);
  digitalWrite(Status_LED,LOW);
  delay(200);
   digitalWrite(Status_LED,HIGH);
  delay(200);
  digitalWrite(Status_LED,LOW);
  delay(200);
  
}
