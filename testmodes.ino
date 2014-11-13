//TODO : Turn this into a library



bool OnOff(int actual, int target, bool alarm){
  bool alarmstate;
  if(alarm){
    if(actual > target){
      alarmstate = 1;
    }else{
      alarmstate = 0;
    }
  }else{
    if(actual < target){
      alarmstate = 1;
    }else{
      alarmstate = 0;
    }
  }
  return alarmstate;
}

bool TargetforPeriod(int actual, int target, bool alarm, unsigned long period, int periodtype, unsigned int tolerance, unsigned long settletime){ //2,2,0
  bool alarmstate;
  static unsigned long currentPeriod;
  static unsigned long previousPeriod;
  static unsigned long periodCounter;
  static unsigned long periodCounterLog;
  static unsigned long previousSettle;
  static bool settledFlag;
  static bool periodReachedFlag;


  if(periodtype == 0){        //milliseconds
    currentPeriod = millis();
  }else if(periodtype == 1){  //seconds
    currentPeriod = (millis()/1000);
  }else if(periodtype == 2){  //minutes
    currentPeriod = (millis()/60000);
  }else if(periodtype == 3){  //hours
    currentPeriod = (millis()/3600000);
  }else if(periodtype == 4){  //days
    currentPeriod = (millis()/86400000);
  }

  if(alarm){
    if(actual > target){
      alarmstate = 1;
    }else{
      alarmstate = 0;
    }
  }else{
    if(actual < target){
      alarmstate = 1;
    }else{
      alarmstate = 0;
    }
  }

  if(periodReachedFlag){
    alarmstate = 0;
  }else{
    if((actual >= (target - tolerance))&&(actual <= (target + tolerance))){
      if(millis()-previousSettle > settletime){
        settledFlag = 1;
      }
    }else{
      previousSettle = millis();
      settledFlag = 0;
    }
    if(settledFlag){
      periodCounter = (currentPeriod-previousPeriod)+periodCounterLog;
      if(periodCounter > period){
        periodReachedFlag = 1;
      }
    }else{
      periodCounterLog = periodCounter;
      previousPeriod = currentPeriod;
    }
  }


  return alarmstate;
}
