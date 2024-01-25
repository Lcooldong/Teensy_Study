#include <Arduino.h>
#include <SPI.h>
#include <LittleFS.h>


#define PROG_FLASH_SIZE 1024 * 900
#define BUILTIN_LED 13
uint64_t count = 0;

File dataFile;  // Specifes that dataFile is of File type

int record_count = 0;
bool write_data = false;
uint32_t diskSize;

LittleFS_Program myfs;

void menu();
void logData();
void stopLogging();
void dumpLog();
void listFiles();
void eraseFiles();
void printDirectory(FS &fs);
void printDirectory(File dir, int numSpaces);
void printSpaces(int num);

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    // wait for serial port to connect.
  }
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);

  Serial.println("Initializing LittleFS ...");

  pinMode(BUILTIN_LED, OUTPUT);
  

  if (!myfs.begin(PROG_FLASH_SIZE)) {
    Serial.printf("Error starting %s\n", "PROGRAM FLASH DISK");
    while (1) {
      Serial.printf("Not Working \r\n");
      delay(500);
    }
  }
  Serial.println("LittleFS initialized.");
  digitalWrite(BUILTIN_LED, HIGH);

  menu();
}

void loop() {
  // Serial.printf("Count : %lu\r\n", count++);
  // delay(1000);

  if ( Serial.available() ) {
    char rr;
    rr = Serial.read();
    //Serial.printf("First Read %c\r\n", rr);
    switch (rr) {
      case 'l': listFiles(); break;
      case 'e': eraseFiles(); break;
      case 's':
        {
          Serial.println("\nLogging Data!!!");
          write_data = true;   // sets flag to continue to write data until new command is received
          // opens a file or creates a file if not present,  FILE_WRITE will append data to
          // to the file created.
          dataFile = myfs.open("datalog.txt", FILE_WRITE);
          logData();
        }
        break;
      case 'x': stopLogging(); break;
      case 'd': dumpLog(); break;
      case '\r':  // Enter 부분 읽으면 아래 menu 실행됨
      case '\n':
      case 'h': menu(); break;
    }
    while (Serial.read() != -1);

    // while (Serial.read() != -1)
    // {
    //   delay(100);
    //   Serial.println("In While");
    // } ;  
    // 다음 1바이트를 읽는데(읽으면 처리될때까지 대기 - 여러번 읽히는 거 방지용) 아무것도 안 읽으면 -1 반환

    //Serial.println("End of Serial");
  } 

  if(write_data) logData(); // Serial Monitor가 끝나더라도 s를 누른 후라면 계속 동작
}

void logData()
{
    // make a string for assembling the data to log:
    String dataString = "";
  
    // read three sensors and append to the string:
    for (int analogPin = 0; analogPin < 3; analogPin++) {
      int sensor = analogRead(analogPin);
      dataString += String(sensor);
      if (analogPin < 2) {
        dataString += ",";  // 마지막만(2) ',' 빼기
      }
    }
  
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      // print to the serial port too:
      Serial.println(dataString);
      record_count += 1;
    } else {
      // if the file isn't open, pop up an error:
      Serial.println("error opening datalog.txt");
    }
    delay(100); // run at a reasonable not-too-fast speed for testing
}

void stopLogging()
{
  Serial.println("\nStopped Logging Data!!!");
  write_data = false;
  // Closes the data file.
  dataFile.close();
  Serial.printf("Records written = %d\n", record_count);
}


void dumpLog()
{
  Serial.println("\nDumping Log!!!");
  // open the file.
  dataFile = myfs.open("datalog.txt");

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

void menu()
{
  Serial.println();
  Serial.println("Menu Options:");
  Serial.println("\tl - List files on disk");
  Serial.println("\te - Erase files on disk");
  Serial.println("\ts - Start Logging data (Restarting logger will append records to existing log)");
  Serial.println("\tx - Stop Logging data");
  Serial.println("\td - Dump Log");
  Serial.println("\th - Menu");
  Serial.println();
}

void listFiles()
{
  Serial.print("\nSpace Used = ");
  Serial.println(myfs.usedSize());
  Serial.print("Filesystem Size = ");
  Serial.println(myfs.totalSize());

  printDirectory(myfs);
}

void eraseFiles()
{
  myfs.quickFormat();  // performs a quick format of the created di
  Serial.println("\nFiles erased !");
}

void printDirectory(FS &fs) {
  Serial.println("Directory\n---------");
  printDirectory(fs.open("/"), 0);
  Serial.println();
}

void printDirectory(File dir, int numSpaces) {
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
       Serial.println(entry.size(), DEC);   // 파일 크기
     }
     entry.close(); // 파일 종료
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}