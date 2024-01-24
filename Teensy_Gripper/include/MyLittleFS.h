#ifndef __MYLITTLEFS_H__
#define __MYLITTLEFS_H__

#include <LittleFS.h>
//extern PACKET dataToSend;


class MyLittleFS
{
private:
    /* data */
    //const int PROG_FLASH_SIZE = 1024 * 900;
public:
    MyLittleFS(/* args */);
    ~MyLittleFS();

    LittleFS_Program myfs;

    File dataFile;
    int record_count = 0;
    const char* datalogFile = "datalog.txt";
    
    
    void initLittleFS();
    // void eraseFiles();
    // void listFiles();
    // void printDirectory(FS &fs);
    // void printDirectory(File dir, int numSpaces);
    // void printSpaces(int num);
    // void dumpLog(const char* _file);
    // void writeServoLog();
    // void loadLogData();
};

MyLittleFS::MyLittleFS(/* args */)
{
    
}

MyLittleFS::~MyLittleFS()
{
}


#endif
