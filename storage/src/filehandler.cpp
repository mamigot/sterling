#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "filehandler.h"
#include "serializers.h"
using namespace std;


bool isValidPath(const char* path){
  struct stat info;

  if(stat(path, &info) != 0){
    return false;
  }
  return true;
}

bool isValidPath(const string& path){
  return isValidPath(path.c_str());
}

unsigned int getFileSize(const string& path){
  if(!isValidPath(path)){
    throw std::runtime_error("Given path is invalid.");
  }

  ifstream in(path, std::ifstream::ate | std::ifstream::binary);
  return in.tellg();
}

int itemMatch(const string& storedFilePath, string& dataType, map<string, string> matchArgs){
  if(!configParams.count("FILE_COUNT_" + dataType)){
    throw std::runtime_error("Given dataType is unknown");
  }

  unsigned int fileSize = getFileSize(storedFilePath);
  unsigned int itemSize = configParams["SERIAL_SIZE_" + dataType];
  int readPtr = itemSize;

  FILE* matchedFile;
  matchedFile = fopen(storedFilePath.c_str(), "r+");

  char item[itemSize];

  while(readPtr <= fileSize){
    fseek(matchedFile, -readPtr, SEEK_END);
    fread(item, itemSize, 1, matchedFile);
    item[itemSize] = '\0'; // Cap the garbage (without this, garbage is appended)

    if(matchesSerialized(string(item), dataType, matchArgs)){
      fclose(matchedFile);
      return readPtr;
    }

    readPtr += itemSize;
  }

  fclose(matchedFile);
  return -1;
}


int main(){
  configServer();

}
