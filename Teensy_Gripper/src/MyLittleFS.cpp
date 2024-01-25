#include "MyLittleFS.h"
#include "MyServo.h"
#include "arduino_freertos.h"

extern PACKET dataToSend;
extern MyServo* myServo;


MyLittleFS::MyLittleFS(/* args */)
{
    
}

MyLittleFS::~MyLittleFS()
{
}


void MyLittleFS::initLittleFS()
{
  if (!myfs.begin(PROG_FLASH_SIZE)) {
    Serial.printf("Error starting %s\n", "PROGRAM FLASH DISK");
    while (1) {
      Serial.printf("Failed to Mount \r\n");
      delay(500);
    }
  }
  Serial.println("LittleFS initialized.");
  listFiles();

  loadLogData();
}


void MyLittleFS::eraseFiles()
{
  myfs.quickFormat();  // performs a quick format of the created di
  Serial.println("\nFiles erased !");
}

void MyLittleFS::listFiles()
{
  Serial.print("\nSpace Used = ");
  Serial.println(myfs.usedSize());
  Serial.print("Filesystem Size = ");
  Serial.println(myfs.totalSize());

  printDirectory(myfs);
}

void MyLittleFS::printDirectory(FS &fs) {
  Serial.println("Directory\n---------");
  printDirectory(fs.open("/"), 0);
  Serial.println();
}

void MyLittleFS::printDirectory(File dir, int numSpaces) {
   while(true) {
     File entry = dir.openNextFile(); // 다음 파일 열기
     if (! entry) {
       //Serial.println("** no more files **");
       break;   // 더 이상 파일이 없으면 나오기
     }
     printSpaces(numSpaces);
     Serial.print(entry.name());  // 파일 이름
     if (entry.isDirectory()) {   // 디렉토리이면 / 추가
       Serial.println("/");
       printDirectory(entry, numSpaces+2);
     } else {
       // files have sizes, directories do not
       printSpaces(36 - numSpaces - strlen(entry.name()));  
       Serial.print("  ");
       Serial.println(entry.size(), arduino::DEC);   // 파일 크기
     }
     entry.close(); // 파일 종료
   }
}

void MyLittleFS::printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}

void MyLittleFS::dumpLog(const char* _file)
{
  Serial.println("\nDumping Log!!!");
  // open the file.
  dataFile = myfs.open(_file);

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
      
    }
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
}

void MyLittleFS::writeServoLog() 
{

  myfs.remove(datalogFile);

  dataFile = myfs.open(datalogFile, FILE_WRITE);
  
  String dataString = "";
  dataString = "Gripper:" + String(myServo->gripperPos) + "," + String(myServo->lockerPos);

  if(dataFile)
  {
    Serial.printf("Write String -> %s\r\n", dataString.c_str());
    dataFile.println(dataString);
  }
  else
  {
    Serial.println("error opening datalog.txt");
  }

  dataFile.close();
  delay(50);

}

void MyLittleFS::loadLogData()
{
  dataFile = myfs.open(datalogFile);

  String readData;

  if(dataFile)
  {
    while(dataFile.available())
    {
      readData = dataFile.readStringUntil('\n');
      if(readData.startsWith("Gripper:"))
      {
        Serial.print(readData);
        int valueIndex = readData.indexOf(':');
        int lockerIndex = readData.indexOf(',');
        myServo->gripperPos = readData.substring(valueIndex + 1, lockerIndex).toInt();
        myServo->lockerPos = readData.substring(lockerIndex + 1, readData.length()).toInt();
        if(myServo->gripperPos == SERVO_TARGET_POS)
        {
          dataToSend.servoState = SERVO_CLOSED;
        }
        else if (myServo->gripperPos == SERVO_INITIAL_POS)
        {
          dataToSend.servoState = SERVO_OPENED;
        }

        if(myServo->lockerPos == SERVO2_TARGET_POS)
        {
          dataToSend.lockerState = SERVO_PUSH;
        }
        else
        {
          dataToSend.lockerState = SERVO_RELEASE;
        }
        

        Serial.printf("SERVO1 : 0x%0x | SERVO2 : 0x%0x", dataToSend.servoState, dataToSend.lockerState);

      }
    }
    dataFile.close();
  }
  else
  {
    Serial.println("error opening datalog.txt");
  }
}