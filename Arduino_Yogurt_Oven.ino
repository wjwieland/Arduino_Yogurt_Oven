#include <Time.h>
//#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 
//#define HOW_LONG_HEADER "S"

#include <OneWire.h>
#include <DallasTemperature.h>
// Data wire is plugged into pin 7 on the Arduino
#define ONE_WIRE_BUS 7
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28, 0xC1, 0xD1, 0xDC, 0x06, 0x00, 0x00, 0xE7 };
//######################################################################################
unsigned long start_millis;
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
  // Start up the library
  pinMode(8, OUTPUT);
  setSyncProvider( requestSync);  //set function to call when sync required
  Serial.println("Waiting for sync message");
  Serial.println("Enter length of ferment time first.  ex. \'S4.5\' for four and and a half hours");
  Serial.println("Subsequently, set the clock by using \'Date +T%s\' ");
}
//#######################################################################################
 
void loop() {
  unsigned long int cycle_start = millis();
  if (Serial.available()> 1) {
    processHeader();
    processSyncMessage();
  }
  if ( (timeStatus() == timeSet) && (heat_off == false) && (timer_set == true) ) {
    digitalWrite(13, HIGH); // LED on if synced
    diff = second() - last;
    if (diff >= 3) {
      last = second();
      sensors.requestTemperatures();
      Serial.println("***************");
      Serial.println();
      Serial.println("Temperature is: ");
      tmp = printTemperature(insideThermometer);
      if (tmp < 98.40)  {
        digitalWrite(8, 1);
      }
      if (tmp > 98.60) {
        digitalWrite(8, 0);
      }
      if (timeStatus()!= timeNotSet) {
        digitalClockDisplay();  
      }
      if (time_left < 1 ) {
        Serial.println("Cook Complete, Yogurt Ready!");
        heat_off = true;
      }
      unsigned long int cycle_end = millis();
      unsigned long int cycle_length = cycle_end - cycle_start;
      Serial.print("Cycle length in milliseconds is ");
      Serial.println(cycle_length);
    }
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

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void processHeader() {
   char c = Serial.read();
   if(c == 'T') {
      isSync = true;
      isTimer = false;
      Serial.println(F("Seting Clock"));
   }
   else if (c == 'S') {
      isTimer = true;
      isSync = false;   
      Serial.println(F("Setting Cook Timer"));
   }  
}

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

//  if(Serial.find(TIME_HEADER)) {
    if(isSync == true) {
     pctime = Serial.parseInt();
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
       setTime(pctime); // Sync Arduino clock to the time received on the serial port
       adjustTime(-18000);  // compensate for local offset
       synced = true;
       heat_off = false;
     } else {
      setTime(DEFAULT_TIME);
      synced = false;
      heat_off = true;
     }
     start_time = now();
     start_millis = millis();
  }
  if(isTimer == true) {
    float length_cook = Serial.parseFloat(); 
    Serial.println("Timer set....");
    cook_timer = (3600000.0 * length_cook) + millis();
    timer_set = true;
  }
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}

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
  time_left = (cook_timer - millis()) / 60000.0;
  Serial.print(time_left);
//  printDigits(minute(time_left));
//  printDigits(second(time_left));
  Serial.println();
  
  Serial.print("Current Time Is ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  /*Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); */
  Serial.println(); 
  Serial.println(); 
}
