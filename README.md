## ESP32 as data reader from OR-WE-51
OR-WE-517 is 3-phase multi-tariff energy meter with RS-485/Modbus

Simple scheme

-------|
ESP32  16 RXD --|
       |        | TTL 485 MAX485 XY-017
       17 TXD --|
       |
-------|
