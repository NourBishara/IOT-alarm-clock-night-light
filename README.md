# IOT-alarm-clock: 
IoT Alarm Clock by: Nour Bishara, Eman Armush and Shaima Abu Elhija.

In this project, we built a traditional digital alarm clock, but with a modern twist!
The user can interact with the alarm clock and customize it using Telegram. 
It lets you choose your bedtime and wake-up time for each day of the week, and activates sleep mode during your bedtime hours. 
The clock can show you the statisctics of your sleep scheduele of the past week. 
In addition, the clock doubles as a timer, you can set it for one of the optional times, and the countdown will appear on the screen.

# Our Project In Details:
Firstly, the user will connect the alarm clock to the internet. Afterwards, the user can start chatting with the clock using telegram. 
 1. Setting the alarm: The user can set an alarm for each day of the week. From a menu, they choose the day, then they choose at what time they wish to go to bed, and when the alarm should ring. They may also choose a ringtone from the menu that will be provided to them. Everynight, the clock will enter sleep mode, and turn off its display at bedtime, and ring at the requested time in the morning.The user needs to press the button on top of the clock to stop it from ringing.
 2. Statistics: The user can request their weekly bedtime statistics via telegram. They will be provided with the amount of hourse they slept each night, as well as their weekly average.
 3. Timer: The user can set a timer via telegram. The countdown (minutes, seconds) will appear on the display of the clock which will ring once it reaches 00:00. The user needs to press the button on top of the clock to stop it from ringing.

# Folder description:
* SP32: source code for the esp side (firmware).
* Documentation: wiring diagram + basic operating instructions
* Unit Tests: tests for individual hardware components (input / output devices)
* Parameters: contains description of configurable parameters
* Assets: 3D printed parts, Audio files used in this project

# Arduino/ESP32 libraries used in this project:
* HardwareSerial
* WiFiManager
* MD_Parola
* MD_MAX72xx
* WiFiClientSecure
* UniversalTelegramBot
* SPI

# Project Poster:![IOT Alarm Clock-8- IOT poster](https://github.com/NourBishara/IOT-alarm-clock-night-light/assets/128970723/13d7a6e0-67bd-48c6-9244-bc1260697224)





This project is part of ICST - The Interdisciplinary Center for Smart Technologies, Taub Faculty of Computer Science, Technion https://icst.cs.technion.ac.il/
