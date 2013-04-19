Air Quality Egg + Arduino + Arduino Ethernet Shield
========

This is some code developed for my Air Quality Egg connected to an Arduino UNO and the Ethernet Shield.

There are 2 files:

* AQEArduinoYaler.ino

This AQE with Arduino is connected to the Internet through Yaler [http://yaler.net].

I also created a JSON and CSV compatible with Cosm [http://cosm.com] for being pulled by Cosm (never worked properly with Cosm).

* AQEArduinoPush.ino

This AQE sends PUT messages to Cosm every 10 seconds to store the data of all the sensors.

