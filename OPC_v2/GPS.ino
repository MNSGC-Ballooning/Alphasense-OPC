float checkAlt;
long lastGPS = -1000000;  //for testing purposes

//function to handle both retrieval of data from GPS module and sensors, as well as recording it on the SD card
void updateGPS() {
  adxl.readAccel(&x,&y,&z);
  while (Serial1.available() > 0) {
    GPS.encode(Serial1.read());
  }
  if (GPS.altitude.isUpdated() || GPS.location.isUpdated()) {
    newData= true;
    if (!firstFix && GPS.Fix) {     //gps.fix
      GPSstartTime = GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second();
      firstFix = true;

    }
    if (getGPStime() > lastGPS && newData) {
      //openGPSlog();
      sensorSuite = "";
      sensorSuite += (flightTimeStr() + "," + String(GPS.location.lat(), 6) + "," + String(GPS.location.lng(), 6) + ",");
      sensorSuite += ((String(GPS.altitude.feet())) + ",");    //convert meters to feet for datalogging
      sensorSuite += (String(GPS.date.month()) + "/" + String(GPS.date.day()) + "/" + String(GPS.date.year()) + ",");
      sensorSuite += (String(GPS.time.hour()) + ":" + String(GPS.time.minute()) + ":" + String(GPS.time.second()) + ","); 
      if (GPS.Fix) { 
        sensorSuite += "fix,";
        lastGPS = GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second();
      }
      else{
        sensorSuite += ("No fix,");
        lastGPS = GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second();
      }
      sensorSuite += (String(x) + ", " + String(y) + ", " + String(z));
      sensorSuite += ",";
      sensorSuite += t4;
      //GPSlog.println(data);
      //closeGPSlog();
    }
  }
}
int getGPStime() {
  return (GPS.time.hour() * 3600 + GPS.time.minute() * 60 + GPS.time.second());
}

int getLastGPS() {    //returns time in seconds between last successful fix and initial fix. Used to match with altitude data
  if (!newDay && lastGPS < GPSstartTime) {
    days++;
    newDay = true;
  }
  else if (newDay && lastGPS > GPSstartTime)
    newDay = false;
  return days * 86400 + lastGPS;
}
