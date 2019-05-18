#include "MQ135.h"
#include <SoftwareSerial.h>         // including the library for the software serial
#define DEBUG true
SoftwareSerial esp8266(10,11);      /* This will make the pin 10 of arduino as RX pin and
pin 11 of arduino as the TX pin Which means that you have to connect the TX from the esp8266
to the pin 10 of arduino and the Rx from the esp to the pin 11 of the arduino*/
                                   
const int trigPin = 8; //8           // Making the arduino's pin 8 as the trig pin of ultrasonic sensor
const int echoPin = 9; //9           // Making the arduino's pin 9 as the echo pin of the ultrasonic sensor
const int trigPin2 = 7;
const int echoPin2 = 6;
const int ledGas = 4;
const int ledSonar = 5;
// defining two variable for measuring the distance
long duration[]={0,0};
int distance[]={0,0};

// measuring air quality
#define PIN_MQ135 A0     //attach A0 of gas sensor to arduino's A0
MQ135 mq135_sensor = MQ135(PIN_MQ135);
float air_quality;

void setup()
{
  Serial.begin(115200);         // Setting the baudrate at 115200
  esp8266.begin(115200);        // Set the baudrate according to you esp's baudrate. your esp's baudrate might be different from mine
  pinMode(trigPin, OUTPUT);   // Setting the trigPin as Output pin
  pinMode(echoPin, INPUT);    // Setting the echoPin as Input pin
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(ledGas, OUTPUT);
  pinMode(ledSonar, OUTPUT);
  digitalWrite(ledGas, LOW);
  digitalWrite(ledSonar, LOW);
  
  sendData("AT+RST\r\n",2000,DEBUG);            // command to reset the module
  sendData("AT+CWMODE=2\r\n",1000,DEBUG);       // This will configure the mode as access point
  sendData("AT+CIFSR\r\n",1000,DEBUG);          // This command will get the ip address
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG);       // This will configure the esp for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // This command will turn on the server on port 80
}

void loop() {

digitalWrite(trigPin, LOW);   // Making the trigpin as low
delayMicroseconds(2);         // delay of 2us
digitalWrite(trigPin, HIGH); // making the trigpin high for 10us to send the signal 
delayMicroseconds(10);
digitalWrite(trigPin, LOW);   
duration[0] = pulseIn(echoPin, HIGH);  // reading the echopin which will tell us that how much time the signal takes to come back 
delayMicroseconds(5);

digitalWrite(trigPin2, LOW);   // Making the trigpin as low
delayMicroseconds(2);         // delay of 2us
digitalWrite(trigPin2, HIGH); // making the trigpin high for 10us to send the signal 
delayMicroseconds(10);
digitalWrite(trigPin2, LOW);
duration[1] = pulseIn(echoPin2, HIGH);

distance[0]= duration[0]*0.034/2;         // Calculating the distance and storing in the distance variable
distance[1]= duration[1]*0.034/2;

air_quality = mq135_sensor.getPPM();

if (distance[0] < 11 || distance[1] < 11) {         //Trash can threshold value**
  digitalWrite(ledSonar, HIGH);
} else {
  digitalWrite(ledSonar, LOW);
}

if (air_quality > 1000) {             //Gas threshold value**
  digitalWrite(ledGas, HIGH);
} else {
  digitalWrite(ledGas, LOW);
}

String air;
if(air_quality < 900) air = "Moderate air";
else if(air_quality < 2000) air = "Poor air";
else air = "Hazardous air!";


Serial.print("Distance 1: ");
Serial.print(distance[0]);
Serial.print("\t Distance 2: ");
Serial.print(distance[1]);
Serial.print("\t Air Quality: ");
Serial.println(air_quality);
delay(200);


  if(esp8266.available())         // This command will check if the esp is sending a message 
  {    
    if(esp8266.find("+IPD,"))
    {
     delay(1000);
     int connectionId = esp8266.read()-48; /* We are subtracting 48 from the output because the read() function returns 
                                            the ASCII decimal value and the first decimal number which is 0 starts at 48*/
     String webpage = "<h1>IOT Garbage Monitoring System</h1>";
       webpage += "<p><h2><br>";
       for(int i=0; i<2; ++i){
       webpage += "<br><font color=red > Trash can # ";
       webpage += i+1;
       webpage += ": </font>";
       if (distance[i]>11 && distance[i]<25)
       {
        webpage+= " Trash can is Half Filled ";
        }  
       else if (distance[i]<11)
       {        webpage+= "<font color=red > Trash can is Full </font>";
        }
        else{
          webpage+= " Trash can is Empty ";
          }
          
        webpage+= "<br>Distance of garbage from sensor: ";
        webpage+= "<font color=blue >";
        webpage+= distance[i];
        webpage+= " cm </font><br>";
       }
        webpage+= "<br>Air Quality Measurement: <font color=blue >";
        webpage+= air_quality;
        webpage+= " ppm </font><br>";
        webpage+= air;
       webpage += "</h2></p></body>"; 
       
     String cipSend = "AT+CIPSEND=";
     cipSend += connectionId;
     cipSend += ",";
     cipSend +=webpage.length();
     cipSend +="\r\n";

     sendData(cipSend,1000,DEBUG);
     sendData(webpage,1000,DEBUG);    
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; 
     closeCommand+="\r\n";
     sendData(closeCommand,3000,DEBUG);
    }
  }
}
 
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";   
    esp8266.print(command); 
    long int time = millis();
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        char c = esp8266.read(); 
        response+=c;
      }  
    }
    if(debug)
    {
      Serial.print(response);
    }
    return response;
}

