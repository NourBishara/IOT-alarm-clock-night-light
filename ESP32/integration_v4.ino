#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>
#include <WiFi.h>
#include "time.h"
#include "sntp.h"
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <HardwareSerial.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <iostream>
#include <sstream>
#include <string>
#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>

//////////////////////////MATRIX
// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

// Defining size, and output pins,MATRIX
#define MAX_DEVICES 4
#define CS_PIN 15
#define SCK_PIN 14
#define MOSI_PIN 13

MD_Parola Display = MD_Parola(HARDWARE_TYPE, MOSI_PIN, SCK_PIN, CS_PIN, MAX_DEVICES);


//////////////////mp3
static int8_t select_SD_card[] = {0x7e, 0x03, 0X35, 0x01, 0xef}; // 7E 03 35 01 EF
static int8_t play_first_song[] = {0x7e, 0x04, 0x41, 0x00, 0x01, 0xef};// 7E 04 42 01 01 EF - plays first song
static int8_t play_first_song_cycle[] = {0x7e, 0x04, 0x33, 0x00, 0x01, 0xef};//7E 04 33 00 01 EF
static int8_t play_second_song[] = {0x7e, 0x04, 0x41, 0x00, 0x02, 0xef}; // play second song
static int8_t play_third_song[] = {0x7e, 0x04, 0x41, 0x00, 0x03, 0xef}; // 7E 04 41 00 03 EF
static int8_t play_fourth_song[] = {0x7e, 0x04, 0x41, 0x00, 0x04, 0xef}; // 7E 04 41 00 04 EF
static int8_t play_fifth_song[] = {0x7e, 0x04, 0x41, 0x00, 0x05, 0xef}; // 7E 04 41 00 05 EF
static int8_t play_song[] = {0x7e, 0x02, 0x01, 0xef}; //7E 02 01 EF - play / resume
static int8_t pause_song[] = {0x7e, 0x02, 0x02, 0xef}; //7E 02 02 EF - pause
static int8_t set_volume[] = {0x7e, 0x03, 0x31, 0x04, 0xef}; // 7E 03 06 00 EF
int8_t song_array[7][6] = {{0x7e, 0x04, 0x41, 0x00, 0x01, 0xef},{0x7e, 0x04, 0x41, 0x00, 0x02, 0xef},
 {0x7e, 0x04, 0x41, 0x00, 0x03, 0xef}, {0x7e, 0x04, 0x41, 0x00, 0x04, 0xef}, {0x7e, 0x04, 0x41, 0x00, 0x05, 0xef}};//should initialize it in setup

HardwareSerial MP3(2);

/////////////////////////////////BUTTONS!!!!
int stop_ring_button = 0; //didn't use it till now
int reset_button = 0;

//declare buttoun pins like const int buttonPin = 2;// the number of the pushbutton pin
const int ring_buttonPin = 2;     // the number of the pushbutton pin
const int reset_buttonPin = 0;
/////////////////////////////////////////

//FUNCTIONS

/////////////////////////////MP3
void send_command_to_MP3_player(int8_t command[], int len) {
  Serial.print("\nMP3 Command => ");
  for (int i = 0; i < len; i++) {
    MP3.write(command[i]); // sends command to mp3
    Serial.print(command[i], HEX);
  }
  // delay(1000); //what is the delay for??
}




///////////////////////////WIFI
WiFiServer server(80);
bool connection=false;
bool res;
WiFiManager wifiManager;
std::vector<const char *> menu = {"wifi","info"};
////////////////////////////REAL TIME CLOCK
struct tm timeinfo;
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

//const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)
//#define TZ_Asia_Jerusalem  PSTR("IST-2IDT,M3.4.4/26,M10.5.0")
const char* time_zone = "IST-2IDT,M3.4.4/26,M10.5.0";
//////////////////////////////////////STATE MACHINE Definition
enum State {
  STATE_SETTINGS,
  STATE_CLOCK,
  STATE_SLEEP,
  STATE_TIMER,
  STATE_RESET,//need it? yes!
  STATE_ERROR
};

State current_state = STATE_SETTINGS;
////////////////////////////////////FOR THE USER
struct wakeupSleepTimes {
  int wakeup_hour;
  int wakeup_min;
  int sleep_hour;
  int sleep_min;
  int song;
};
wakeupSleepTimes week_array[7];

