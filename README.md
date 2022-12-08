## ESP32 as data reader from OR-WE-517
OR-WE-517 is 3-phase multi-tariff energy meter with RS-485/Modbus

## Used devices
- OR-WE-517 energy meter
- MAX485 IC with with 2 pins Rx and Tx, known as HW-0519 / MAX485 / XY-017 / XY-K485
- ESP32 DEVKIT V1

## Simple scheme

![Devices connection](/pictures/scheme.JPG)

**Additional scheme with DS18B20 temperature sensors "DALLAS"**

![Devices connection+Dallas](/pictures/scheme_dallas.JPG)

## Important notes
1. All used sources and comments are directly in code as comment. 
2. Serial2 config - don't use 8N1 with deveice OR-WE-517 !!! It take me long to find out correct setting SERIAL_8E1  .
3. Configure WIFI as static and uncommnet //WiFi.config(ip, gateway, subnet,primaryDNS, secondaryDNS);  or use current DHCP .
4. I don't read all values from OR-WE-517 because I don't need them. This is basic code to get what I need and save values to DB on server. Then I read and visualize data by Grafana.

## Web UI
Web server running on ESP32 provide you actual data. 
Values are pushed and page don't require reload.

**ESP32-OR-WE-517**

![Web page reachable on ESP32](/pictures/webUI.png)

**ESP32-OR-WE-517 and DS18B20 temperature sensors DALLAS**

![Web page reachable on ESP32](/pictures/webUIDalas.PNG)

## Next plans
- Add Dalas temperature sensors ds18b20 
- [X] Done ...separate ino file [ESP32-OR-WE-517_and_DS18B20_temperature_sensors_DALLAS](/ESP32-OR-WE-517_and_DS18B20_temperature_sensors_DALLAS)
- Add REST API or Websocket and read data by server app instead sending data to server
