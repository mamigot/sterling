#include <iostream> // debuggin'
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <mutex>
#include "config.h"
#include "serializers.h"
#include "utils.h"
#include "filehandler.h"
using namespace std;


// Map file paths to their respective files' mutexes
map<string, shared_ptr<mutex>> fileMutexes = {};
mutex fileMutexesAccess;

// Get a pointer to a mutex corresponding to a given file path
// Ex.:
//  auto mut = getDataFileMutex(someFilePath);
//  unique_lock<mutex> lck(*mut); // how to use it
shared_ptr<mutex> getDataFileMutex(const string& filePath){
  if(!fileMutexes.count(filePath)){
    unique_lock<mutex> lock(fileMutexesAccess);

    shared_ptr<mutex> mut(new mutex());
    fileMutexes.insert(std::make_pair(filePath, mut));
  }

  return fileMutexes.at(filePath);
}

class LReader {
public:
  LReader(const string& filePath, const string& dataType) : filePath(filePath){
    // Make sure that the type of stored data in the application is valid
    if(configParams.at("FILE_COUNT_" + dataType) == -1)
      throw std::runtime_error("Given dataType is unknown");

    // Size of each item in the file (allows to analyze one-by-one)
    itemSize = configParams.at("SERIAL_SIZE_" + dataType);

    // Start from the bottom of the file
    offsetFromEnd = 0;

    matchedFile = fopen(filePath.c_str(), "rb");
    fileSize = getFileSize(filePath);
  }

  ~LReader() { fclose(matchedFile); }

  bool hasNext() { return offsetFromEnd < fileSize; }

  string next() {
    if(hasNext()){
      offsetFromEnd += itemSize;
      return readItem(offsetFromEnd);

    }else{
      return string();
    }
  }

  bool hasPrev() { return offsetFromEnd >= itemSize; }

  string prev() {
    if(hasPrev()){
      offsetFromEnd -= itemSize;
      return readItem(offsetFromEnd);

    }else{
      return string();
    }
  }

  int getItemSize() { return itemSize; }

  int getReadPtr() { return offsetFromEnd; }

  FILE* getFilePtr() { return matchedFile; }

private:
  const string& filePath;
  FILE* matchedFile;
  int fileSize;
  int itemSize;
  int offsetFromEnd;

  string readItem(int offsetFromEnd) {
    char buff[itemSize];

    fseek(matchedFile, -offsetFromEnd, SEEK_END);
    fread(buff, itemSize, 1, matchedFile);

    buff[itemSize] = '\0'; // Cap the garbage (without this, it is appended)
    return string(buff);
  }
};

void appendToDataFile(const string& filePath, const string& content){
  ofstream outfile;

  // This entails access of a shared resource, so use a lock.
  auto mut = getDataFileMutex(filePath);
  unique_lock<mutex> lck(*mut);

  outfile.open(filePath, ios_base::app);
  outfile << content;
}

int itemMatch(const string& filePath, string& dataType, const map<string, string> matchArgs){
  auto mut = getDataFileMutex(filePath);
  unique_lock<mutex> lck(*mut);

  LReader reader = LReader(filePath, dataType);

  while(reader.hasNext()){
    auto item = reader.next();

    if(matchesSerialized(item, dataType, matchArgs)){
      return reader.getReadPtr();
    }
  }

  return -1;
}

vector<string> itemMatchSweep(const string& filePath, string& dataType, map<string, string> matchArgs, int limit){
  vector<string> allMatches;
  if(!limit) return allMatches;

  auto mut = getDataFileMutex(filePath);
  unique_lock<mutex> lck(*mut);

  LReader reader = LReader(filePath, dataType);

  while(reader.hasNext()){
    auto item = reader.next();

    // Compare each item to its corresponding serialized version using the
    // criteria listed by matchArgs
    if(matchesSerialized(item, dataType, matchArgs)){
      // Append to the vector of known matches. Return if we have enough.
      // Interpret limit == -1 to mean "take all".
      allMatches.push_back(item);

      if(limit != -1 && allMatches.size() >= limit){
        break;
      }
    }
  }

  return allMatches;
}

int setActiveFlag(bool active, const string& filePath, string& dataType, map<string, string> matchArgs){
  // There's supposed to be a '1' if the item is active and a '0' if it's not
  char activeFlag = active ? '1' : '0';
  unsigned int numModified = 0;

  auto mut = getDataFileMutex(filePath);
  unique_lock<mutex> lck(*mut);

  LReader reader = LReader(filePath, dataType);

  while(reader.hasNext()){
    auto item = reader.next();

    // Compare each item to its corresponding serialized version using the
    // criteria listed by matchArgs
    if(matchesSerialized(item, dataType, matchArgs)){
      // For each successful match, step back, modify the active bit to be
      // what was specified, and keep going.

      auto filePtr = reader.getFilePtr();
      auto currReadPtr = reader.getReadPtr();

      fseek(filePtr, -currReadPtr, SEEK_END);
      fwrite(&activeFlag, 1, 1, filePtr);
      fseek(filePtr, -currReadPtr, SEEK_END);

      numModified++;
    }
  }

  return numModified;
}
