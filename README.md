# WSN-Contiki
Wireless Sensor Network - basic communication protocol implementation. 

## Unicast Radio communication
I perform unicast radio communication between two motes. There are 2 motes participating in the communication. One is a light sensor which sends data to the master node which receives the data and performs an action.

## Cascade Communication
This is an implementation of cascade communication. Cascade communication is the process of passing on a message to subsequent nodes till it reaches a sink(father). This helps in propagation of messages between far away nodes by message hopping.
Here I created a network of N motes where an arbitrarily selected node is the youngest child and mote n is the father. With the click of a mote button a cascade communication is  triggered.

When a mote is clicked it becomes ’child 1’ the following events will occur:
  1. Child 1 creates the message child1 and forwards it to the mote of the next ID.
  2. The 2nd mote adds the message child2 and forwards it to child 3, etc.
  3. Once the father receives the message, he blinks its red led as many times as the number of
    children he has.
  4. For every mote, when a message is sent blink its green led, and once a message is received turn on its blue led for 3 seconds.
 
 ## IPv6 Implementation 
  IPv6 connection among the nodes and implement COAP protocol to do some operations.
  
 TODO : 
  1. I Implemented an IPv6-based network with 3 sensor nodes.
  2. One node acts as a coap-server (sky) such as upon reception of a CoAP GET request it replies with its node id and an light   sensor measurement.
  3. The 2 nd node acts as another coap-server (z1) such as upon reception of a CoaP PUT or POST request, with a desired color (r|g|b) and an integer N, it blinks its respective led N times (the integer is passed in the post request payload).
  4. One node acts as a border router between the OS and Cooja.

I use two resources to handle the respective POST and GET requests.
