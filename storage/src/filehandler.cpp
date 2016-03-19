#include <sstream>
#include <stdexcept>
#include "serializers.h"
#include "utils.h"
#include "filehandler.h"
using namespace std;


int itemMatch(const string& filePath, string& dataType, map<string, string> matchArgs){
  // Make sure that the type of stored data in the application is valid
  if(!configParams.count("FILE_COUNT_" + dataType)){
    throw std::runtime_error("Given dataType is unknown");
  }

  // How many characters each item takes up (will read one chunk at a time)
  int fileSize = getFileSize(filePath);

  // Start from the bottom of the file
  int itemSize = configParams["SERIAL_SIZE_" + dataType];
  int readPtr = itemSize;

  FILE* matchedFile;
  matchedFile = fopen(filePath.c_str(), "r+");

  char item[itemSize];

  while(readPtr <= fileSize){
    fseek(matchedFile, -readPtr, SEEK_END);
    fread(item, itemSize, 1, matchedFile);
    item[itemSize] = '\0'; // Cap the garbage (without this, garbage is appended)

    // Compare each item to its corresponding serialized version using the
    // criteria listed by matchArgs
    if(matchesSerialized(string(item), dataType, matchArgs)){
      // Return the first match that is found
      fclose(matchedFile);
      return readPtr;
    }

    readPtr += itemSize;
  }

  fclose(matchedFile);
  return -1;
}

vector<string> itemMatchSweep(const string& filePath, string& dataType, map<string, string> matchArgs, int limit){
  // Make sure that the type of stored data in the application is valid
  if(!configParams.count("FILE_COUNT_" + dataType)){
    throw std::runtime_error("Given dataType is unknown");
  }

  // How many characters each item takes up (will read one chunk at a time)
  int fileSize = getFileSize(filePath);

  // Start from the bottom of the file
  int itemSize = configParams["SERIAL_SIZE_" + dataType];
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

    // Compare each item to its corresponding serialized version using the
    // criteria listed by matchArgs
    if(matchesSerialized(string(item), dataType, matchArgs)){
      // Append to the vector of known matches. Return if we have enough.
      // Interpret limit == -1 to mean "take all".
      allMatches.push_back(item);

      if(limit != -1 && (int) allMatches.size() >= limit){
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
  // Make sure that the type of stored data in the application is valid
  if(!configParams.count("FILE_COUNT_" + dataType)){
    throw std::runtime_error("Given dataType is unknown");
  }

  int fileSize = getFileSize(filePath);

  // How many characters each item takes up (will read one chunk at a time)
  int itemSize = configParams["SERIAL_SIZE_" + dataType];
  int readPtr = itemSize;

  FILE* matchedFile;
  matchedFile = fopen(filePath.c_str(), "r+");

  // There's supposed to be a '1' if the item is active and a '0' if it's not
  char activeByte = active ? '1' : '0';
  unsigned int numModified = 0;
  char item[itemSize];

  while(readPtr <= fileSize){
    fseek(matchedFile, -readPtr, SEEK_END);
    fread(item, itemSize, 1, matchedFile);
    item[itemSize] = '\0'; // Cap the garbage (without this, garbage is appended)

    // Compare each item to its corresponding serialized version using the
    // criteria listed by matchArgs
    if(matchesSerialized(string(item), dataType, matchArgs)){
      // For each successful match, step back, modify the active bit to be
      // what was specified, and keep going.
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
