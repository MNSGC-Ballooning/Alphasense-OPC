# Alphasense-OPC
Code to run the ALphasense OPC-N2 optical particle counter using an Arduino. Versions 1 and 2 take advantage of the sensor's STANDALONE mode, and uses active heating (heating pads that turn on/off based on temperature inputs).

OPC_v1: builds off Heater_V2.ino and includes an additional temperature sensor and relay switch to power "ON" OPC. This code assumes a total of 3 temperature sensors (1 analog, 2 digital) and 1 active heater -- BE SURE TO NOTE which temperature sensor is which and corresponds to which header in the SD card log!!!! 

OPC_v2: modification of OPC_v1 that now uses TWO active heaters. There is one temperature sensor per heater unit (the first monitors battery temp., the second monitors OPC temp.), and a third temperature sensor measures the atmospheric temperature outside the payload.

USE LATEST VERSION OF CODE!!

Email habec021@umn.edu for any questions.