///////////////////////////////////////////////////////////changed hereeeeeee //////////////////////////////////////////////////////
struct statistics {
  String statistics_day;
  int sleeping_time_in_hours;
  int sleeping_time_in_minutes;
};

statistics weekStatistics[7];

int done_sleep = 0;
int done_timer = 0;
bool setting_timer = false;
bool setting_alarm = false;
bool timing_is_ok = false; // For avoiding problems with zimun tahlich time fetching
int timer = 0;
bool setting_day = false;
bool back_to_menu = false;
bool setting_wakeup_alarm = false;
bool setting_sleep_alarm = false;
bool choosing_tune=false;



int hh = 0; // hh and mm for alarm
int mm = 0;
int day;


void printLocalTime()
{
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  timing_is_ok = true;
  printLocalTime();
}
/////////////////////////////////////TIMER --- changed so it runs by minutes and seconds

void print_countdown(int timer)
{
  Display.print(&timeinfo, "%M:%S");

  struct tm timeinfo;
  timeinfo.tm_min = timer;
  timeinfo.tm_sec = 0;

  Display.print(&timeinfo, "%M:%S");

  while (timeinfo.tm_min != 0 || timeinfo.tm_sec != 0) {
    if (timeinfo.tm_sec == 0) {
      timeinfo.tm_min--;
      timeinfo.tm_sec = 59;
    } else {
      timeinfo.tm_sec--;
    }
    Display.print(&timeinfo, "%M:%S");

    delay(1000);
  }
  //done timer, ring
  send_command_to_MP3_player(play_first_song_cycle, 6); //Temporary
  while (digitalRead(ring_buttonPin) == HIGH) { //dont forget to add or a message from telegram to stop
    //wait for button press
    Serial.println("AM I STUCK? in timer :( ,waiting for button press babyyy");
   // send_command_to_MP3_player(play_first_song, 6);
  }
  send_command_to_MP3_player(pause_song, 4);
  current_state = STATE_CLOCK;
  setting_timer = false;
  timer = 0;
}

bool checkWakeupSleepTime(int today_hour, int today_minutes, int user_hour, int user_minutes)
{
  /*Serial.println("today_hour:");
    Serial.println(today_hour);
    Serial.println("today_minutes:");
    Serial.println(today_minutes);
    Serial.println("user_hour:");
    Serial.println(user_hour);
    Serial.println("user_minutes:");
    Serial.println(user_minutes);
  */


  if ((today_hour == user_hour) && (today_minutes == user_minutes))
    return true;
  return false;
}

///////////////////////////////////////TELEGRAM BOT
#define BOT_TOKEN "5662253371:AAHIiLVljhpesYfw3JHmUb_KKiV4iZTX1ZU"
const unsigned long BOT_MTBS = 1000; // mean time between scan messages
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

