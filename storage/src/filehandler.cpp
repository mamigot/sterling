#include <iostream>
#include <fstream>
#include <sstream>
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

void appendToFile(const string& path, const string& content){
  std::ofstream outfile;

  outfile.open(path, std::ios_base::app);
  outfile << content;
}

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

  vector<string> allMatches;
  //stringstream allMatches;
  //unsigned int matchCount = 0;
  char item[itemSize];

  while(readPtr <= fileSize){
    fseek(matchedFile, -readPtr, SEEK_END);
    fread(item, itemSize, 1, matchedFile);
    item[itemSize] = '\0'; // Cap the garbage (without this, garbage is appended)

    if(matchesSerialized(string(item), dataType, matchArgs)){
      allMatches.push_back(item);
      //allMatches << item;
      //matchCount++;

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


/*
int main(){
  configServer();

  string username = "petrov";
  string filePath = getStoredFilePath(StoredFileType::CredentialFile, username);

  cout << filePath << "\n";

  map<string, string> matchArgs = {
    {"USERNAME", "4444user"}
  };
  string dataType = "CREDENTIAL";
  int numModified = setActiveFlag(false, filePath, dataType, matchArgs);

  cout << numModified << "\n";
}
*/
