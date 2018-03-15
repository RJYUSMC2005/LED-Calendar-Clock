/*  //////// Using WiFi Manager utility for WiFi Credentials on initial connection.
 *   Connect phone ot computer to "ClockAP"
 *   Go to 192.168.4.1 to enter WiFi Credentials
 *   Reboot Clock   
 *   
 *   ///////  planned added features
 * if time is between 9pm & 6am, hour markers to red, brightness value to 1/2, hands to other dim color perhaps 
 *    orangeish in nature
 */

///////////// INCLUSIONS
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <String.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <WifiUDP.h>
#include <WiFiManager.h>
#include <Wire.h>

///////////// WiFi CONFIGURATIONS
// DEFINE HERE THE KNOWN NETWORKS
const char* KNOWN_SSID[] = {"SSID1", "SSID2"};
const char* KNOWN_PASSWORD[] = {"SSID1PWD", "SSID2PWD"};
const int   KNOWN_SSID_COUNT = sizeof(KNOWN_SSID) / sizeof(KNOWN_SSID[0]); // number of known networks
WiFiManager wifiManager;



///////////// NTP CONFIGURATION
#define NTP_OFFSET   (60 * 60)      // In seconds
#define NTP_INTERVAL (30*(60*1000))    // In miliseconds (HOUR*(MIN*(SEC*MILLISEC))) - (30*(60*1000)) = 30 Minute update interval
#define NTP_ADDRESS  "pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

///////////// NEOPIXEL CONFIGURATION
//pin configuraion
#define ClockPin  14                  // clock
#define MonthPin  D1                  // Month
#define WDayPin   D2                  // weekday
#define DatePin   D3                  // date
#define PIXEL_TYPE NEO_RGBW           // RGBW led ring
int NumClockPixels  = 60;
int NumMonmthPixels = 12;
int NumWDayPixels   = 7;
int NumDatePixels   = 31;

//led strip configuration
Adafruit_NeoPixel ClockStrip = Adafruit_NeoPixel(NumClockPixels,  ClockPin, PIXEL_TYPE + NEO_KHZ800);     // clock
Adafruit_NeoPixel MonthPixel = Adafruit_NeoPixel(NumMonmthPixels, MonthPin, PIXEL_TYPE + NEO_KHZ800);     // month
Adafruit_NeoPixel WDayPixel  = Adafruit_NeoPixel(NumWDayPixels,   WDayPin,  PIXEL_TYPE + NEO_KHZ800);     // weekday
Adafruit_NeoPixel DatePixel  = Adafruit_NeoPixel(NumDatePixels,   DatePin,  PIXEL_TYPE + NEO_KHZ800);     // date

//pre-defined color values
////    COLOR       G,   R,   B,   W
#define RED         0,   255, 0,   0
#define GREEN       255, 0,   0,   0
#define BLUE        0,   0,   255, 0
#define WHITE       0,   0,   0,   255
#define CYAN        150, 10,  70,  0
#define PEACH       50,  200, 5,   0
#define PURPLE      0,   180, 180, 0
#define REDORANGE   50,  250, 0,   0
#define ORANGE      100, 255, 0,   0
#define OFF         0,   0,   0,   0
#define LIGHTBLUE   236, 181, 255, 0
#define MIDBLUE     205, 0,   251, 0
#define FALL        142, 254, 0,   0
#define WINTER      205, 0,   251, 0
#define SPRING      236, 102, 0,   0
#define SUMMER      0,   255, 0,   0
#define PINK        0,   150, 0,   100

///////////// LED Brightness Adjustment
#define BrightPin   A0                  // LED brightness adjustment pot pin
int LEDBrightness = 5;
int BrightValue   = 0;

///////////// DATE CONFIGURATION
String date;
String t;
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

///////////// CLOCK VARIABLES
byte secondval = second();          // get seconds
byte minuteval = minute();          // get minutes
byte hourval   = hourFormat12();    // get hours
int  SecClear  = (second()-2);       // for clearing previous second
unsigned long lastSync = millis();

////////////////// Initialization
void setup () 
{
  Serial.begin(115200); // most ESP-01's use 115200 but this could vary
  timeClient.begin();   // Start the NTP UDP client
  wifiManager.autoConnect("AutoConnectAP");
}

////////////////// Main Program
void loop()                     // (Serial.print) texts located at end of LOOP() for testing
{    
// clear the variables
    date = "";  
    t = "";

// update the NTP client and get the UNIX UTC timestamp 
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();

// convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;

// Then convert the UTC UNIX timestamp to local time
    TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
    TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -240};   //UTC - 4 hours - change this as needed
    Timezone usEastern(usEDT, usEST);
    local = usEastern.toLocal(utc);

// now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local)-1];
    date += ", ";
    date += months[month(local)-1];
    date += " ";
    date += day(local);
    date += ", ";
    date += year(local);

// format the time to 12-hour format with AM/PM and no seconds
    t += hourFormat12(local);
    t += ":";
    if(minute(local) < 10)  // add a zero if minute is under 10
      t += "0";
    t += minute(local);
    t += ":";
    if(second(local) < 10)  // add a zero if minute is under 10
    t += "0";
    t += second(local);
    t += " ";
    t += ampm[isPM(local)];
      