//FUNCS
////////for alarm
bool isValidTime(String timeString, int num) {
  // Ensure the string has the correct length
  if (timeString.length() != 5) {
    return false;
  }

  // Parse hours
  hh = (timeString[0] - '0') * 10 + (timeString[1] - '0');

  // Parse minutes
  mm = (timeString[3] - '0') * 10 + (timeString[4] - '0');
  day = num;
  // Check validity
  if (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59 && timeString[2] == ':') {
    Serial.print("recieved time is: ");
    Serial.print(hh);
    Serial.print(":");
    Serial.print(mm);
    // Valid time
    //setting_day = false;
    if (setting_wakeup_alarm) {
      setting_wakeup_alarm = false;
      week_array[day].wakeup_hour = hh;
      week_array[day].wakeup_min = mm;
      Serial.println("got to validate the wakeup alarm" );
    }
    else if (setting_sleep_alarm) {
      setting_sleep_alarm = false;
      week_array[day].sleep_hour = hh;
      week_array[day].sleep_min = mm;

    }
    return true;
  }
  return false; // Invalid time
}
void handleNewMessages(int numNewMessages)
{

  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id; //singular user - not used
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") {
      from_name = "Guest";
    }

    if (text.indexOf("set Timer⏰") != -1)
    {
      setting_timer = true;
      bot.sendMessage(chat_id, "👌", "");
      String keyboardJson = "[[\"🔙\", \"1 minute\", \"5 minutes\"], [\"10 minutes\", \"20 minutes\", \"30 minutes\"], [\"45 minute\", \"50 minutes\", \"60 minutes\"]]";//// the buttons
      bot.sendMessageWithReplyKeyboard(chat_id, "For how long would you like it to be?\n", "", keyboardJson, true);
      return;
    }

    ////////////---------------- timer options--------------////////////////

    if (setting_timer == true) {
      if (text.indexOf("1 minute") != -1) {
        bot.sendMessage(chat_id, "Got it! 1 minute timer starting now⏳. ", "");
        timer = 1;
      }
      if (text.indexOf("5 minutes") != -1) {
        bot.sendMessage(chat_id, "Got it! 5 minutes timer starting now⏳. ", "");
        timer = 5;
      }
      if (text.indexOf("10 minutes") != -1) {
        bot.sendMessage(chat_id, "Got it! 10 minutes timer starting now⏳. ", "");
        timer = 10;
      }
      if (text.indexOf("20 minutes") != -1) {
        bot.sendMessage(chat_id, "Got it! 20 minutes timer starting now⏳. ", "");
        timer = 20;
      }
      if (text.indexOf("30 minutes") != -1) {
        bot.sendMessage(chat_id, "Got it! 30 minutes timer starting now⏳. ", "");
        timer = 30;
      }
      if (text.indexOf("45 minutes") != -1) {
        bot.sendMessage(chat_id, "Got it! 45 minutes timer starting now⏳. ", "");
        timer = 45;
      }
      if (text.indexOf("50 minutes") != -1) {
        bot.sendMessage(chat_id, "Got it! 50 minutes timer starting now⏳. ", "");
        timer = 50;
      }
      if (text.indexOf("60 minutes") != -1) {
        bot.sendMessage(chat_id, "Got it! 60 minutes timer starting now⏳. ", "");
        timer = 60;
      }

      //// go back to menu
      if (text.indexOf("🔙") != -1) {
        back_to_menu = true;
      }
    }
    /////////////-----------------setting alarm mode------------------///////////////
    if (text.indexOf("set Alarm⏱") != -1)
    {
      setting_day = true;

      bot.sendMessage(chat_id, "Hi I am your alarm! we are setting wakeup time now.)", "");
      String keyboardJson = "[[\"SUNDAY\", \"MONDAY\", \"TUESDAY\"], [\"WEDNESDAY\", \"THURSDAY\"], [\"FRIDAY\", \"SATURDAY\", \"🔙\"]]";//// the buttons
      bot.sendMessageWithReplyKeyboard(chat_id, "Ok! please choose the day you want to set this alarm for", "", keyboardJson, true);
      return;
    }
    if (setting_day)
    {
      if (text.indexOf("SUNDAY") != -1) {
        day = 0;
      }
      if (text.indexOf("MONDAY") != -1) {
        day = 1;
      }
      if (text.indexOf("TUESDAY") != -1) {
        day = 2;
      }
      if (text.indexOf("WEDNESDAY") != -1) {
        day = 3;
      }
      if (text.indexOf("THURSDAY") != -1) {
        day = 4;
      }
      if (text.indexOf("FRIDAY") != -1) {
        day = 5;
      }
      if (text.indexOf("SATURDAY") != -1) {
        day = 6;
      }

      if (text.indexOf("🔙") != -1) {
        back_to_menu = true;
      } else {
        bot.sendMessage(chat_id, "Ok! please send me a message with the requested time for the alarm by the hour and minute\nfor example 12:00", "");
        setting_wakeup_alarm = true;
        setting_alarm = true;
        setting_day = false;
        return;
      }
      setting_day = false;


    }
    /////////////// ----------- receiving wakeup alarm and validating---------------//////////////////
    if (setting_wakeup_alarm == true) {
      if (isValidTime(text, day) == false) {
        bot.sendMessage(chat_id, "looks like there was am mistake in the message!⚠ \n please try to send it again correctly", "");
      } else {
        setting_sleep_alarm = true;
        bot.sendMessage(chat_id, "Ok! Setting sleep time😴", "");
        bot.sendMessage(chat_id, " please send me a message with the requested time for the alarm by the hour and minute.\nfor example 12:00.\n make sure that it's not after 00:00 in the same day please🌙.", "");
        return;
      }
    }
    /////////////// ----------- receiving sleep alarm and validating---------------//////////////////
    if (setting_sleep_alarm) {

      if (isValidTime(text, day) == false) {
        bot.sendMessage(chat_id, "looks like there was am mistake in the message!⚠ \n please try to send it again correctly", "");
      } else {
         String keyboardJson = "[[\"Beep Beep!\", \"Soft\", \"Wakeup 🌞\"], [\"Piano 🎹\", \"Morning Birds 🐦 \", \"🔙\"]]";//// the buttons 
        bot.sendMessageWithReplyKeyboard(chat_id, "Great! Both wakeup alarm and sleep alarm are set✅. Now let's choose a ringtone 🎶\n ", "", keyboardJson, true); //user can not choose a ringtone and go stright back
        choosing_tune=true;
      }
    }
    ////////////-------------user chooses ringtone------------//////////
    if (choosing_tune){
      if(text.indexOf("Beep Beep!") != -1){
       send_command_to_MP3_player(song_array[0], 6);
       delay(5000);
       send_command_to_MP3_player(pause_song,4);
       week_array[day].song=0;//meaning song_array[0] 
      }
     if(text.indexOf("Soft") != -1){
      send_command_to_MP3_player(song_array[1], 6);
      delay(5000);
      send_command_to_MP3_player(pause_song,4);
      week_array[day].song=1;//meaning song_array[1] 
     }
     if(text.indexOf("Wakeup 🌞") != -1){
     send_command_to_MP3_player(song_array[2], 6);
     delay(5000);
     send_command_to_MP3_player(pause_song,4);
     week_array[day].song=2;//meaning song_array[2] 

    }
    if(text.indexOf("Piano 🎹") != -1){
      send_command_to_MP3_player(song_array[3], 6);
      delay(5000);
      send_command_to_MP3_player(pause_song,4);
      week_array[day].song=3;//meaning song_array[3] 
    }
    if(text.indexOf("Morning Birds 🐦") != -1){
      send_command_to_MP3_player(song_array[4], 6);
      delay(5000);
      send_command_to_MP3_player(pause_song,4);
      week_array[day].song=4;//meaning song_array[4] 

     }
    if(text.indexOf("🔙") != -1){
      back_to_menu = true;
      choosing_tune=false;
     }
    }
    //////////////-------------------------getting back to menu----------------------------/////////////////
    if ( back_to_menu) {
      String bot_menu = "Can I help with anything else?😬\n";
      String keyboardJson = "[[\"set Alarm⏱\", \"set Timer⏰\", \"Show me statistics📊\"]]";//// the buttons
      bot.sendMessageWithReplyKeyboard(chat_id, bot_menu , "", keyboardJson, true); //reply with option keyboard
      back_to_menu = false;
    }
    //////////////--------------------statistics mode ------------------------------/////////////


      if (text.indexOf("Show me statistics📊") != -1) {
      int totalDuration = 0;

      for (int i = 0; i < 7; i++) {
        int wakeupHour = week_array[i].wakeup_hour;
        int wakeupMin = week_array[i].wakeup_min;
        int sleepHour = week_array[i].sleep_hour;
        int sleepMin = week_array[i].sleep_min;

        int prevDayIndex = (i == 0) ? 6 : (i - 1);  // Index of the previous day

        int prevWakeupHour = week_array[prevDayIndex].wakeup_hour;
        int prevWakeupMin = week_array[prevDayIndex].wakeup_min;
        int prevSleepHour = week_array[prevDayIndex].sleep_hour;
        int prevSleepMin = week_array[prevDayIndex].sleep_min;

        // Convert wakeup time and sleep time to minutes
        int prevWakeupTime = prevWakeupHour * 60 + prevWakeupMin;
        int prevSleepTime = prevSleepHour * 60 + prevSleepMin;
        int wakeupTime = wakeupHour * 60 + wakeupMin;

        // Calculate duration in minutes
        int duration = (wakeupTime >= prevSleepTime) ? (wakeupTime - prevSleepTime) : (1440 - prevSleepTime + wakeupTime);
        totalDuration += duration;

        // Store sleep duration and day in the weekStatistics array
        weekStatistics[i].sleeping_time_in_hours = duration / 60;
        weekStatistics[i].sleeping_time_in_minutes = duration % 60;

        // Print sleep duration for the day

        bot.sendMessage(chat_id, "You slept in " + weekStatistics[i].statistics_day + " for " + String(weekStatistics[i].sleeping_time_in_hours) + ":" + String(weekStatistics[i].sleeping_time_in_minutes) + "\n", "");

      }

      // Calculate average duration for the week
      int averageDuration = totalDuration / 7;
      int averageHours = averageDuration / 60;
      int averageMinutes  = averageDuration % 60;
      if (averageHours >= 7) {
        bot.sendMessage(chat_id, "Your average sleeping time this week is " + String(averageHours) + ":" + String(averageMinutes) + " which is according to the reaserches healthy sleep.", "");
        bot.sendMessage(chat_id, "Keep it healthy 💪", "");
      } else {
        bot.sendMessage(chat_id, "Your average sleeping time this week is " + String(averageHours) + ":" + String(averageMinutes) + " which is according to the reaserches not healthy sleep.😨", "");
        bot.sendMessage(chat_id, "We recommend you to give yourself more time for sleeping and resting during the week. ", "");
        bot.sendMessage(chat_id, "🤞", "");
      }
      back_to_menu = true;
    }


    ///////////////////----------------the start of telegram -------------------/////////////////////
    if (text.indexOf("start") != -1)
    {
      String welcome = "Welcome to the IOT Alarm " + from_name + "☺.\n";
      welcome += "What would you like to do?";
      //bot.sendMessage(chat_id, welcome, "");
      String keyboardJson = "[[\"set Alarm⏱\", \"set Timer⏰\", \"Show me statistics📊\"]]";//// the buttons
      bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true); //reply with option keyboard
    }
  }
}


