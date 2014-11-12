void serialEvent() { //called upon incoming serial
 /*  check if data has been sent from the computer: */
  while (Serial.available()) {
    /* read the most recent byte */
    byte byteRead = Serial.read();
    
    //listen for numbers between 0-9
    if(byteRead>47 && byteRead<58){
       //number found
       if(!mySwitch){
         variable=(variable*10)+(byteRead-48);
       }else{
         data=(data*10)+(byteRead-48);
       }
    }
    
    /*Listen for an equal sign (byte code 61) 
      to calculate the answer and send it back to the
      serial monitor screen*/
    else if(byteRead=='\n'){
      if(altcommand == 0){
        if(readorwrite){
          if(negnumber){
            data = -data;
          }
          switch (variable) {
            case 1:
              SensorTarget[0] = data;
              break;
            case 2:
              SensorTarget[1] = data;
              break;
            case 3:
              SensorTarget[2] = data;
              break;
            case 4:
              SensorTarget[3] = data;
              break;
            case 5:
              sensorMap[0] = data;
              break;
            case 6:
              sensorMap[1] = data;
              break;
            case 7:
              sensorMap[2] = data;
              break;
            case 8: 
              sensorMap[3] = data;
              break;
            case 9:
              sensorType[0] = data;
              break;
            case 10:
              sensorType[1] = data;
              break;
            case 11:
              sensorType[2] = data;
              break;
            case 12:
              sensorType[3] = data;
              break;
            case 13:
              Max_Amps = data;
              break;
            case 14:
              CurrentDraw[0] = data;
              break;
            case 15:
              CurrentDraw[1] = data;
              break;
            case 16:
              SensorOverUnderAlarm[0] = data;
              break;
            case 17:
              SensorOverUnderAlarm[1] = data;
              break;
            case 18:
              SensorOverUnderAlarm[2] = data;
              break;
            case 19:
              SensorOverUnderAlarm[3] = data;
              break;
            default:
              Serial.println("Unrecognized Variable");
              break;
          }
          eepromstore();

        }else{
          switch (variable) {
            case 0:
              eepromget();
              break;
            case 1:
              Serial.print("Sensor 0 Target: ");Serial.println(SensorTarget[0]);
              break;
            case 2:
              Serial.print("Sensor 1 Target: ");Serial.println(SensorTarget[1]);
              break;
            case 3:
              Serial.print("Sensor 2 Target: ");Serial.println(SensorTarget[2]);
              break;
            case 4:
              Serial.print("Sensor 3 Target: ");Serial.println(SensorTarget[3]);
              break;
            case 5:
              Serial.print("Sensor 0 Map: ");Serial.println(sensorMap[0]);
              break;
            case 6:
              Serial.print("Sensor 1 Map: ");Serial.println(sensorMap[1]);
              break;
            case 7:
              Serial.print("Sensor 2 Map: ");Serial.println(sensorMap[2]);
              break;
            case 8: 
              Serial.print("Sensor 3 Map: ");Serial.println(sensorMap[3]);
              break;
            case 9: 
              Serial.print("Sensor 0 Type: ");Serial.println(sensorType[0]);
              break;
            case 10: 
              Serial.print("Sensor 1 Type: ");Serial.println(sensorType[0]);
              break;
            case 11: 
              Serial.print("Sensor 2 Type: ");Serial.println(sensorType[0]);
              break;
            case 12: 
              Serial.print("Sensor 3 Type: ");Serial.println(sensorType[0]);
              break;
            case 13: 
              Serial.print("Max_Amps: ");Serial.println(Max_Amps);
              break;
            case 14: 
              Serial.print("Socket 1 CurrentDraw: ");Serial.println(CurrentDraw[0]);
              break;
            case 15: 
              Serial.print("Socket 2 CurrentDraw: ");Serial.println(CurrentDraw[1]);
              break;
            case 16: 
              Serial.print("Sensor 0 OverUnderAlarm: ");Serial.println(SensorOverUnderAlarm[0]);
              break;
            case 17: 
              Serial.print("Sensor 1 OverUnderAlarm: ");Serial.println(SensorOverUnderAlarm[1]);
              break;
            case 18: 
              Serial.print("Sensor 2 OverUnderAlarm: ");Serial.println(SensorOverUnderAlarm[2]);
              break;
            case 19: 
              Serial.print("Sensor 3 OverUnderAlarm: ");Serial.println(SensorOverUnderAlarm[3]);
              break;
            default:
              Serial.println("Unrecognized Variable");
              break;
          }
        }
        /* Reset the variables for the next round */
        variable=0;
        data=0;
        mySwitch=false;
        readorwrite = 0;
        negnumber = 0;
        altcommand = 0;
      }
      
    /* Listen for the addition sign (byte code 43). This is
       used as a delimiter to help define variable from data */  
    }else if (byteRead== '='){
      mySwitch=true;
    }else if (byteRead == 'r'){
      readorwrite = 0;
      altcommand = 0;
    }else if (byteRead == 'w'){
      readorwrite = 1;
      altcommand = 0;
    }else if (byteRead == 's'){
      SerialDisplay = !SerialDisplay;
      altcommand = 1;
    }else if (byteRead == '-'){
      negnumber = 1;
    }else{
      Serial.println("Unrecognized Command");
    }

  }
}

