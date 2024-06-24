#include "credentials.h"
#include <WiFi.h>               // check this later https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
#include <ESPAsyncWebServer.h> // https://randomnerdtutorials.com/esp32-async-web-server-espasyncwebserver-library/
                              // https://github.com/me-no-dev/ESPAsyncWebServer

#include <ArduinoJson.h>  //https://microcontrollerslab.com/esp32-rest-api-web-server-get-post-postman/
                          // https://arduinojson.org/v6/api/jsonobject/createnestedobject/
#include <OneWire.h> // https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/
#include <DallasTemperature.h>  // https://randomnerdtutorials.com/esp32-multiple-ds18b20-temperature-sensors/
                                //https://randomnerdtutorials.com/guide-for-ds18b20-temperature-sensor-with-arduino/

// Data wire is connected to GPIO15
#define ONE_WIRE_BUS 15
// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

DeviceAddress sensor1 = { 0x28, 0xFF, 0x7A, 0xB4, 0x1, 0x17, 0x3, 0xCC };
DeviceAddress sensor2 = { 0x28, 0xFF, 0xE5, 0x2D, 0x0, 0x17, 0x3, 0xCD };
DeviceAddress sensor3 = { 0x28, 0xFF, 0xC9, 0x1D, 0x3, 0x17, 0x4, 0x3B };
DeviceAddress sensor4 = { 0x28, 0xFF, 0xF8, 0xBD, 0xD0, 0x16, 0x5, 0xBC };
DeviceAddress sensor5 = { 0x28, 0xFF, 0xC9, 0x8,  0x2, 0x17, 0x3, 0xAA };

#define RXD2 16
#define TXD2 17

float voltageL1,voltageL2,voltageL3,frequency,currentL1,currentL2,currentL3,activePowerTotal,activePowerL1,activePowerL2,activePowerL3,tempSensor1,tempSensor2,tempSensor3,tempSensor4,tempSensor5,totalActiveEnergy,totalActiveEnergyL1,totalActiveEnergyL2,totalActiveEnergyL3;

char buffer[3048];

// Replace with your network credentials
const char* ssid = WIFI_SSID;
const char* password  = WIFI_PASSWD;

//to use IPAddress uncomment WiFi.config in Setup()
IPAddress ip(192, 168, 10, 202);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(1, 1, 1, 1);
IPAddress secondaryDNS(8, 8, 8, 8);
IPAddress gateway(192, 168, 10, 1);
String hostname = "ESP32Power"; // https://randomnerdtutorials.com/esp32-set-custom-hostname-arduino/
                                 //used alsso for API names     
AsyncWebServer server(80);

//web page with all available data reachable on http://192.168.10.200/ ...auto refresh
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: left;}
    p {font-size: 1.0rem;}
    body {max-width: 600px; vertical-align:left; padding-bottom: 25px;}
   
  </style>
  <script src="https://code.jquery.com/jquery-3.5.1.js"></script>
</head>
<body>
  <h2>ESP32 | OR-WE-517</h2>
 <div id="reloaddiv">         
   %MYDATA%
   </div>

<script>
$(document).ready(function(){
setInterval(function(){
      $("#reloaddiv").load(window.location.href + " #reloaddiv" );
}, 2000);
});

</script>
</body>
</html>
)rawliteral";

