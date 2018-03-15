# LED-Calendar-Clock

Description, Function, Features
original code developed on Particle Photon and working perfectly. Not the code listed here.
New code (found here) being enhanced/modified for NodeMCU/ESP8266 based boards
Wall clock with central perpetual calendar displaying month, weekday, and date.
LED's are SK6812RGBW, color code is in G,R,B,W value order when setting colors

60 pixel ring for clock
    - hours constantly illuminated
    - hour hand 3 led's wide
    - minute hand green
    - second hand blue
    
12 pixel ring for month
    - months colored by meterological season
    
7 pixel ring for week day
    - weekdays are green
    - weekends are red
    
31 pixel ring for date
    - weekends are red
    - weekdays are green
    
MAX of 19 LED's on at any given time with current setup

10k potientiometer for brightness adjustment

Time set via NTP server at power on and every few hours thereafter

USE PREDEFINED COLORS OR G,R,B,W VALUES TO SET COLORS

******* planned updates

-----thoughts for future improvement
- add light sensor for auto dim at night? 1/2 or 1/4 daytime brightness value based on ambient light
- CHANGE DISPLAY COLORS BASED ON TIME OF DAY (am, pm, morning, afternoon, evening, night)
