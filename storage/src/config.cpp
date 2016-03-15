#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "config.h"
#include "filehandler.h"
using namespace std;


map<string, int> configParams;
string STORAGE_FILES_PATH;


void setConfigParams(void){
  // Get env. variable to path of txt file with configuration params
  char* configPath = getenv("CONFIG_PATH");
  if(configPath == NULL || strlen(configPath) == 0){
    throw std::runtime_error("CONFIG_PATH is not set!");
  }

  std::ifstream infile(configPath);

  std::string line;
  while(std::getline(infile, line)){
    std::istringstream iss(line);

    // Skip if the line is empty or starts with a '#' (comment)
    if(!line.empty() && line.substr(0, 1) != "#"){
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
	struct stat info;

	if(stat(path, &info) != 0)
		throw std::runtime_error("STORAGE_FILES_PATH does not contain a valid path");
	else if( info.st_mode & S_IFDIR )
		cerr << "setting STORAGE_FILES_PATH=" << path << "\n";

  STORAGE_FILES_PATH = string(path);
}

void initiateStorage(void){
  // Assumes that setStorageFilesPath() setConfigParams() have been called
	// Get the constants that denote file counts (start with this:)
	string match = "FILE_COUNT_";

  for(auto const& x : configParams){
		if(x.first.substr(0, match.length()).compare(match) == 0){
			string fileType = x.first.substr(match.length());

			for(int fileNum = 0; fileNum < x.second; fileNum++){
				string fileName = fileType + "_" + to_string(fileNum) + ".txt";
				string absPath = STORAGE_FILES_PATH + '/' + fileName;

				// Create the file if it does not exist
				if(!fileExists(absPath.c_str())){
					ofstream createdFile(absPath);
					cerr << "Adding storage file... " << absPath << "\n";
				}
			}
		}
  }
}

int main() {
  setConfigParams();

  setStorageFilesPath();

  initiateStorage();
}