// get brightness value from pot
   BrightValue = analogRead(BrightPin);
   LEDBrightness = map(BrightValue,0,1024,15,250);  //1024 max reading from pot, mapped to 255 max brightness value for LED's

// CLOCK OPERATIONS
   if (hour() >= 21 || hour() <= 6)
   {
    // set hour markers always on to given color
    HourMarkerNight();

// turn on LED's for clock
    secondval = second(local);               // get seconds
    minuteval = minute(local);               // get minutes
    hourval   = hour(local);                 // get hours
    ClockStrip.setBrightness(LEDBrightness); // default 15
    
// clock hands
    if(hourval > 11) hourval -= 12;
    {
        hourval = (hourval*60 + minuteval) / 12; 
    }
    if  (hourval-2 ==  0 || hourval-2 == 5  || hourval-2 == 10 || hourval-2 == 15 ||
         hourval-2 == 20 || hourval-2 == 25 || hourval-2 == 30 || hourval-2 == 35 ||
         hourval-2 == 40 || hourval-2 == 45 || hourval-2 == 50 || hourval-2 == 55)
    {
        HourMarkerNight();
    }
// ensure hour-2 is cleared when hour hand advances
    else
    {
        ClockStrip.setPixelColor(hourval-2,OFF);             
    }
    
    if (hourval == 0 || hourval == 12)
    {
        ClockStrip.setPixelColor(59, REDORANGE);         // fix for midnight and noon hour marker not illuminating properly
    }
    else
    {
        ClockStrip.setPixelColor(hourval-1, REDORANGE); 
    }
    ClockStrip.setPixelColor(hourval,   RED);              // hour center marker
    ClockStrip.setPixelColor(hourval+1, RED);      // hour +1 marker
    ClockStrip.setPixelColor(minuteval, ORANGE);          // minute marker
    ClockStrip.setPixelColor(secondval, PINK);           // second marker
    ClockStrip.show();

    // clear previous values but do not turn off to ensure they do not stay on as the hands advance. 
    // this does not turn them off since "strip.show()" isn't called, it just clears the current values
    ClockStrip.setPixelColor(hourval,   OFF);
    ClockStrip.setPixelColor(minuteval, OFF);
    ClockStrip.setPixelColor(secondval, OFF);

// turn on LED for month colored be season
// March - May = spring
    if (month(local) == 3 || month(local) == 4 || month(local) == 5 )          
    {
        MonthPixel.setPixelColor((month(local)-1), RED);
    }
// June - Aug = summer
    else if (month(local) == 6 || month(local) == 7 ||month(local) == 8 )     
    {
        MonthPixel.setPixelColor((month(local)-1), RED);
    }
// Sept - Nov = Fall
    else if (month(local) == 9 || month(local) == 10 || month(local) == 11 )   
    {
        MonthPixel.setPixelColor((month(local)-1), RED);
    }    
// Dec - Feb = Winter
    else
    {
        MonthPixel.setPixelColor((month(local)-1), RED);                        
    }
    
// set brightness and turn LED's on
    MonthPixel.setPixelColor((month(local)-2), OFF);
    MonthPixel.setBrightness(LEDBrightness);
    MonthPixel.show();

// turn on LED for weekday
    if (weekday(local) == 7 || weekday(local) == 1)
    {
        WDayPixel.setPixelColor((weekday(local)-1), RED);
    }
    else 
    {
        WDayPixel.setPixelColor((weekday(local)-1), RED);
    }

    WDayPixel.setPixelColor((weekday(local)-2), OFF);
    WDayPixel.setBrightness(LEDBrightness);
    WDayPixel.show();
   
// turn on LED for date
// DateMarker();
    if (weekday(local) == 7 || weekday(local) == 1)
    {
        DatePixel.setPixelColor((day(local)-1), RED);
    }
    else 
    {
        DatePixel.setPixelColor((day(local)-1), RED);
    }
    DatePixel.setPixelColor((day(local)-2), OFF);
    DatePixel.setBrightness(LEDBrightness);
    DatePixel.show();;
   }
   else
   {
    
  // set hour markers always on to given color
    HourMarkerDay();

// turn on LED's for clock
    secondval = second(local);               // get seconds
    minuteval = minute(local);               // get minutes
    hourval   = hour(local);                 // get hours
    ClockStrip.setBrightness(LEDBrightness); // default 15
    
// clock hands
    if(hourval > 11) hourval -= 12;
    {
        hourval = (hourval*60 + minuteval) / 12; 
    }
    if  (hourval-2 ==  0 || hourval-2 == 5  || hourval-2 == 10 || hourval-2 == 15 ||
         hourval-2 == 20 || hourval-2 == 25 || hourval-2 == 30 || hourval-2 == 35 ||
         hourval-2 == 40 || hourval-2 == 45 || hourval-2 == 50 || hourval-2 == 55)
    {
        HourMarkerDay();
    }
// ensure hour-2 is cleared when hour hand advances
    else
    {
        ClockStrip.setPixelColor(hourval-2,OFF);             
    }
    
    if (hourval == 0 || hourval == 12)
    {
        ClockStrip.setPixelColor(59, REDORANGE);         // fix for midnight and noon hour marker not illuminating properly
    }
    else
    {
        ClockStrip.setPixelColor(hourval-1, REDORANGE); 
    }
    ClockStrip.setPixelColor(hourval,   RED);              // hour center marker
    ClockStrip.setPixelColor(hourval+1, REDORANGE);      // hour +1 marker
    ClockStrip.setPixelColor(minuteval, GREEN);          // minute marker
    ClockStrip.setPixelColor(secondval, BLUE);           // second marker
    ClockStrip.show();

    // clear previous values but do not turn off to ensure they do not stay on as the hands advance. 
    // this does not turn them off since "strip.show()" isn't called, it just clears the current values
    ClockStrip.setPixelColor(hourval,   OFF);
    ClockStrip.setPixelColor(minuteval, OFF);
    ClockStrip.setPixelColor(secondval, OFF);

// turn on LED for month colored be season
// March - May = spring
    if (month(local) == 3 || month(local) == 4 || month(local) == 5 )          
    {
        MonthPixel.setPixelColor((month(local)-1), SPRING);
    }
// June - Aug = summer
    else if (month(local) == 6 || month(local) == 7 ||month(local) == 8 )     
    {
        MonthPixel.setPixelColor((month(local)-1), SUMMER);
    }
// Sept - Nov = Fall
    else if (month(local) == 9 || month(local) == 10 || month(local) == 11 )   
    {
        MonthPixel.setPixelColor((month(local)-1), FALL);
    }    
// Dec - Feb = Winter
    else
    {
        MonthPixel.setPixelColor((month(local)-1), WINTER);                        
    }
    
// set brightness and turn LED's on
    MonthPixel.setPixelColor((month(local)-2), OFF);
    MonthPixel.setBrightness(LEDBrightness);
    MonthPixel.show();

// turn on LED for weekday
    if (weekday(local) == 7 || weekday(local) == 1)
    {
        WDayPixel.setPixelColor((weekday(local)-1), RED);
    }
    else 
    {
        WDayPixel.setPixelColor((weekday(local)-1), GREEN);
    }

    WDayPixel.setPixelColor((weekday(local)-2), OFF);
    WDayPixel.setBrightness(LEDBrightness);
    WDayPixel.show();
   
// turn on LED for date
// DateMarker();
    if (weekday(local) == 7 || weekday(local) == 1)
    {
        DatePixel.setPixelColor((day(local)-1), RED);
    }
    else 
    {
        DatePixel.setPixelColor((day(local)-1), GREEN);
    }
    DatePixel.setPixelColor((day(local)-2), OFF);
    DatePixel.setBrightness(LEDBrightness);
    DatePixel.show();
   }
        