String dataToHtml(const String& var){
  //Serial.println(var);
  if(var == "MYDATA"){
    String lines = "";
      // lines += "<p>L1 Voltage: " + String(voltageL1)+  "V</p>";
      // lines += "<p>L2 Voltage: " + String(voltageL2)+  "V</p>";
      // lines += "<p>L3 Voltage: " + String(voltageL3)+  "V</p>";
      // lines += "<p>L1 Current: " + String(currentL1)+  "A</p>";
      // lines += "<p>L2 Current: " + String(currentL2)+  "A</p>";
      // lines += "<p>L3 Current: " + String(currentL3)+  "A</p>";
      lines += "<p>Active Power Total: " + String(activePowerTotal)+  "W</p>";
      lines += "<p>L1 Active Power: " + String(activePowerL1)+  "W</p>";
      lines += "<p>L2 Active Power: " + String(activePowerL2)+  "W</p>";
      lines += "<p>L3 Active Power: " + String(activePowerL3)+  "W</p>";
      lines += "<p>Total Active Energy: " + String(totalActiveEnergy)+  "kWh</p>";
      lines += "<p>L1 Total Active Energy: " + String(totalActiveEnergyL1)+  "kWh</p>";
      lines += "<p>L2 Total Active Energy: " + String(totalActiveEnergyL2)+  "kWh</p>";
      lines += "<p>L3 Total Active Energy: " + String(totalActiveEnergyL3)+  "kWh</p>";
      lines += "<br>";
      lines += "<h2>ESP32 | Temperature sensors</h2>";
      lines += "<p>Sensor1:  " + String(tempSensor1)+ "°C</p>";
      lines += "<p>Sensor2:  " + String(tempSensor2)+ "°C</p>";
      lines += "<p>Sensor3:  " + String(tempSensor3)+ "°C</p>";
      lines += "<p>Sensor4:  " + String(tempSensor4)+ "°C</p>";
      lines += "<p>Sensor5:  " + String(tempSensor5)+ "°C</p>";

    return lines;
  }
  return String();
}
 
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8E1, RXD2, TXD2);  //Important!!! ...don't use 8N1 with deveice OR-WE-517 ...check device configuration/datasheet ...https://www.arduino.cc/reference/en/language/functions/communication/serial/begin/
  delay(1000);

  
  Serial.print("\n");
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA); /* Configure ESP32 in STA Mode */
  //WiFi.config(ip, gateway, subnet,primaryDNS, secondaryDNS);  
  WiFi.setHostname(hostname.c_str()); 
  WiFi.begin(ssid, password); /* Connect to Wi-Fi based on the above SSID and Password */
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("*");
    delay(100);
  }
  Serial.print("\n");
  Serial.print("Connected to Wi-Fi: ");
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  delay(200);

  delay(2000);
  
 // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html,dataToHtml);
  });

 // Route for metrics / Prometheus
  server.on("/metrics", HTTP_GET, [](AsyncWebServerRequest *request){
  String response = 
            "# HELP ham_temp_sensor1 Temp counter metric show Degre Celsius\n# TYPE ham_temp_sensor1 gauge\nham_temp_sensor1 " + String(tempSensor1) + "\n"
            "# HELP ham_temp_sensor2 Temp counter metric show Degre Celsius\n# TYPE ham_temp_sensor2 gauge\nham_temp_sensor2 " + String(tempSensor2) + "\n"
            "# HELP ham_temp_sensor3 Temp counter metric show Degre Celsius\n# TYPE ham_temp_sensor3 gauge\nham_temp_sensor3 " + String(tempSensor3) + "\n"
            "# HELP ham_temp_sensor4 Temp counter metric show Degre Celsius\n# TYPE ham_temp_sensor4 gauge\nham_temp_sensor4 " + String(tempSensor4) + "\n"
            "# HELP ham_temp_sensor5 Temp counter metric show Degre Celsius\n# TYPE ham_temp_sensor5 gauge\nham_temp_sensor5 " + String(tempSensor5) + "\n";
        
        request->send(200, "text/plain", response);
    });


// just for testing
 server.on("/hello", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello World!");
  });

// just for monitoring
  server.on("/status", HTTP_GET, [] (AsyncWebServerRequest *request) {
       request->send(200, "text/plain", "OK");
  });

// get temperature
  server.on("/temperature", HTTP_GET, [] (AsyncWebServerRequest *request) {
      Serial.println("Get temperatures");

      //generate Json objects and array
      StaticJsonDocument<1024> doc;

      doc["id"] = hostname;
      doc["api"] = "temperature";

      JsonArray values = doc.createNestedArray("values");

      JsonObject values_0 = values.createNestedObject();
      values_0["sensor"] = "temp1";
      values_0["value"] = tempSensor1;
      values_0["unit"] = "°C";

      JsonObject values_1 = values.createNestedObject();
      values_1["sensor"] = "temp2";
      values_1["value"] = tempSensor2;
      values_1["unit"] = "°C";

      JsonObject values_2 = values.createNestedObject();
      values_2["sensor"] = "temp3";
      values_2["value"] = tempSensor3;
      values_2["unit"] = "°C";

      JsonObject values_3 = values.createNestedObject();
      values_3["sensor"] = "temp4";
      values_3["value"] = tempSensor4;
      values_3["unit"] = "°C";

      JsonObject values_4 = values.createNestedObject();
      values_4["sensor"] = "temp5";
      values_4["value"] = tempSensor5;
      values_4["unit"] = "°C";

      serializeJson(doc, buffer);

      request->send(200, "application/json", buffer);
  }); 

