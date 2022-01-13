#include "Ticker.h"
#include <ESP8266WiFi.h>
#define SSID "*"
#define PASSWORD "*"

int stepPin = 4; 
int dirPin = 5;  

int optic = 12;
int nEnable = 2 ;

bool turn = true;
long step = 0;
int stepCalibrate=0;
long distance = 100000;
int RxData = 0;
int data = 0;

WiFiServer server(80);
String header;
//Auxiliar variables to store the current output state
String StateRollUp = "off";
String StateRollDown = "off";
String StateCalibrate = "off";

void setupWIFI()
{
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void rollDown()
{
  if (StateCalibrate == "on")
  {
    digitalWrite(dirPin, 1);
    digitalWrite(stepPin, stepCalibrate);
    stepCalibrate =(stepCalibrate+1)%2;
    step=0;

  }
  else if (step < long( distance * 0.9f))
  {
    //Serial.println(step);
    //Serial.println(distance);
    digitalWrite(dirPin, 1);
    digitalWrite(stepPin, (step++) % 2);
  }
  else
  {
    
    digitalWrite(stepPin, 0);
    StateRollDown = "off";
  }
}

void rollUp()
{
  if (digitalRead(optic) == 1)
  {
    digitalWrite(dirPin, 0);
    digitalWrite(stepPin, (step--) % 2);
  }
  else
  {
    digitalWrite(stepPin, 0);
    if (StateCalibrate = "on")
    {
      distance = abs(step);
      Serial.print(distance);
      StateCalibrate = "off";
    }
    
    step=0;
    StateRollUp = "off";
  }
}
void roll()
{
  if (StateRollUp == "on")
  {
    rollUp();
  }
  else if (StateRollDown == "on")
  {
    rollDown();
  }
  else
  {
    digitalWrite(stepPin, 0);
    digitalWrite(nEnable, 1);

  }
  
  
}
Ticker timer(roll, 100, 0, MICROS_MICROS);

void setup()
{
  Serial.begin(9600);
  pinMode(stepPin, OUTPUT); // stepPin set to receive Arduino signals
  pinMode(dirPin, OUTPUT);  // DIR set to receive Arduino signals
  pinMode(2, OUTPUT);
  digitalWrite(nEnable, 0);
  digitalWrite(dirPin, 0);
  analogWriteFreq(11000);
  pinMode(D0, OUTPUT);
  digitalWrite(D0, 0);
  pinMode(optic, INPUT);
  setupWIFI();
  delay(2000);
  timer.start();
}

//------------------------------

void loop()
{

  timer.update();
  /*
  if(digitalRead(optic) == 1)
    digitalWrite(D0, HIGH) ;
  else
    digitalWrite(D0, LOW) ; 
   */
  //digitalWrite(stepPin,0);
  //digitalWrite(stepPin,1);

  //Web Server
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client)
  {                                // If a new client connects,
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0)
            {
              Serial.println("GPIO 5 on");
              StateRollUp = "on";
              StateRollDown = "off";
              digitalWrite(nEnable, 0);

              //digitalWrite(output5, HIGH);
            }
            else if (header.indexOf("GET /5/off") >= 0)
            {
              Serial.println("GPIO 5 off");
              StateRollUp = "off";
              digitalWrite(nEnable, 1);

              //digitalWrite(output5, LOW);
            }
            else if (header.indexOf("GET /4/on") >= 0)
            {
              Serial.println("GPIO 4 on");
              Serial.println(step);
              Serial.println(distance);
              StateRollDown = "on";
              StateRollUp = "off";
              digitalWrite(nEnable, 0);

              //digitalWrite(output4, HIGH);
            }
            else if (header.indexOf("GET /4/off") >= 0)
            {
              Serial.println("GPIO 4 off");
              StateRollDown = "off";
              digitalWrite(nEnable, 1);

              //digitalWrite(output4, LOW);
            }
            else if (header.indexOf("GET /1/on") >= 0)
            {
              Serial.println("Calibrate on");
              StateCalibrate="on";

              //digitalWrite(output4, HIGH);
            }
            else if (header.indexOf("GET /1/off") >= 0)
            {
              Serial.println("Calibrate off");
              StateCalibrate= "off";
              //digitalWrite(output4, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");

            // Display current state, and ON/OFF buttons for GPIO 5
            client.println("<p>GPIO 5 - State " + StateRollUp + "</p>");
            // If the output5State is off, it displays the ON button
            if (StateRollUp == "off")
            {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">Roll Up</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">Roll Up</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for GPIO 4
            client.println("<p>GPIO 4 - State " + StateRollDown + "</p>");
            // If the output4State is off, it displays the ON button
            if (StateRollDown == "off")
            {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">Roll Down</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">Roll Down</button></a></p>");
            }


            if (StateCalibrate == "off")
            {
              client.println("<p><a href=\"/1/on\"><button class=\"button\">Calibrate</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">Calibrate</button></a></p>");
            }

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
