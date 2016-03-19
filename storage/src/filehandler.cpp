#include <sstream>
#include "serializers.h"
#include "utils.h"
#include "filehandler.h"
using namespace std;


int itemMatch(const string& filePath, string& dataType, map<string, string> matchArgs){
  if(!configParams.count("FILE_COUNT_" + dataType)){
    throw std::runtime_error("Given dataType is unknown");
  }

  unsigned int fileSize = getFileSize(filePath);
  unsigned int itemSize = configParams["SERIAL_SIZE_" + dataType];
  int readPtr = itemSize;

  FILE* matchedFile;
  matchedFile = fopen(filePath.c_str(), "r+");

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

vector<string> itemMatchSweep(const string& filePath, string& dataType, map<string, string> matchArgs, int limit){
  if(!configParams.count("FILE_COUNT_" + dataType)){
    throw std::runtime_error("Given dataType is unknown");
  }

  unsigned int fileSize = getFileSize(filePath);
  unsigned int itemSize = configParams["SERIAL_SIZE_" + dataType];
  int readPtr = itemSize;

  FILE* matchedFile;
  matchedFile = fopen(filePath.c_str(), "r+");

  char item[itemSize];
  vector<string> allMatches;
  if(!limit) return allMatches;

  while(readPtr <= fileSize){
    fseek(matchedFile, -readPtr, SEEK_END);
    fread(item, itemSize, 1, matchedFile);
    item[itemSize] = '\0'; // Cap the garbage (without this, garbage is appended)

    if(matchesSerialized(string(item), dataType, matchArgs)){
      allMatches.push_back(item);

      if(limit != -1 && allMatches.size() >= limit){
        break;
      }
    }

    readPtr += itemSize;
  }

  fclose(matchedFile);
  //return allMatches.str();
  return allMatches;
}

int setActiveFlag(bool active, const string& filePath, string& dataType, map<string, string> matchArgs){
  if(!configParams.count("FILE_COUNT_" + dataType)){
    throw std::runtime_error("Given dataType is unknown");
  }

  unsigned int fileSize = getFileSize(filePath);
  unsigned int itemSize = configParams["SERIAL_SIZE_" + dataType];
  int readPtr = itemSize;

  FILE* matchedFile;
  matchedFile = fopen(filePath.c_str(), "r+");

  char activeByte = active ? '1' : '0';
  unsigned int numModified = 0;
  char item[itemSize];

  while(readPtr <= fileSize){
    fseek(matchedFile, -readPtr, SEEK_END);
    fread(item, itemSize, 1, matchedFile);
    item[itemSize] = '\0'; // Cap the garbage (without this, garbage is appended)

    if(matchesSerialized(string(item), dataType, matchArgs)){
      fseek(matchedFile, -readPtr, SEEK_END);
      fwrite(&activeByte, 1, 1, matchedFile);
      fseek(matchedFile, -readPtr, SEEK_END);
      numModified++;
    }

    readPtr += itemSize;
  }

  fclose(matchedFile);
  return numModified;
}
