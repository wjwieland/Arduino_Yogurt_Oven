#include <DallasTemperature.h>
#include <OneWire.h>
#include <Time.h>

#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

// Data wire is plugged into pin 7 on the Arduino
#define ONE_WIRE_BUS 4
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28, 0xC1, 0xD1, 0xDC, 0x06, 0x00, 0x00, 0xE7 };
//######################################################################################
unsigned long start_millis;
unsigned long accum_millis;
unsigned long cycle_length;
time_t diff;
time_t last;
time_t start_time;
time_t acc_time;
time_t DEFAULT_TIME = 1357041600;
float cook_timer;
float time_left = 3600000;
float tmp;
boolean synced = false;
boolean heat_off = true;
boolean isTimer = false;
boolean isSync = false;
boolean timer_set = false;
//######################################################################################

void setup()
{
  Serial.begin(115200);  //Start the serial connection with the computer
                       //to view the result open the serial monitor 
  Serial.print("Program = yogurt_one_wire");
  Serial.println();
  pinMode(5, OUTPUT);
  Serial.println("Waiting for sync message");
  Serial.println("Enter length of ferment time first.  ex. \'S4.5\' for four and and a half hours");
  Serial.println("Subsequently, set the clock by using \'Date +T%s\' ");
}
//#######################################################################################
 
void loop() {
  unsigned long int cycle_start = millis();
  if ( (Serial.available() > 1) ) {
    processHeader();
  }
  diff = second() - last;
  if ( (diff >= 3) && ( ( timer_set == false) || (synced == false) ) ) {
    sensors.requestTemperatures();
    Serial.println("***************");
    Serial.println();
    Serial.println("Temperature is: ");
    float tempC = sensors.getTempC(insideThermometer);
    tmp = DallasTemperature::toFahrenheit(tempC);
    Serial.println(tmp);
    digitalClockDisplay();
    last = second();
  }
  if ( (diff >= 3) && (synced == true) && (timer_set == true) ) {
     heat_off = false;
     digitalWrite(13, HIGH); // LED on if synced
     sensors.requestTemperatures();
     float tempC = sensors.getTempC(insideThermometer);
     tmp = DallasTemperature::toFahrenheit(tempC);
     Serial.print("Temp is ");
     Serial.println(tmp);
     if (tmp < 114.60)  {
        digitalWrite(5, 1);
        Serial.println("Heat ON");
     }
     if (tmp >= 115.00) {
        digitalWrite(5, 0);
        Serial.println("Heat OFF");
     }
    if (time_left < 1 ) {
      Serial.println("Cook Complete, Yogurt Ready!");
      digitalWrite(5, 0);
      heat_off = true;
      timer_set = false;
      acc_time = 0;
    }
    unsigned long int cycle_end = millis();
    cycle_length = cycle_end- cycle_start;
    last = second();
    Serial.print("Cycle length is ");
    Serial.println(cycle_length);
    digitalClockDisplay();
    } else {
      digitalWrite(13, LOW);  // LED off if needs refresh
      heat_off = true;
    }
}

//################################################################
long unsigned int printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    Serial.print("Error getting temperature");
  } else {
    Serial.print("C: ");
    Serial.println(tempC);
    Serial.print("F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempC));
  }
  return (DallasTemperature::toFahrenheit(tempC));
}
//################################################################
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
    Serial.print(digits);
}
//################################################################
void processHeader() {
   char c = Serial.read();
   if(c == 'T') {
      isSync = true;
      isTimer = false;
   }
   else if (c == 'S') {
      isTimer = true;
      isSync = false;
   }  
   processSyncMessage(); 
}
//################################################################
void processSyncMessage() {
  unsigned long pctime;
    if(isSync == true) {
     pctime = Serial.parseInt();
     setTime(pctime); // Sync Arduino clock to the time received on the serial port
     adjustTime(-18000);  // compensate for local offset
     synced = true;
     heat_off = false;
     isSync = false;
     Serial.println("Clock Set");
  }
  if(isTimer == true) {
    float length_cook = Serial.parseFloat(); 
    Serial.println("Timer set....");
    cook_timer = (3600000.0 * length_cook) + millis();
    time_left = (cook_timer - millis()) / 60000.0;
    start_time = now();
    timer_set = true;
    isTimer == false;
  }
}
//################################################################
void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print("Start Time: ");
  Serial.print(hour(start_time));
  printDigits(minute(start_time));
  printDigits(second(start_time));
  Serial.println();
  Serial.print("Accumulated Time ");
  acc_time = now() - start_time;
  Serial.print(hour(acc_time));
  printDigits(minute(acc_time));
  printDigits(second(acc_time));
  Serial.println();

  Serial.print("Time Left ");
  if (synced == true) {
    time_left = (cook_timer - millis()) / 60000.0;
  }
  Serial.print(time_left);
  printDigits(minute(time_left));
  printDigits(second(time_left));
  Serial.println();
  
  Serial.print("Current Time Is ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.println(); 
  Serial.println(); 
}
