/*******
PowerTrol V0.0

This is the main firmware for the PowerTrol.  A home automation project aimed to bring automation in to every day life.
The goal of the firmware, PowerTrol and accompanying software is to make automation easy and accessible for everyone.

The genesis of this firmware is the AnodizationController firmware.

V0.0:   Changed pinouts for first PCB, "PTL-PCB001-00" 08/10/14
        Added switch recoginition on inputs 08/20/14
        Added Functions, eepromstore, eepromget. 08/20/14
          eepromstore:  runs after every time a variable is updated through the serial port to store all user variables
                        in EEPROM, if EEPROM is empty, function writes default values to EEPROM
          eepromget:    runs during setup to recover user variables from EEPROM
        Added EEPROM.xls to program directory to map variables to EEPROM 08/20/14
        Added EEPROMEx Library Folder to program folder 08/20/14
        Added EEPROMEx.h include 08/20/14
        08/20/14 - Added Serial Event function. Called upon incoming serial. Function listens for identifier characters and then
        responds to input by sending values or writing values depending of identifiers
          'r': read
          'w': write
          's': get data
          
        Function will look for 'r', 'w' or 'g' first.  If 'r' is sent, the function will read the value of the corresponding
        variable sent as the next character. If 'w' is sent, the function will look to the next character to determine
        which variable to change and then to all following characters for the value to store there. It will then update the EEPROM to the new value
        If 's' is sent, the function will begin outputing sensor and and socket information through the serial port.  Sending 's' again will
        stop sending data.

         

*******/

/*
***** User Variables *****
*/

//Sensor Setpoints
// int Sensor0_Target = 10;
// int Sensor1_Target = 10;
// int Sensor2_Target = 10;
// int Sensor3_Target = 10;

// //Over/Under Control
// boolean Sensor0_OverUnderAlarm = 0; //0: ALARM when Under 1: ALARM when Over
// boolean Sensor1_OverUnderAlarm = 0;
// boolean Sensor2_OverUnderAlarm = 0;
// boolean Sensor3_OverUnderAlarm = 0;

// //Sensor Map
// byte Sensor0_Map = 1; //0: Unregistered to Socket >0: Socket Number
// byte Sensor1_Map = 2;
// byte Sensor2_Map = 0;
// byte Sensor3_Map = 0;

// //Sensor Type
// byte Sensor0_Type = 0;
// byte Sensor1_Type = 0;
// byte Sensor2_Type = 0;
// byte Sensor3_Type = 0;

// //Power Draw
 
// byte Socket0_CurrentDraw = 3;
// byte Socket1_CurrentDraw = 3;

/*
***** Global Variables These should not need to be changed for general program settings ****
*/

//Includes
#include "EEPROMex.h"

//Pin Assignments
int Socket_Pin[2] = {2,3};
int Sensor_Pin[4] = {A3,A2,A1,A0};
int Sensor_Switch[4] = {4,5,10,11};
int txPin = 9;

const byte Num_Sockets = 2;
const byte Num_Sensors = 4;

//Serial Event
long variable, data, answer;
boolean mySwitch = 0;
boolean readorwrite = 0; //0: read data 1: write data
boolean negnumber = 0;
boolean SerialDisplay = 0; //0: Don't Output Data to Serial 1: Output Data to Serial
boolean altcommand = 0; //0: Command is for Read/Write 1: Command is for other actions

//sensors
byte sensorMap[] = {1,2,0,0}; //0: Unregistered to Socket >0: Socket Number
byte sensorType[] = {0,0,0,0};
int SensorActual[Num_Sensors];
int SensorTarget[] = {10,10,10,10};
boolean SensorOverUnderAlarm[] = {0,1,0,0}; //0: ALARM when Under 1: ALARM when Over
boolean SensorAlarmState[Num_Sensors];

//sockets
byte Max_Amps = 10;
boolean Is_On[Num_Sockets];
byte CurrentDraw[Num_Sockets];

//timers
unsigned long previousMillisSerialUpdate;
int SerialUpdateInterval = 1000; //ms


