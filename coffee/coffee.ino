#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <Hash.h>
#include <FS.h>
#include <ESPAsyncWebServer.h>

/* Set these to your desired credentials. */


AsyncWebServer server(80);

const int trigPin = 14;  //D5
const int echoPin = 12;  //D6

const int coil = 15; //D8
const int valve1 = 4; //D7
const int valve2 = 5; //D1
const int settings = 4; //D2
int buttonState = 0;
long duration;
int distance;

const char* PARAM_STRING = "inputInt";
const char* PARAM_INT = "inputInt1";
const char* PARAM_FLOAT = "inputInt2";
const char* PARAM_MIN = "inputInt3";
const char* PARAM_MAX = "inputInt4";
const char* FILLING_INTERVAL = "inputInt5";

// HTML web page to handle 3 input fields (inputString, inputInt, inputFloat)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(true); }, 500);  
    }
  </script>
  <style>
  input{
    min-width: 15rem;
    min-height: 2rem;
    border: 1px solid #999;
    border-radius: 3px;
    padding-left: 10px;
    padding-right: 3px;
  }
  input:focus{
    border-color: #0000ff;
  }

  #button{
    min-width: 10rem;
    min-height: 2rem;
    border-radius: 5px;
    border: 1px solid #a569bd;
    background-color: #a569bd;
    color: #fff;
    font-size: 1rem;
  }
</style>
</head><body>
<center>
<h1 >Rajnandini Automation</h1>

  <br>
  <br>

  <form action="/get" target="hidden-form">
    MIN DISTANCE (current value %inputInt3%)<br> <input type="number" name="inputInt3" required>
<br><br>
    MAX DISTANCE (current value %inputInt4%)<br> <input type="number" name="inputInt4" required>
<br><br>    
    COIL (current value %inputInt%)<br> <input type="number" name="inputInt" required>
<br><br>
    VALVE1 (current value %inputInt1%)<br> <input type="number" name="inputInt1" required>
<br><br>
    VALVE2 (current value %inputInt2%)<br> <input type="number" name="inputInt2" required>
    <br><br>
    CUP FILLING INTERVAL (current %inputInt5%)<br> <input type="number" name="inputInt5" required>
<br><br><br><br>
    <input id="button" type="submit" value="Submit" onclick="submitMessage()">
  </form>
