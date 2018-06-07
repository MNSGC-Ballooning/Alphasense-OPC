# Alphasense-OPC
Code to run the ALphasense OPC-N2 optical particle counter using an Arduino. Versions 1 and 2 take advantage of the sensor's STANDALONE mode, and uses active heating (heating pads that turn on/off based on temperature inputs).


OPC_v2: builds off Heater_V2.ino; now uses TWO active heaters. Assumes there is one temperature sensor per heater unit (the first monitors battery temp., the second monitors OPC temp.), and a third temperature sensor measures the atmospheric temperature outside the payload. BE SURE TO NOTE which temperature sensor is which and corresponds to which header in the SD card log!!!!

Email habec021@umn.edu for any questions.


