// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef DRIVER_H
#define DRIVER_H

typedef void (*DriverFunc)(void);

struct Driver {
  int* pollInterval;
  DriverFunc setupFunc;
  DriverFunc loopFunc;
  bool* enabled;
};

extern Driver drv_sACN;
extern Driver drv_webServer;
extern Driver drv_OTA;
extern Driver drv_artnet;

Driver drivers[] = {
  drv_webServer,
  drv_sACN,
  drv_artnet,
  drv_OTA,
};

const int num_drivers = sizeof(drivers)/sizeof(Driver);
unsigned long lastCall[num_drivers];


// TODO: put this back to checking if things are enabled and 
// restart to enable things...

void driverSetup() {
  //Serial.print("Size of drivers: ");
  //Serial.println(sizeof(drivers)/sizeof(Driver));
  //Serial.print("Size of lastCall: ");
  //Serial.println(sizeof(lastCall)/sizeof(unsigned long));
  for (int i=0; i<num_drivers; i++) {
    lastCall[i] = 0;
    drivers[i].setupFunc();
    //Serial.println(lastCall[i]);
    //if (*(drivers[i].enabled)) {
      
    //}
  }
}

//TODO: get this out of the .h
void driverLoop() {
  for(int i=0; i<num_drivers; i++) {
    if (!*(drivers[i].enabled)) {
      continue;
    }
    unsigned long thisTime = millis();
    int pollInterval = *(drivers[i].pollInterval);
    if (thisTime > lastCall[i] + pollInterval) {
      drivers[i].loopFunc();
      //Serial.print("setting lastCall(");
      //Serial.print(i);
      //Serial.print(") =  ");
      //Serial.println(thisTime);
      lastCall[i] = thisTime;
    }

    //if (thisTime > lastCall[i] + 2*pollInterval) {
      // we are running behind! alert the user
      //Serial.println("Running behind - unable to service all driver requests");
    //}
  }
}


#endif