//  SERIAL DEBUGGING  
  
    Serial.println("");
    Serial.print("Brightnes POT set to = ");
    Serial.println(BrightValue);    
    Serial.print("Brightnes set to = ");
    Serial.println(LEDBrightness);
    Serial.print("Local date: ");
    Serial.println(date);
    Serial.print("Local time: ");
    Serial.println(t);
    Serial.print("hour: ");
    Serial.println(hour(local));
    Serial.print("minute: ");
    Serial.println(minute(local));
    Serial.print("second: ");
    Serial.println(second(local));
    Serial.print("month: ");
    Serial.println(month(local));
    Serial.print("weekday: ");
    Serial.println(weekday(local));
    Serial.print("day: ");
    Serial.println(day(local));
//  1 sec delay before repeating loop
    delay(1000);
}
//////////////////  END OF MAIN APP /////////////////////

////////////////// Turn on LED's for Hour Markers on Clock Ring
void HourMarkerDay()               //     SET COLORS FOR HOUR MARKERS
{
//  *****   SET ALL HOUR MARKERS TO THE SAME COLOR
  for (int j = 0; j < NumClockPixels; j = j + (NumClockPixels/12))
  {
        ClockStrip.setPixelColor(j, ClockStrip.Color(WHITE));
  }
    
//  *****   SET 3, 6, 9 & 12 HOUR MARKERS TO STAND OUT
  for (int i = 0; i < NumClockPixels; i = i + (NumClockPixels/4))
  {
        ClockStrip.setPixelColor(i, ClockStrip.Color(MIDBLUE));
  }
}

////////////////// Turn on LED's for Hour Markers on Clock Ring @ night
void HourMarkerNight()               //     SET COLORS FOR HOUR MARKERS
{
//  *****   SET ALL HOUR MARKERS TO THE SAME COLOR
  for (int j = 0; j < NumClockPixels; j = j + (NumClockPixels/12))
  {
        ClockStrip.setPixelColor(j, ClockStrip.Color(RED));
  }
    
//  *****   SET 3, 6, 9 & 12 HOUR MARKERS TO STAND OUT
  for (int i = 0; i < NumClockPixels; i = i + (NumClockPixels/4))
  {
        ClockStrip.setPixelColor(i, ClockStrip.Color(ORANGE));
  }
}
