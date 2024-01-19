#include <Arduino.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

#include <HardwareSerial.h>
//#include <PWMServo.h>

#include <EEPROM.h>


// #include "FS.h"
#include <LittleFS.h>
#include "neopixel.h"


const int serialInterval = 1000;
uint64_t systemCount = 0;

// Flash not work yet 2024/01/18
LittleFS_Program myfs;

#define PROG_FLASH_SIZE 1024 * 900 // Specify size to use of onboard Teensy Program Flash chip
                                        // This creates a LittleFS drive in Teensy PCB FLash. 

File dataFile;  // Specifes that dataFile is of File type

int record_count = 0;
bool write_data = false;
uint32_t diskSize;


void logData();
void stopLogging();
void dumpLog();
void menu();
void listFiles();
void eraseFiles();
void printDirectory(FS &fs);
void printDirectory(File dir, int numSpaces);
void printSpaces(int num);

static void blink(void*) {
    while (true) {
        ::digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
        ::vTaskDelay(pdMS_TO_TICKS(500));

        ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
        ::vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void tickTock(void*) {
    while (true) {
        ::Serial.printf("Flow : %lu\r\n", systemCount++);
        ::vTaskDelay(pdMS_TO_TICKS(serialInterval));
    }
}


static void uartTask(void*)
{
    while (true)
    {
        // char text = Serial.read();
        
        // switch (text)
        // {
        // case '1':
        //     for (int i = 0; i < 255; i++)
        //     {
        //         int value = EEPROM.read(i);
        //         Serial.printf("%d : %d \r\n", i, value);
        //     }
            
        //     break;
        
        // default:
        //     break;
        // }

        char rr;
        rr = Serial.read();
        switch (rr) 
        {
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
            case '\r':
            case '\n':
            case 'h': menu(); break;
        }
        while (Serial.read() != -1) ; // remove rest of characters. 

        if(write_data) logData();
    }     
}

void setup() {
  ::Serial.begin(115200);
  while (!Serial) {
    // wait for serial port to connect.
  }
  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);

  Serial.println("Initializing LittleFS ...");

  // see if the Flash is present and can be initialized:
  // lets check to see if the T4 is setup for security first
  #if ARDUINO_TEENSY40
    if ((IOMUXC_GPR_GPR11 & 0x100) == 0x100) {
      //if security is active max disk size is 960x1024
      if(PROG_FLASH_SIZE > 960*1024){
        diskSize = 960*1024;
        Serial.printf("Security Enables defaulted to %u bytes\n", diskSize);  
      } else {
        diskSize = PROG_FLASH_SIZE;
        Serial.printf("Security Not Enabled using %u bytes\n", diskSize);
      }
    }
  #else
    diskSize = PROG_FLASH_SIZE;
  #endif

  // checks that the LittFS program has started with the disk size specified
  if (!myfs.begin(diskSize)) {
    Serial.printf("Error starting %s\n", "PROGRAM FLASH DISK");
    while (1) {
      // Error, so don't do anything more - stay stuck here
      Serial.println("Stuck in here");
      delay(100);
    }
  }
  Serial.println("LittleFS initialized.");
  
  menu();
//   for (int i = 0; i < 255; i++)
//   {
//     EEPROM.write(i, i);
//   }

//   for (int i = 0; i < 255; i++)
//   {
//     EEPROM.update(i, i+1);
//   }
  
  


  ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
  if (CrashReport) 
  {
      ::Serial.print(CrashReport);
      ::Serial.println();
      ::Serial.flush();
  }

  ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

  
  ::xTaskCreate(blink, "blink", 128, nullptr, 1, nullptr);
  ::xTaskCreate(tickTock, "tickTock", 2048, nullptr, 1, nullptr);
  ::xTaskCreate(uartTask, "uartTask", 8196, nullptr, 1, nullptr);
  ::Serial.println("setup(): starting scheduler...");
  ::Serial.println("========Start Teensy Gripper========");
  ::Serial.flush(); // 단점 : UART 느림
  ::vTaskStartScheduler();
}

void loop() {}

void logData()
{
    // make a string for assembling the data to log:
    String dataString = "";
  
    // read three sensors and append to the string:
    for (int analogPin = 0; analogPin < 3; analogPin++) {
      int sensor = analogRead(analogPin);
      dataString += String(sensor);
      if (analogPin < 2) {
        dataString += ",";
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
  Serial.print("\n Space Used = ");
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
     File entry = dir.openNextFile();
     if (! entry) {
       //Serial.println("** no more files **");
       break;
     }
     printSpaces(numSpaces);
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numSpaces+2);
     } else {
       // files have sizes, directories do not
       printSpaces(36 - numSpaces - strlen(entry.name()));
       Serial.print("  ");
       Serial.println(entry.size(), arduino::DEC);
     }
     entry.close();
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}