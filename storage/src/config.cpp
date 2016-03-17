#include <iostream>
#include <fstream>
#include <sstream>
#include "filehandler.h"
#include "utils.h"
#include "config.h"
using namespace std;


string STORAGE_FILES_PATH;

map<string, int> configParams;

map<StoredFileType, string> storedFileTypes = {
  {StoredFileType::CredentialFile, "CREDENTIAL"},
  {StoredFileType::RelationFile, "RELATION"},
  {StoredFileType::ProfilePostFile, "PROFILE_POST"},
  {StoredFileType::TimelinePostFile, "TIMELINE_POST"}
};


void setConfigParams(void);
void setStorageFilesPath(void);
void initiateStorage(void);
string getFileName(StoredFileType storedFileType, unsigned int fileNum);


void configServer(void){
  setConfigParams();
  setStorageFilesPath();
  initiateStorage();
}

void setConfigParams(void){
  // Get env. variable to path of txt file with configuration params
  // (these are constants that are used throughout for serialization, etc.)
  char* configPath = getenv("CONFIG_PATH");
  if(configPath == NULL || strlen(configPath) == 0){
    throw std::runtime_error("CONFIG_PATH is not set!");
  }

  ifstream infile(configPath);

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

void setStorageFilesPath(void){
  // Get env. variable to path of user data files
  char* path = getenv("STORAGE_FILES_PATH");
  if(path == NULL || strlen(path) == 0)
		throw std::runtime_error("STORAGE_FILES_PATH is not set!");

	// Check if the provided directory is valid
  if(isValidPath(path)){
    cerr << "setting STORAGE_FILES_PATH=" << path << "\n";
    STORAGE_FILES_PATH = string(path);
  }else{
    throw std::runtime_error("STORAGE_FILES_PATH does not contain a valid path");
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
  unsigned int maxFileNum = configParams["FILE_COUNT_" + storedFileTypes[storedFileType]];

  unsigned int numericUsername = 0;
  for(unsigned int i = 0; i < username.size(); i++) {
    numericUsername += username[i];
  }

  string fileName = getFileName(storedFileType, (numericUsername % maxFileNum));
  return STORAGE_FILES_PATH + '/' + fileName;
}

string getFileName(StoredFileType storedFileType, unsigned int fileNum){
  // Matches the format set by initiateStorage()
  return storedFileTypes[storedFileType] + "_" + to_string(fileNum) + ".txt";
}


//int main(){
//  configServer();
//}