void eepromstore(){
  Serial.println("Updating EEPROM");
  //EEPROM Status 
    EEPROM.updateBit(0,0,1);
  //Sensor0 Target (Int)
    EEPROM.updateInt(1,SensorTarget[0]);
  //Sensor1 Target (Int)
    EEPROM.updateInt(3,SensorTarget[1]);
  //Sensor2 Target (Int)
    EEPROM.updateInt(5,SensorTarget[2]);
  //Sensor3 Target (Int)
    EEPROM.updateInt(7,SensorTarget[3]);
  //Sensor0 Map (Byte)
    EEPROM.updateByte(9,sensorMap[0]);
  //Sensor1 Map (Byte)
    EEPROM.updateByte(10,sensorMap[1]);
  //Sensor2 Map (Byte)
    EEPROM.updateByte(11,sensorMap[2]);
  //Sensor3 Map (Byte)
    EEPROM.updateByte(12,sensorMap[3]);
  //Sensor0 Type (Byte)
    EEPROM.updateByte(13,sensorType[0]);
  //Sensor1 Type (Byte)
    EEPROM.updateByte(14,sensorType[1]);
  //Sensor2 Type (Byte)
    EEPROM.updateByte(15,sensorType[2]);
  //Sensor3 Type (Byte)
    EEPROM.updateByte(16,sensorType[3]);
  //Max_Amps (Byte)
    EEPROM.updateByte(17,Max_Amps);
  //Socket0 CurrentDraw (Byte)
    EEPROM.updateByte(18,CurrentDraw[0]);
  //Socket1 CurrentDraw (Byte)
    EEPROM.updateByte(19,CurrentDraw[1]);
  //Sensor0 OverUnderAlarm (Bool)
    EEPROM.updateBit(0,1,SensorOverUnderAlarm[0]);
  //Sensor1 OverUnderAlarm (Bool)
    EEPROM.updateBit(0,2,SensorOverUnderAlarm[1]);
  //Sensor2 OverUnderAlarm (Bool)
    EEPROM.updateBit(0,3,SensorOverUnderAlarm[2]);
  //Sensor3 OverUnderAlarm (Bool)
    EEPROM.updateBit(0,4,SensorOverUnderAlarm[3]);
}