void setup(){
   //Initialize Serial
  Serial.begin(9600);
  delay(10);
  Serial.println("PowerTrol V0.0");

  //Read EEPROM
  eepromget();


  //Initialize Sensors
  for(byte i=0;i<Num_Sensors;i++){
    pinMode(Sensor_Pin[i], INPUT);
    pinMode(Sensor_Switch[i], INPUT);
  }

  //Initialize Sockets
  for(byte i=0;i<Num_Sockets;i++){
    pinMode(Socket_Pin[i], OUTPUT);
  }

  //Initialize Timers
  previousMillisSerialUpdate = millis();
}

void loop(){

  //Update Serial
  if((millis()-previousMillisSerialUpdate > SerialUpdateInterval) && SerialDisplay){
    previousMillisSerialUpdate = millis();
    for(byte i=0;i<Num_Sensors;i++){
      boolean Sensor_Present = !digitalRead(Sensor_Switch[i]);
      if(Sensor_Present == 1){
        Serial.print("Sensor ");Serial.print(i);Serial.print(": ");Serial.println(SensorActual[i]);
      }else{
        Serial.print("Sensor ");Serial.print(i);Serial.println(" NOT Present");
      }
    }
    for(byte i=0;i<Num_Sockets;i++){
      Serial.print("Socket ");Serial.print(i);Serial.print(": ");
      if(Is_On[i]){
        Serial.println("ON");
      }else{
        Serial.println(" ");
      }
    }
  }

  //Read Sensors
  for(byte i=0;i<Num_Sensors;i++){
    boolean Sensor_Present = !digitalRead(Sensor_Switch[i]);
    if(Sensor_Present == 1){
      if(sensorType[i] == 0){
        // int ADCReading = AnalogReadModeFilter(Sensor_Pin[i],100);
        // AdafruitThermistor.ReadADC(ADCReading);
        // AdafruitThermistor.CalculateTemperature(3);
        // SensorActual[i] = AdafruitThermistor.GetFarenheit();
        SensorActual[i] = AnalogReadModeFilter(Sensor_Pin[i],100);
        //Serial.print("Thermistor ");Serial.print(i, DEC);Serial.print(": ");Serial.println(SensorActual[i], 5);
      }else{
        SensorActual[i] = AnalogReadModeFilter(Sensor_Pin[i],100);
        //Serial.print("Sensor ");Serial.print(i, DEC);Serial.print(": ");Serial.println(SensorActual[i]);
      }
    }else{
      SensorActual[i] = 0;
    }
  }

  //Test Sensors
  for(byte i=0;i<Num_Sensors;i++){
    boolean Sensor_Present = !digitalRead(Sensor_Switch[i]);
    if(Sensor_Present == 1){
      if(SensorOverUnderAlarm[i]){
        if(SensorActual[i] > SensorTarget[i]){
          SensorAlarmState[i] = 1;
        }else{
          SensorAlarmState[i] = 0;
        }
      }else{
        if(SensorActual[i] < SensorTarget[i]){
          SensorAlarmState[i] = 1;
        }else{
          SensorAlarmState[i] = 0;
        }
      }
    }else{
      SensorAlarmState[i] = 0;
    }
  }

  //Socket Controller
  for(byte i=0; i<Num_Sensors;i++){
    if(sensorMap[i] != 0){
      byte j = sensorMap[i] - 1;
      if(SensorAlarmState[i]){
        Is_On[j] = 1;
        digitalWrite(Socket_Pin[j], HIGH);
      }else{
        Is_On[j] = 0;
        digitalWrite(Socket_Pin[j], LOW);
      }
    }
    
  }


}

















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

int AnalogReadModeFilter(unsigned char pin, unsigned char num_reads){
  int sortedValues[num_reads];
   for(int i=0;i<num_reads;i++){
     int value = analogRead(pin);
     int j;
     if(value<sortedValues[0] || i==0){
        j=0; //insert at first position
     }
     else{
       for(j=1;j<i;j++){
          if(sortedValues[j-1]<=value && sortedValues[j]>=value){
            // j is insert position
            break;
          }
       }
     }
     for(int k=i;k>j;k--){
       // move all values higher than current reading up one position
       sortedValues[k]=sortedValues[k-1];
     }
     sortedValues[j]=value; //insert current reading
   }
   //return scaled mode of 10 values
   int ADCAverage = 0;
   for(int i=num_reads/2-5;i<(num_reads/2+5);i++){
     ADCAverage +=sortedValues[i];
   }
   ADCAverage = ADCAverage/10;

   return ADCAverage;
}