// get power data
  server.on("/power", HTTP_GET, [] (AsyncWebServerRequest *request) {
      Serial.println("Get power data");

      //generate Json objects and array
      StaticJsonDocument<1024> doc;

      doc["id"] = hostname;
      doc["api"] = "power";

      JsonArray values = doc.createNestedArray("values");

      JsonObject values_0 = values.createNestedObject();
      values_0["sensor"] = "activePowerTotal";
      values_0["value"] = activePowerTotal;
      values_0["unit"] = "W";

      JsonObject values_1 = values.createNestedObject();
      values_1["sensor"] = "activePowerL1";
      values_1["value"] = activePowerL1;
      values_1["unit"] = "W";

      JsonObject values_2 = values.createNestedObject();
      values_2["sensor"] = "activePowerL2";
      values_2["value"] = activePowerL2;
      values_2["unit"] = "W";

      JsonObject values_3 = values.createNestedObject();
      values_3["sensor"] = "activePowerL3";
      values_3["value"] = activePowerL3;
      values_3["unit"] = "W";

      serializeJson(doc, buffer);

      request->send(200, "application/json", buffer);
  }); 

  // get power data
  server.on("/energy", HTTP_GET, [] (AsyncWebServerRequest *request) {
      Serial.println("Get power data");

      //generate Json objects and array
      StaticJsonDocument<1024> doc;

      doc["id"] = hostname;
      doc["api"] = "energy";

      JsonArray values = doc.createNestedArray("values");

      JsonObject values_0 = values.createNestedObject();
      values_0["sensor"] = "totalActiveEnergy";
      values_0["value"] = totalActiveEnergy;
      values_0["unit"] = "kWh";

      JsonObject values_1 = values.createNestedObject();
      values_1["sensor"] = "totalActiveEnergyL1";
      values_1["value"] = totalActiveEnergyL1;
      values_1["unit"] = "kWh";

      JsonObject values_2 = values.createNestedObject();
      values_2["sensor"] = "totalActiveEnergyL2";
      values_2["value"] = totalActiveEnergyL2;
      values_2["unit"] = "kWh";

      JsonObject values_3 = values.createNestedObject();
      values_3["sensor"] = "totalActiveEnergyL3";
      values_3["value"] = totalActiveEnergyL3;
      values_3["unit"] = "kWh";

      serializeJson(doc, buffer);

      request->send(200, "application/json", buffer);
  }); 

  // get all data
  server.on("/all", HTTP_GET, [] (AsyncWebServerRequest *request) {
      Serial.println("Get all data");

      //generate Json objects and array
      StaticJsonDocument<3048> doc;

      doc["id"] = hostname;
      doc["api"] = "all";

      JsonArray values = doc.createNestedArray("values");

      JsonObject values_0 = values.createNestedObject();
      values_0["sensor"] = "temp1";
      values_0["value"] = tempSensor1;
      values_0["unit"] = "°C";

      JsonObject values_1 = values.createNestedObject();
      values_1["sensor"] = "temp2";
      values_1["value"] = tempSensor2;
      values_1["unit"] = "°C";

      JsonObject values_2 = values.createNestedObject();
      values_2["sensor"] = "temp3";
      values_2["value"] = tempSensor3;
      values_2["unit"] = "°C";

      JsonObject values_3 = values.createNestedObject();
      values_3["sensor"] = "temp4";
      values_3["value"] = tempSensor4;
      values_3["unit"] = "°C";

      JsonObject values_4 = values.createNestedObject();
      values_4["sensor"] = "temp5";
      values_4["value"] = tempSensor5;
      values_4["unit"] = "°C";

      JsonObject values_5 = values.createNestedObject();
      values_5["sensor"] = "voltageL1";
      values_5["value"] = voltageL1;
      values_5["unit"] = "V";

      JsonObject values_6 = values.createNestedObject();
      values_6["sensor"] = "voltageL2";
      values_6["value"] = voltageL2;
      values_6["unit"] = "V";

      JsonObject values_7 = values.createNestedObject();
      values_7["sensor"] = "voltageL3";
      values_7["value"] = voltageL3;
      values_7["unit"] = "V";

      JsonObject values_8 = values.createNestedObject();
      values_8["sensor"] = "frequency";
      values_8["value"] = frequency;
      values_8["unit"] = "Hz";

      JsonObject values_9 = values.createNestedObject();
      values_9["sensor"] = "currentL1";
      values_9["value"] = currentL1;
      values_9["unit"] = "A";

      JsonObject values_10 = values.createNestedObject();
      values_10["sensor"] = "currentL2";
      values_10["value"] = currentL2;
      values_10["unit"] = "A";

      JsonObject values_11 = values.createNestedObject();
      values_11["sensor"] = "currentL3";
      values_11["value"] = currentL3;
      values_11["unit"] = "A";

      JsonObject values_12 = values.createNestedObject();
      values_12["sensor"] = "activePowerTotal";
      values_12["value"] = activePowerTotal;
      values_12["unit"] = "W";

      JsonObject values_13 = values.createNestedObject();
      values_13["sensor"] = "activePowerL1";
      values_13["value"] = activePowerL1;
      values_13["unit"] = "W";

      JsonObject values_14 = values.createNestedObject();
      values_14["sensor"] = "activePowerL2";
      values_14["value"] = activePowerL2;
      values_14["unit"] = "W";

      JsonObject values_15 = values.createNestedObject();
      values_15["sensor"] = "activePowerL3";
      values_15["value"] = activePowerL3;
      values_15["unit"] = "W";

      JsonObject values_16 = values.createNestedObject();
      values_16["sensor"] = "totalActiveEnergy";
      values_16["value"] = totalActiveEnergy;
      values_16["unit"] = "kWh";

      JsonObject values_17 = values.createNestedObject();
      values_17["sensor"] = "totalActiveEnergyL1";
      values_17["value"] = totalActiveEnergyL1;
      values_17["unit"] = "kWh";

      JsonObject values_18 = values.createNestedObject();
      values_18["sensor"] = "totalActiveEnergyL2";
      values_18["value"] = totalActiveEnergyL2;
      values_18["unit"] = "kWh";

      JsonObject values_19 = values.createNestedObject();
      values_19["sensor"] = "totalActiveEnergyL3";
      values_19["value"] = totalActiveEnergyL3;
      values_19["unit"] = "kWh";

      serializeJson(doc, buffer);

      request->send(200, "application/json", buffer);
  }); 

 
  // Start server
  server.begin();
 
  sensors.begin();
}

