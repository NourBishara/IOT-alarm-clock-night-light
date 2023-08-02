
#include <FS.h>
#include <SPIFFS.h>

typedef struct {
  char someString[10];
  float someFloat;
} MyStruct;

MyStruct myStruct = { "test test",1.7 };

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  if(!SPIFFS.begin()){
      Serial.println("Card Mount Failed");
      return;
  }

  strncpy( myStruct.someString, "testtesttest", sizeof(myStruct.someString) );
  File myFile = SPIFFS.open("/somefile.txt", FILE_WRITE);
  myFile.write((byte *)&myStruct, sizeof(myStruct));
  myFile.close();
}

void loop() {
  // put your main code here, to run repeatedly:
  File myFile = SPIFFS.open("/somefile.txt", FILE_WRITE);
  myFile.read((byte *)&myStruct, sizeof(myStruct));
  myFile.close();
  Serial.printf( "read: %s, %.2f\n", myStruct.someString, myStruct.someFloat );
  delay(1000);
}
