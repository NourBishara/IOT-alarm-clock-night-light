 Hardcoded Parameters: TelegramBot token
 The parameters are at the top of the ESP32 code:

 // Defining size, and output pins,MATRIX
 MAX_DEVICES 4
 CS_PIN 15
 SCK_PIN 14
 MOSI_PIN 13

//Defining commands to send to mp3 player
  select_SD_card[]  // 7E 03 35 01 EF
  play_first_song[]// 7E 04 42 01 01 EF - plays first song
  play_first_song_cycle[] //7E 04 33 00 01 EF
  play_second_song[]  // play second song
  play_third_song[]  // 7E 04 41 00 03 EF
  play_fourth_song[] // 7E 04 41 00 04 EF
  play_fifth_song[]  // 7E 04 41 00 05 EF
  play_song[] //7E 02 01 EF - play / resume
  pause_song[]  //7E 02 02 EF - pause
  set_volume[]  // 7E 03 06 00 EF
  song_array[7][6]  // an array that holds the commands for playing each song of the week
 
  
  // defining the physical buttons
  ring_buttonPin = 2;     // the number of the pushbutton pin
  reset_buttonPin = 0;
// initial values (high/ low)
 stop_ring_button = 0; 
 reset_button = 0;

// array to save customized sleep scheduele
truct wakeupSleepTimes {
  int wakeup_hour;
  int wakeup_min;
  int sleep_hour;
  int sleep_min;
  int song;
};
wakeupSleepTimes week_array[7];

// Array to track weekly sleep scheduele
struct statistics {
  String statistics_day;
  int sleeping_time_in_hours;
  int sleeping_time_in_minutes;
};
statistics weekStatistics[7];