float From32HexToDec (byte val1, byte val2, byte val3, byte val4){
  float x;
  ((byte*)&x)[3]= val1;
  ((byte*)&x)[2]= val2;
  ((byte*)&x)[1]= val3;
  ((byte*)&x)[0]= val4;
  //Serial.println(x);
  return x;
}  

void getTemperatures() {
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  tempSensor1 = sensors.getTempC(sensor1); 
  Serial.print("Sensor 1(*C): ");
  Serial.println(tempSensor1); 
 
  tempSensor2 = sensors.getTempC(sensor2);
  Serial.print("Sensor 2(*C): ");
  Serial.println(tempSensor2); 
  
  tempSensor3 = sensors.getTempC(sensor3);
  Serial.print("Sensor 3(*C): ");
  Serial.println(tempSensor3); 
  
  tempSensor4 = sensors.getTempC(sensor4);
  Serial.print("Sensor 4(*C): ");
  Serial.println(tempSensor4); 
    
  tempSensor5 = sensors.getTempC(sensor5);
  Serial.print("Sensor 5(*C): ");
  Serial.println(tempSensor5); 
  Serial.println();
 
}

void sendMessageModbus(byte sendmsg[]) {
  // ************************  SEND DATA TO DEVICE ************************* 
  // https://unserver.xyz/modbus-guide/#modbus-rtu-data-frame-section
  // CRC calculator https://www.simplymodbus.ca/crc.xls
  // OR-WE-517 modbus registers list https://b2b.orno.pl/download-resource/26065/
  // byte sendmsg[] = {0x01, 0x03, 0x00, 0x1C, 0x00, 0x01, 0x45, 0xCC};
  //byte sendmsg[] = {0x01, 0x03, 0x00, 0x1C, 0x00, 0x02, 0x05, 0xCD}; 
  int i;
  int sendmsglen=8; // send message for OR-WE-517 is always 8bites long 

  Serial.print("Message sent to device: ");
  for(i = 0 ; i < sendmsglen ; i++){
    Serial2.write(sendmsg[i]); 
    Serial.print(sendmsg[i], HEX); 
  }

  // This part is commented as I used it during development and it is not needed later.
  // Results looks like this:

  // Message sent to device: 1301C025CD
  // Message byte array
  // Bite number:0|1|2|3|4|5|6|7|
  // HEX :1|3|0|1C|0|2|5|CD|
  // DEC :1|3|0|28|0|2|5|205|

  // Uncoment when needed
  // Serial.println();
  // Serial.println("Message byte array");
  // Serial.print("Bite number:");
  //  for(i = 0 ; i < sendmsglen ; i++){
  //       Serial.print(i);  
  //             Serial.print("|");      
  //  }
  //  Serial.println();
  //   Serial.print("HEX :");
  //  for(i = 0 ; i < sendmsglen ; i++){
  //       Serial.print(sendmsg[i], HEX);  
  //             Serial.print("|");      
  //  }
  //   Serial.println();
  //    Serial.print("DEC :");
  //  for(i = 0 ; i < sendmsglen ; i++){
  //       Serial.print(sendmsg[i]);  
  //             Serial.print("|");      
  //  }
  // Serial.println();
  Serial.println();
  delay(500); 
}

