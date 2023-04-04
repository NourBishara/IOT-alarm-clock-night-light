//Nours version

#include <HardwareSerial.h>
// Connections
// ESP32:           
//                                Serial MP3 Player Module (OPEN-SMART)
// GPIO17 ------------------------ TX
// GPIO16 ------------------------ RX
//int8_t variable will use 8 bits, its values from (-2**7) to (2**7 - 1), or -128 to 127

static int8_t select_SD_card[] = {0x7e, 0x03, 0X35, 0x01, 0xef}; // 7E 03 35 01 EF
static int8_t play_first_song[] = {0x7e, 0x04, 0x41, 0x00, 0x01, 0xef};// 7E 04 42 01 01 EF - plays first song
static int8_t play_second_song[]= {0x7e, 0x04, 0x41, 0x00, 0x02, 0xef}; // play second song
static int8_t play_song[] = {0x7e, 0x02, 0x01, 0xef}; //7E 02 01 EF - play / resume
static int8_t pause_song[] = {0x7e, 0x02, 0x02, 0xef}; //7E 02 02 EF - pause
static int8_t set_volume[] = {0x7e, 0x03, 0x31, 0x04, 0xef}; // 7E 03 06 00 EF
//what are these for???? 
//bool first; 
//int unit = 1;
//int goal = 1;
HardwareSerial MP3(2);
void setup(){
  Serial.begin(9600);
  MP3.begin(9600);
  send_command_to_MP3_player(select_SD_card, 5);  
  send_command_to_MP3_player(set_volume, 5);
  //send_command_to_MP3_player(select_SD_caplay_indx_song, 6);
}

void loop() {
	// Play the second song.
	send_command_to_MP3_player(play_first_song, 6);
  delay(10000);
  send_command_to_MP3_player(pause_song,4);
  delay(5000);
 send_command_to_MP3_player(play_second_song, 6);
  delay(10000);
//  send_command_to_MP3_player(pause_song,4);

}

void send_command_to_MP3_player(int8_t command[], int len){
  Serial.print("\nMP3 Command => ");
  for(int i=0;i<len;i++){ 
    MP3.write(command[i]); // sends command to mp3 
    Serial.print(command[i], HEX); 
    }
  delay(1000); //what is the delay for??
}