void eepromget(){
  Serial.println("Checking EEPROM");
  boolean eepromStatus = EEPROM.readBit(0,0);
  if(eepromStatus == 0){
    Serial.println("Initializing EEPROM");
    EEPROM.writeBit(0,0,1);
    eepromstore();
  }else{
    Serial.println("Retrieving EEPROM Settings");
    //Sensor0 Target (Int)
      SensorTarget[0] = EEPROM.readInt(1);
    //Sensor1 Target (Int)
      SensorTarget[1] = EEPROM.readInt(3);
    //Sensor2 Target (Int)
      SensorTarget[2] = EEPROM.readInt(5);
    //Sensor3 Target (Int)
      SensorTarget[3] = EEPROM.readInt(7);
    //Sensor0 Map (Byte)
      sensorMap[0] = EEPROM.readByte(9);
    //Sensor1 Map (Byte)
      sensorMap[1] = EEPROM.readByte(10);
    //Sensor2 Map (Byte)
      sensorMap[2] = EEPROM.readByte(11);
    //Sensor3 Map (Byte)
      sensorMap[3] = EEPROM.readByte(12);
    //Sensor0 Type (Byte)
      sensorType[0] = EEPROM.readByte(13);
    //Sensor1 Type (Byte)
      sensorType[1] = EEPROM.readByte(14);
    //Sensor2 Type (Byte)
      sensorType[2] = EEPROM.readByte(15);
    //Sensor3 Type (Byte)
      sensorType[3] = EEPROM.readByte(16);
    //Max Amps (byte)
      Max_Amps = EEPROM.readByte(17);
    //Socket0 CurrentDraw (Byte)
      CurrentDraw[0] = EEPROM.readByte(18);
    //Socket1 CurrentDraw (Byte)
      CurrentDraw[1] = EEPROM.readByte(19);
    //Sensor0 OverUnderAlarm (Bit)
      SensorOverUnderAlarm[0] = EEPROM.readBit(0,1);
    //Sensor1 OverUnderAlarm (Bit)
      SensorOverUnderAlarm[1] = EEPROM.readBit(0,2);
    //Sensor2 OverUnderAlarm (Bit)
      SensorOverUnderAlarm[2] = EEPROM.readBit(0,3);
    //Sensor3 OverUnderAlarm (Bit)
      SensorOverUnderAlarm[3] = EEPROM.readBit(0,4);

    Serial.println(" ");

    Serial.print("1:  Sensor0_Target: ");Serial.println(SensorTarget[0]);
    Serial.print("2:  Sensor1_Target: ");Serial.println(SensorTarget[1]);
    Serial.print("3:  Sensor2_Target: ");Serial.println(SensorTarget[2]);
    Serial.print("4:  Sensor3_Target: ");Serial.println(SensorTarget[3]);
    Serial.print("5:  Sensor0_Map: ");Serial.println(sensorMap[0]);
    Serial.print("6:  Sensor1_Map: ");Serial.println(sensorMap[1]);
    Serial.print("7:  Sensor2_Map: ");Serial.println(sensorMap[2]);
    Serial.print("8:  Sensor3_Map: ");Serial.println(sensorMap[3]);
    Serial.print("9:  Sensor0_Type: ");Serial.println(sensorType[0]);
    Serial.print("10: Sensor1_Type: ");Serial.println(sensorType[1]);
    Serial.print("11: Sensor2_Type: ");Serial.println(sensorType[2]);
    Serial.print("12: Sensor3_Type: ");Serial.println(sensorType[3]);
    Serial.print("13: Max_Amps: ");Serial.println(Max_Amps);
    Serial.print("14: Socket0_CurrentDraw: ");Serial.println(CurrentDraw[0]);
    Serial.print("15: Socket1_CurrentDraw: ");Serial.println(CurrentDraw[1]);
    Serial.print("16: Sensor0_OverUnderAlarm: ");Serial.println(SensorOverUnderAlarm[0]);
    Serial.print("17: Sensor1_OverUnderAlarm: ");Serial.println(SensorOverUnderAlarm[1]);
    Serial.print("18: Sensor2_OverUnderAlarm: ");Serial.println(SensorOverUnderAlarm[2]);
    Serial.print("19: Sensor3_OverUnderAlarm: ");Serial.println(SensorOverUnderAlarm[3]);

    Serial.println(" ");

  }
}