byte receivedmsg[50];

void receivedMessageModbus() {
// **********************  DATA RECEIVED FROM DEVICE **********************
int a=0;
int b=0;


while(Serial2.available()) {
 receivedmsg[a] = Serial2.read();
 a++;
 }
Serial.print("Data received from device: ");
for(b = 0 ; b < a ; b++){      
 Serial.print(receivedmsg[b], HEX); 
 }
Serial.println();

// This part is commented as I used it during development and it is not needed later.
// Results looks like this:

// Data received: 1343EE66666BDA6
// Message byte array
// Bite number:0|1|2|3|4|5|6|7|8|
// HEX :1|3|4|3E|E6|66|66|BD|A6|
// DEC :1|3|4|62|230|102|102|189|166|

// Uncoment when needed
// Serial.println("Message byte array");
// Serial.print("Bite number:");
//  for(b = 0 ; b < a ; b++){
//   Serial.print(b);  
//   Serial.print("|");      
//  }
// Serial.println();
// Serial.print("HEX :");
//  for(b = 0 ; b < a ; b++){
//   Serial.print(receivedmsg[b], HEX);  
//   Serial.print("|");      
//  }
// Serial.println();
// Serial.print("DEC :");
// for(b = 0 ; b < a ; b++){
//   Serial.print(receivedmsg[b]);  
//   Serial.print("|");      
//  }
// Serial.println(); 
Serial.println();
}


void firstPowerData(byte data[]){

    sendMessageModbus(data);  
    receivedMessageModbus();
    

    // ************************************************************************
    // Get DATA part from byte message

    // We get message for 6 registers but each register of device OR-WE-517 contain from 4 DATA bits.   
    // Example:                         1       2      3
    // Data received from device: 13C 437200 437200 43734CCD 301A
    // Below is whole magic. I use function From32HexToDec to do is nice way.
    // Testing https://babbage.cs.qc.cuny.edu/IEEE-754.old/32bit.html
    // float x;
    // ((byte*)&x)[3]= receivedmsg[3];
    // ((byte*)&x)[2]= receivedmsg[4];
    // ((byte*)&x)[1]= receivedmsg[5];
    // ((byte*)&x)[0]= receivedmsg[6];
    // Serial.println(x,2);
    // Serial.println(From32HexToDec(receivedmsg[3],receivedmsg[4],receivedmsg[5],receivedmsg[6]),2);

    voltageL1 =From32HexToDec(receivedmsg[3],receivedmsg[4],receivedmsg[5],receivedmsg[6]);
    Serial.print("L1 Voltage: ");
    Serial.print(voltageL1,2);
    Serial.println(" V");

    voltageL2 =From32HexToDec(receivedmsg[7],receivedmsg[8],receivedmsg[9],receivedmsg[10]);
    Serial.print("L2 Voltage: ");
    Serial.print(voltageL2,2);
    Serial.println(" V");

    voltageL3 =From32HexToDec(receivedmsg[11],receivedmsg[12],receivedmsg[13],receivedmsg[14]);
    Serial.print("L3 Voltage: ");
    Serial.print(voltageL3,2);
    Serial.println(" V");

    frequency =From32HexToDec(receivedmsg[15],receivedmsg[16],receivedmsg[17],receivedmsg[18]);
    Serial.print("Grid Frequency: ");
    Serial.print(frequency,2);
    Serial.println(" Hz");

    currentL1 =From32HexToDec(receivedmsg[19],receivedmsg[20],receivedmsg[21],receivedmsg[22]);
    Serial.print("L1 Current: ");
    Serial.print(currentL1,2);
    Serial.println(" A");

    currentL2 =From32HexToDec(receivedmsg[23],receivedmsg[24],receivedmsg[25],receivedmsg[26]);
    Serial.print("L2 Current: ");
    Serial.print(currentL2,2);
    Serial.println(" A");

    currentL3 =From32HexToDec(receivedmsg[27],receivedmsg[28],receivedmsg[29],receivedmsg[30]);
    Serial.print("L3 Current: ");
    Serial.print(currentL3,2);
    Serial.println(" A");

    // *1000 to get W because provided value is in kW
    activePowerTotal =From32HexToDec(receivedmsg[31],receivedmsg[32],receivedmsg[33],receivedmsg[34])*1000;
    Serial.print("Total Active Power: ");
    Serial.print(activePowerTotal);
    Serial.println(" W");

    // *1000 to get W because provided value is in kW
    activePowerL1 =From32HexToDec(receivedmsg[35],receivedmsg[36],receivedmsg[37],receivedmsg[38])*1000;
    Serial.print("L1 Active Power: ");
    Serial.print(activePowerL1);
    Serial.println(" W");

    // *1000 to get W because provided value is in kW
    activePowerL2 =From32HexToDec(receivedmsg[39],receivedmsg[40],receivedmsg[41],receivedmsg[42])*1000;
    Serial.print("L2 Active Power: ");
    Serial.print(activePowerL2);
    Serial.println(" W");

    // *1000 to get W because provided value is in kW
    activePowerL3 =From32HexToDec(receivedmsg[43],receivedmsg[44],receivedmsg[45],receivedmsg[46])*1000;
    Serial.print("L3 Active Power: ");
    Serial.print(activePowerL3);
    Serial.println(" W");

    Serial.println();
}

