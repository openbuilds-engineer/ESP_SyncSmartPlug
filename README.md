# ESP_AsyncSmartPlug
WiFi Based Distributed IOT Home Automation
My aim is to create autonomous home automation IOT devices, that require no installation and minimum configuration. At the same time these devices need to form a connected automation system that offers great flexibility and cater for the needs of a connected home. No central management unit required - just add more devices!
The idea is based on ESP8266 nodes or any other module that has WIFI connectivity and enough processing power and capacity to send UDP broadcast and run a small web server. The aim is every node to broadcast via UDP its sensor values, serve out its own management interface and to be able to 'see' the sensor values from the other nodes. Also nodes that are able to switch things ON and OFF or 'DO' any actual work have a simple IFTT like rule engine. This type of set up allows for autonomous operation of each node with reporting and configuration functions as well as building a complex home automation system based on simple rules involving different nodes' sensor values.

The first node is a 'smart plug' with ESP8266-12 NodeMCU 1.0, ACS712 30A hall-effect current sensor, 30A relay module and 7-segment display module based on the TM1637 chip.

I am writing the code through the Arduino IDE with the help of the "Arduino core for ESP8266 Wi-Fi chip" project. Currently I have three entity lists that I keep in memory – “Nodes”, “Things” and “Recipes”. They are persisted via ESPFFS to the 3Mb flash memory and loaded at start up. Each “Node” has a set of “Things” that it can either switch ON or OFF, or it can report their current value, or both. The decision to switch a state of a “Thing” is made either by direct command via override property or by processing the “Recipes” that involve the “Thing”. The “Recipes” follow “If This Then That” format and are executed sequentially (order is important). The management and reporting interface is a single html5 page stored also on the ESPFFS with jquery and jqplot JavaScript libraries. The page is loaded once and the communication with the node is done via Web Sockets. Each “Node” broadcasts its “Things” state at a set interval via UDP and to all its Web Socket clients. The UDP broadcast is used to update the local list of “Nodes” and to process the “Recipes” that involve the “Things” in the broadcast. 