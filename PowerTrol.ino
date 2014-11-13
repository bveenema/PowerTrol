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
#include "Thermistor5.h"

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
byte sensorType[] = {0,0,0,0}; //0: Undeclared 1:SparkFun Thermistor
int SensorActual[Num_Sensors];
int SensorTarget[] = {10,10,10,10};
byte SensorTestMode[] = {1,0,0,0}; //0: ON/OFF 1: TargetforPerios
boolean SensorOverUnderAlarm[] = {0,1,0,0}; //0: ALARM when Under 1: ALARM when Over
boolean SensorAlarmState[Num_Sensors];

//sockets
byte Max_Amps = 10;
boolean Is_On[Num_Sockets];
byte CurrentDraw[Num_Sockets];

//timers
unsigned long previousMillisSerialUpdate;
int SerialUpdateInterval = 1000; //ms

//Sensor Instanciations
Thermistor5 SparkFunThermistor;

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
    if(sensorType[i] == 1){         //SparkFun Thermistor
      SparkFunThermistor.Pin = Sensor_Pin[i];
      SparkFunThermistor.SetUp();
      SparkFunThermistor.BitResolution=pow(2,10)-1;
      SparkFunThermistor.VoltageSupply = 5.0;
      SparkFunThermistor.ResistanceFixed = 9800;  //9800
      SparkFunThermistor.NominalResistance = 10000;
      SparkFunThermistor.Offset = 0;
      SparkFunThermistor.NominalTemp = 25;
      SparkFunThermistor.BValue = 3977;
    }
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
        Serial.print(F("Sensor "));Serial.print(i);Serial.print(F(": "));Serial.print(SensorActual[i]);
        if(sensorType[i] == 1){
          Serial.println(F(" F"));
        }else{
          Serial.println(F(" "));
        }
      }else{
        Serial.print(F("Sensor "));Serial.print(i);Serial.println(F(" NOT Present"));
      }
    }
    for(byte i=0;i<Num_Sockets;i++){
      Serial.print(F("Socket "));Serial.print(i);Serial.print(F(": "));
      if(Is_On[i]){
        Serial.println(F("ON"));
      }else{
        Serial.println(F(" "));
      }
    }
  }

  //

  //Read Sensors
  for(byte i=0;i<Num_Sensors;i++){
    boolean Sensor_Present = !digitalRead(Sensor_Switch[i]);
    if(Sensor_Present == 1){
      if(sensorType[i] == 0){
        SensorActual[i] = AnalogReadModeFilter(Sensor_Pin[i],100);
        //Serial.print("Thermistor ");Serial.print(i, DEC);Serial.print(": ");Serial.println(SensorActual[i], 5);
      }else if (sensorType[i] == 1){
        int ADCReading = AnalogReadModeFilter(Sensor_Pin[i],100);
        SparkFunThermistor.ReadADC(ADCReading);
        SparkFunThermistor.CalculateTemperature(3);
        SensorActual[i] = SparkFunThermistor.GetFarenheit();
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
      if(SensorTestMode[i] == 0){
        SensorAlarmState[i] = OnOff(SensorActual[i],SensorTarget[i],SensorOverUnderAlarm[i]);
      }else if(SensorTestMode[i] == 1){
        SensorAlarmState[i] = TargetforPeriod(SensorActual[i],SensorTarget[i],SensorOverUnderAlarm[i],110,2,2,120000);
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