void secondPowerData(byte data[]){

    sendMessageModbus(data);  
    receivedMessageModbus();
 
    // ************************************************************************
    // Get DATA part from byte message

    // We get message for 6 registers but each register of device OR-WE-517 contain from 4 DATA bits.   
    // Example:                         1       2      3
    // Data received from device: 13C 437200 437200 43734CCD 301A
    // Below is whole magic. I use function From32HexToDec to do is nice way.
    // Testing https://babbage.cs.qc.cuny.edu/IEEE-754.old/32bit.html
    // float x;
    // ((byte*)&x)[3]= receivedmsg[3];
    // ((byte*)&x)[2]= receivedmsg[4];
    // ((byte*)&x)[1]= receivedmsg[5];
    // ((byte*)&x)[0]= receivedmsg[6];
    // Serial.println(x,2);
    // Serial.println(From32HexToDec(receivedmsg[3],receivedmsg[4],receivedmsg[5],receivedmsg[6]),2);   

    totalActiveEnergy =From32HexToDec(receivedmsg[3],receivedmsg[4],receivedmsg[5],receivedmsg[6]);
    Serial.print("Total Active Energy: ");
    Serial.print(totalActiveEnergy,2);
    Serial.println(" kWh");

    totalActiveEnergyL1 =From32HexToDec(receivedmsg[7],receivedmsg[8],receivedmsg[9],receivedmsg[10]);
    Serial.print("L1 Total Active Energy: ");
    Serial.print(totalActiveEnergyL1,2);
    Serial.println(" kWh");

    totalActiveEnergyL2 =From32HexToDec(receivedmsg[11],receivedmsg[12],receivedmsg[13],receivedmsg[14]);
    Serial.print("L2 Total Active Energy: ");
    Serial.print(totalActiveEnergyL2,2);
    Serial.println(" kWh");

    totalActiveEnergyL3 =From32HexToDec(receivedmsg[15],receivedmsg[16],receivedmsg[17],receivedmsg[18]);
    Serial.print("L3 Total Active Energy: ");
    Serial.print(totalActiveEnergyL3,2);
    Serial.println(" kWh");

    Serial.println();
}

void loop() {
    Serial.print("\n");
    Serial.print("Connected to Wi-Fi: ");
    Serial.println(WiFi.SSID());
    Serial.println(WiFi.localIP());
    Serial.print("\n");

    getTemperatures();

    byte firstMsgArray[]={0x01, 0x03, 0x00, 0x0E, 0x00, 0x16, 0xA5, 0xC7};
    // sendMessageModbus(firstMsgArray);  
    // receivedMessageModbus();
    // splitReceivedData();
    firstPowerData(firstMsgArray);

    delay(2000);  

    byte secondMsgArray[]={0x01, 0x03, 0x01, 0x00, 0x00, 0x08, 0x45, 0xF0};
    // sendMessageModbus(secondMsgArray);  
    // receivedMessageModbus();
    // splitReceivedData();
    secondPowerData(secondMsgArray);

    delay(1000);        
}