<br><br><br>
  <div style="font-size:0.9rem">Developed by ProAutomater<br>contact@proautomater.com</div>
  </center>
  <iframe style="display:none" name="hidden-form"></iframe>
 
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS &fs, const char * path){
  //Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  //Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  //Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Replaces placeholder with stored values
String processor(const String& var){
  if(var == "inputInt"){
    return readFile(SPIFFS, "/inputInt.txt");
  }
  else if(var == "inputInt1"){
    return readFile(SPIFFS, "/inputInt1.txt");
  }
  else if(var == "inputInt2"){
    return readFile(SPIFFS, "/inputInt2.txt");
  }
  else if(var == "inputInt3"){
    return readFile(SPIFFS, "/inputInt3.txt");
  }
  else if(var == "inputInt4"){
    return readFile(SPIFFS, "/inputInt4.txt");
  }
  else if(var == "inputInt5"){
    return readFile(SPIFFS, "/inputInt5.txt");
  }
  return String();
}

void setup() {
  Serial.begin(115200);
  // Initialize SPIFFS
  WiFi.softAP("Coffee-Machine", "pro12345");
 
  //IPAddress myIP = WiFi.softAPIP();
 
  Serial.print("AP IP address: ");
  //Serial.println(myIP);
  Serial.println(WiFi.softAPIP());

 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
 
  pinMode(coil, OUTPUT);
  pinMode(valve1, OUTPUT);
  pinMode(valve2, OUTPUT);
  pinMode(settings, INPUT);
 
 
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //WiFi.softAP(WIFI_STA);
 
 
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputint=<inputMessage>&inputString=<inputMessage>&inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
Serial.println("++++++++++++++++into writing?++");
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_STRING)) {
      inputMessage = request->getParam(PARAM_STRING)->value();
      writeFile(SPIFFS, "/inputInt.txt", inputMessage.c_str());

 Serial.println("--------------written--"+inputMessage);
    }
    else {
      inputMessage = "No int sent";
    }
    // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
    if (request->hasParam(PARAM_INT)) {
      inputMessage = request->getParam(PARAM_INT)->value();
      writeFile(SPIFFS, "/inputInt1.txt", inputMessage.c_str());

 Serial.println("--------------written--"+inputMessage);
    }
    else {
      inputMessage = "No int1 sent";
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    if (request->hasParam(PARAM_FLOAT)) {
      inputMessage = request->getParam(PARAM_FLOAT)->value();
      writeFile(SPIFFS, "/inputInt2.txt", inputMessage.c_str());

 Serial.println("--------------written--"+inputMessage);
    }
    else {
      inputMessage = "No int2 sent";
    }
   
    if (request->hasParam(PARAM_MIN)) {
      inputMessage = request->getParam(PARAM_MIN)->value();
      writeFile(SPIFFS, "/inputInt3.txt", inputMessage.c_str());

 Serial.println("--------------written--"+inputMessage);
    }
    else {
      inputMessage = "No int3 sent";
    }
    if (request->hasParam(PARAM_MAX)) {
      inputMessage = request->getParam(PARAM_MAX)->value();
      writeFile(SPIFFS, "/inputInt4.txt", inputMessage.c_str());

 Serial.println("--------------written--"+inputMessage);
    }
    else {
      inputMessage = "No int4 sent";
    }
    if (request->hasParam(FILLING_INTERVAL)) {
      inputMessage = request->getParam(FILLING_INTERVAL)->value();
      writeFile(SPIFFS, "/inputInt5.txt", inputMessage.c_str());

 Serial.println("--------------written--"+inputMessage);
    }
    else {
      inputMessage = "No int5 sent";
    }
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {

    // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
 
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
 
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  delay(2000);

 
  Serial.println(WiFi.softAPIP());
  // To access your stored values on inputString, inputInt, inputFloat
  int coilDelay = readFile(SPIFFS, "/inputInt.txt").toInt();
  //Serial.print("*** Your inputString: ");
  Serial.println(coilDelay);
 
  int valve1Delay = readFile(SPIFFS, "/inputInt1.txt").toInt();
  //Serial.print("*** Your inputInt: ");
  Serial.println(valve1Delay);
 
  int valve2Delay = readFile(SPIFFS, "/inputInt2.txt").toInt();
  //Serial.print("*** Your inputFloat: ");
  Serial.println(valve2Delay);
  delay(10);

  int MINDIST = readFile(SPIFFS, "/inputInt3.txt").toInt();
  //Serial.print("*** Your inputFloat: ");
  Serial.println(MINDIST);
  delay(10);

  int MAXDIST = readFile(SPIFFS, "/inputInt4.txt").toInt();
  //Serial.print("*** Your inputFloat: ");
  Serial.println(MAXDIST);
  delay(10);

  int FILLING_INTERVAL = readFile(SPIFFS, "/inputInt5.txt").toInt();
  //Serial.print("*** Your inputFloat: ");
  Serial.println(FILLING_INTERVAL);
  delay(10);

  buttonState = digitalRead(settings);
  if (buttonState == HIGH){
    server.end();
    int coffeemode = 1;
    WiFi.softAPdisconnect(true);
    Serial.println(coffeemode);

  if (coffeemode == 1){
    Serial.println(MINDIST);
    Serial.println(MAXDIST);
    Serial.println("coffeemode=1");
    Serial.println(MINDIST<distance);
    if (MINDIST<distance){
      if (distance<MAXDIST){
   
      Serial.println("cup placed for coffee");
     
      // coil on
      digitalWrite(coil, HIGH);
      Serial.println("coil on");
      Serial.println(coilDelay);
      delay(coilDelay);
      digitalWrite(coil, LOW);
      Serial.println("coil off");
      delay(100);
 
      // valve1 on
      digitalWrite(valve1, HIGH);
      Serial.println("valve1 on");
      delay(valve1Delay);
      digitalWrite(valve1, LOW);
      Serial.println("valve1 off");
      delay(100);
 
      // valve2 on
      digitalWrite(valve2, HIGH);
      Serial.println("valve2 on");
      delay(valve2Delay);
      digitalWrite(valve2, LOW);
      Serial.println("valve2 off");
      delay(FILLING_INTERVAL);
  }
    }
}
}
}