/////////////////////////////////////////
void setup()
{

  weekStatistics[0].statistics_day = "Sunday";
  weekStatistics[1].statistics_day = "Monday";
  weekStatistics[2].statistics_day = "Tuesday";
  weekStatistics[3].statistics_day = "Wednesday";
  weekStatistics[4].statistics_day = "Thursday";
  weekStatistics[5].statistics_day = "Friday";
  weekStatistics[6].statistics_day = "Saturday";

  for(int i = 0; i < 7; i++){
    weekStatistics[i].sleeping_time_in_minutes = 0;
    weekStatistics[i].sleeping_time_in_hours = 0;
  }

  //Serial.begin(115200); for wifi?
  Serial.begin(9600);
  MP3.begin(9600);
  Display.begin();
  //initialize buttons!!!!! with pinMode ,check info bank for resistors,
  // initialize the pushbutton pin as an input:
  pinMode(ring_buttonPin, INPUT_PULLUP);
  pinMode(reset_buttonPin, INPUT_PULLUP);
  
  //set initial state
  current_state = STATE_SETTINGS;

  ///////////////////////////mp3
  send_command_to_MP3_player(select_SD_card, 5);
  send_command_to_MP3_player(set_volume, 5);

  Display.setIntensity(0);
  Display.displayClear();


  //////////////////////////connect to WIFI
  //bool res;
  //WiFiManager wifiManager;
  wifiManager.resetSettings();//remove any previous wifi settings
 // std::vector<const char *> menu = {"wifi","info"};
  wifiManager.setMenu(menu);
  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  res = wifiManager.autoConnect("IoTAlarmAP");
  if (!res) {
    Serial.println("Failed to connect");
  }
  connection = true;
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  ////////////////////////telegram
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  ////////////////////////////////////////////////////////////////////////////////////////////
  for (int d = 0 ; d < 7; d++)
  {
    week_array[d].wakeup_hour = 0;
    week_array[d].wakeup_min = 0;
    week_array[d].sleep_hour = 0;
    week_array[d].sleep_min = 0;
    week_array[d].song = 0; //meaning song_array[0] ,default song
  }

  // song_array[0]=  {0x7e, 0x04, 0x41, 0x00, 0x01, 0xef};

  // set notification call-back function
  sntp_set_time_sync_notification_cb( timeavailable );

  /**
     NTP server address could be aquired via DHCP,

     NOTE: This call should be made BEFORE esp32 aquires IP address via DHCP,
     otherwise SNTP option 42 would be rejected by default.
     NOTE: configTime() function call if made AFTER DHCP-client run
     will OVERRIDE aquired NTP server address
  */
  sntp_servermode_dhcp(1);    // (optional)

  /**
     This will set configured ntp servers and constant TimeZone/daylightOffset
     should be OK if your time zone does not need to adjust daylightOffset twice a year,
     in such a case time adjustment won't be handled automagicaly.
  */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  /**
     A more convenient approach to handle TimeZones with daylightOffset
     would be to specify a environmnet variable with TimeZone definition including daylight adjustmnet rules.
     A list of rules for your zone could be obtained from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
  */
  //configTzTime(time_zone, ntpServer1, ntpServer2);

  //send_command_to_MP3_player(play_first_song, 6);// does the mp3 work? it Does!
}
const int ledPin=13;
void loop()
{
  //check active internet
  if (WiFi.status() != WL_CONNECTED) {
    connection = false;
  }
  int reset_state=digitalRead(reset_buttonPin);
  //read pushbutton values!!!!
  while(reset_state == LOW){ //dont forget to add or a message from telegram to stop
     //wait for button press
    Serial.println("AM I reset button clicked");
    digitalWrite(ledPin,LOW);
    reset_button=1;
   }
  //handle telegram messages
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
  ///////////////////////////////////////////////////////////////////////////
  ////////////////////////STATE MACHINE
  // perform state transitions
  Serial.println(current_state);
  if (current_state == STATE_SETTINGS)//connection not declared yet, it represents if the user entered legal network&password
  {
    if (!connection)
    {
      current_state = STATE_ERROR;
    }
    if (reset_button == 1) //not declared yet
    {
      current_state = STATE_RESET;
      reset_button = 0; //?
    }
    else current_state = STATE_CLOCK;//display hour, be ready to set alarm,
  }
  if (current_state == STATE_CLOCK )
  {
    //Time to go to bed
    if (!getLocalTime(&timeinfo)) {
      Serial.println("No time available (yet)");
    }
    if (timing_is_ok && checkWakeupSleepTime(timeinfo.tm_hour, timeinfo.tm_min, week_array[timeinfo.tm_wday].sleep_hour, week_array[timeinfo.tm_wday].sleep_min)) {
      current_state = STATE_SLEEP;
      Serial.println("Got in if in line 406");
    }

    if (timer != 0) {
      current_state = STATE_TIMER;
    }//eman working on it
    //Time to go to bed
    if(!getLocalTime(&timeinfo)){
           Serial.println("No time available (yet)");
      }
    if(timing_is_ok && checkWakeupSleepTime(timeinfo.tm_hour,timeinfo.tm_min,week_array[timeinfo.tm_wday].sleep_hour,week_array[timeinfo.tm_wday].sleep_min)){
      current_state = STATE_SLEEP;
      Serial.println("now going to sleep,my sleeping time is");
       Serial.println(week_array[timeinfo.tm_wday].sleep_hour);
       Serial.println(":");
       Serial.println(week_array[timeinfo.tm_wday].sleep_min);
       Serial.println("now the time is");
       Serial.println(&timeinfo,"%A, %B %d %Y %H:%M:%S");
      }
    //if(setting_alarm) {current_state = STATE_SLEEP;
    //Serial.println("Problem is in line 409");} //edit
    if(reset_button==1)
    {
      current_state = STATE_RESET;
      reset_button = 0; //?
    }
    if (!connection)
    {
      current_state = STATE_ERROR;
    }
  }
  if (current_state == STATE_SLEEP)
  {
    if (done_sleep)
    {
      //DONE FROM TELEGRAM
      current_state = STATE_CLOCK;
      done_sleep = 0;
    }
    if (reset_button == 1)
    {
      current_state = STATE_RESET;
      reset_button = 0; //?
    }
    if (!connection)
    {
      current_state = STATE_ERROR;
    }
  }
  if (current_state == STATE_TIMER )
  {
    if (done_timer) //think how to know that done=1
    {
      //DONE FROM TELEGRAM
      current_state = STATE_CLOCK;
      done_timer = 0;
    }
    if (reset_button == 1)
    {
      current_state = STATE_RESET;
      reset_button = 0; //?
    }
    if (!connection)
    {
      current_state = STATE_ERROR;
    }
  }
  if (current_state == STATE_ERROR & connection == true) ////////////////how to know if we shouls get here?sloved.
  {
    //CONNECTED HERE MEANING THE INTERNET IS BACK AGAIN
    current_state = STATE_CLOCK;
  }
  if (current_state == STATE_RESET)
  {
    //done should symbolize done resetting. do we need it?
    current_state = STATE_SETTINGS;
    if (!connection)
    {
      current_state = STATE_ERROR;
    }
  }
  //add restet button to each state and go to settings!!!!!!!!
  ///////////////////// State Machine


  // Perform actions based on the current state
  switch (current_state)
  {
    case STATE_SETTINGS:
      //enter esp router code &ask in while(!connection) about username&password and make connection=1
      //Nour working on it, done on setup :)
      //initialize arrays, specificly week_array
      for (int d = 0 ; d < 7; d++)
      {
        week_array[d].wakeup_hour = 0;
        week_array[d].wakeup_min = 0;
        week_array[d].sleep_hour = 0;
        week_array[d].sleep_min = 0;
        week_array[d].song = 0; //meaning song_array[0] ,default song
      }
      break;
    case STATE_CLOCK:
      //first display time, may be a poroblem where to put the delay
      //delay(60000); //every 60 secs
      //printLocalTime();     // it will take some time to sync time :)
      //Serial.println("Awake");
      Display.setTextAlignment(PA_CENTER);
      Display.print(&timeinfo, "%H:%M");
      //ckeck if to ring, %d is for days, no need done in sleep state since the only case we need to ring
      break;
    case STATE_SLEEP:
      if (!done_sleep)
        Display.displayClear();
      //ring goodnight,play song with mp3
      //printLocalTime(); // //////////////////////////////////////////////////////////////////////////////////////
      Serial.println("Sleeping");
      //if(checkWakeupSleepTime(timeinfo.tm_hour,timeinfo.tm_min,week_array[timeinfo.tm_wday].wakeup_hour,week_array[timeinfo.tm_wday].wakeup_min))
      if (!getLocalTime(&timeinfo)) {
        Serial.println("No time available (yet)");
      }
       if(timing_is_ok & checkWakeupSleepTime(timeinfo.tm_hour,timeinfo.tm_min,week_array[timeinfo.tm_wday].wakeup_hour,week_array[timeinfo.tm_wday].wakeup_min))// This is a temporary edit
      {
        done_sleep = 1;
        setting_alarm = false;
        Serial.println("WAKE UP");
        //play song
        //    bot.sendMessage(chat_id, "Ring Ring!", "");
        // send_command_to_MP3_player(song_array[week_array[timeinfo.tm_wday].song], 6);
        send_command_to_MP3_player(play_first_song, 6); //Temporary
        while (digitalRead(ring_buttonPin) == HIGH) { //dont forget to add or a message from telegram to stop
          //wait for button press
          Serial.println("AM I STUCK? :( ,waiting for button press babyyy");
        }
        send_command_to_MP3_player(pause_song, 4);
        current_state = STATE_CLOCK;
      }//should timer ring
      break;
    case STATE_TIMER:
      //parse from telegram what timer count
      Display.setTextAlignment(PA_CENTER);
      print_countdown(timer);//timer from telegram
      timer = 0;
      done_timer = 1;

      break;
    case STATE_RESET:
      //print reset,check
      Display.setTextAlignment(PA_LEFT);
      Display.displayClear();
      Display.displayText("RST", PA_CENTER, 0, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);//check if working good
     // bool res;
     // WiFiManager wifiManager;
      wifiManager.resetSettings();//remove any previous wifi settings
      //std::vector<const char *> menu = {"wifi","info"};
      wifiManager.setMenu(menu);
      // fetches ssid and pass from eeprom and tries to connect
      // if it does not connect it starts an access point with the specified name
     res =wifiManager.autoConnect("IoTAlarmAP");
       if(!res){
       Serial.println("Failed to connect");
      }
      connection = true;
      Serial.println("WiFi connected again");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      break;
    case STATE_ERROR:
      //reinitiLIZE wifi and reupload user info
      //we get there if no internet connection, connenction=false
      //from setup
      //connect to WIFI
      // bool res;
      // WiFiManager wifiManager;
      //setSettings();//remove any previous wifi settings
      //std::vector<const char *> menu = {"wifi","info"};
      //wifiManager.setMenu(menu);
      // fetches ssid and pass from eeprom and tries to connect
      // if it does not connect it starts an access point with the specified name
      res =wifiManager.autoConnect("IoTAlarmAP");
      if(!res){
        Serial.println("Failed to connect");
      }
      connection = true;
      Serial.println("WiFi connected again");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      break;
  }
}