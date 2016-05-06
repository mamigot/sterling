#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <mutex>
#include "utils.h"
#include "storage/filehandler.h"
#include "storage/config.h"
using namespace std;


const string STORAGE_FILES_PATH(getenv("STORAGE_FILES_PATH"));

const map<StoredFileType, string> storedFileTypes = {
  {StoredFileType::CredentialFile, "CREDENTIAL"},
  {StoredFileType::RelationFile, "RELATION"},
  {StoredFileType::ProfilePostFile, "PROFILE_POST"},
  {StoredFileType::TimelinePostFile, "TIMELINE_POST"}
};

// Only modify the configParams after locking the mutex
map<string, int> configParams;
mutex configParamsAccess;

void setConfigParams();
void initiateStorage();

void configStorage(){
  setConfigParams();
  initiateStorage();
}

void setConfigParams(){
  // Get env. variable to path of txt file with configuration params
  // (these are constants that are used throughout for serialization, etc.)
  char* configPath = getenv("CONFIG_PATH");
  if(configPath == NULL || strlen(configPath) == 0){
    throw std::runtime_error("CONFIG_PATH is not set!");
  }

  ifstream infile(configPath);
  unique_lock<mutex> lck(configParamsAccess);

  string line;
  while(std::getline(infile, line)){
    istringstream iss(line);

    // Skip if the line is empty or starts with a '#' (comment)
    if(!line.empty() && !startswith(line, "#")){
      // constants and variables are split by a '='
      int divider = line.find("=");

      string constantKey = line.substr(0, divider);
      int constantValue = stoi(line.substr(divider + 1));

      configParams[constantKey] = constantValue;
    }
  }
}

void initiateStorage(void){
  // Assumes that setStorageFilesPath() setConfigParams() have been called
	// Get the constants that denote file counts (start with this:)
	string match = "FILE_COUNT_";

  for(auto const& x : configParams){
		if(startswith(x.first, match)){
			string fileType = x.first.substr(match.length());

			for(int fileNum = 0; fileNum < x.second; fileNum++){
				string fileName = fileType + "_" + to_string(fileNum) + ".txt";
				string absPath = STORAGE_FILES_PATH + '/' + fileName;

				// Create the file if it does not exist
				if(!isValidPath(absPath)){
					ofstream createdFile(absPath);
					cerr << "Adding storage file... " << absPath << "\n";
				}
			}
		}
  }
}

string getStoredFilePath(StoredFileType storedFileType, const string& username){
  // "Hash" the username to a number within the relevant file's limits
  // and return the path of the file that corresponds to that hash

  string param = "FILE_COUNT_" + storedFileTypes.at(storedFileType);
  unsigned int maxFileNum = configParams.at(param);

  if(!maxFileNum){
    throw std::runtime_error("maxFileNum is zero. Likely that configStorage() has not been called.");
  }

  unsigned int numericUsername = 0;
  for(unsigned int i = 0; i < username.size(); i++) {
    numericUsername += username[i];
  }

  unsigned int fileNum = numericUsername % maxFileNum;

  // Matches the format set by initiateStorage()
  string fileName = storedFileTypes.at(storedFileType) + "_" + to_string(fileNum) + ".txt";

  return STORAGE_FILES_PATH + '/' + fileName;
}
