#ifndef __MYLITTLEFS_H__
#define __MYLITTLEFS_H__

#include <LittleFS.h>
#include "config.h"


class MyLittleFS
{
private:
    LittleFS_Program myfs;
    //const int PROG_FLASH_SIZE = 1024 * 900;
public:
    MyLittleFS(/* args */);
    ~MyLittleFS();

    

    File dataFile;
    int record_count = 0;
    const char* datalogFile = "datalog.txt";
    
    void initLittleFS();
    void eraseFiles();
    void listFiles();
    void printDirectory(FS &fs);
    void printDirectory(File dir, int numSpaces);
    void printSpaces(int num);
    void dumpLog(const char* _file);
    void writeServoLog();
    void loadLogData();
};

MyLittleFS::MyLittleFS(/* args */)
{
    
}

MyLittleFS::~MyLittleFS()
{
}


#endif
