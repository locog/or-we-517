## ESP32 as data reader from OR-WE-517
OR-WE-517 is 3-phase multi-tariff energy meter with RS-485/Modbus


## Simple scheme

![Devices connection](/pictures/scheme.JPG)

## Important notes
1. All used sources and comments are directly in code as comment. 
2. Serial2 config - don't use 8N1 with deveice OR-WE-517 !!! It take me long to find out correct setting SERIAL_8E1  .
3. Configure WIFI as static and uncommnet //WiFi.config(ip, gateway, subnet,primaryDNS, secondaryDNS);  or use current DHCP .
4. I don't read all values from OR-WE-517 because I don't need them. This is basic code to get what I need and save values to DB on server. Then I read and visualize data by Grafana.

## Web UI
Web server running on ESP32 provide you actual data. 
Values are pushed and page don't require reload.

![Web page reachable on ESP32](/pictures/webUI.png)

## Next plans
- [X] Add Dalas temperature sensors ds18b20 ..new feature branch
- [ ] Add REST API or Websocket and read data by server app instead sending data to server
