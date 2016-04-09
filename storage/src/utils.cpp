#include <sstream>
#include <fstream>
#include <ctime>
#include <stdexcept>
#include <sys/stat.h>
#include "utils.h"
using namespace std;


bool startswith(const string& str, const string& match){
  // True if match is at the beginning of str
  return str.substr(0, match.length()).compare(match) == 0;
}

string getTimeNow(void){
  // Time now as a string
  time_t now = time(NULL);

  stringstream strm;
  strm << now;

  return strm.str();
}

bool isValidPath(const string& filePath){
  // True if the path is valid (whether to a directory or a file)
  // http://stackoverflow.com/a/18101042/2708484
  struct stat info;
  return stat(filePath.c_str(), &info) == 0;
}

unsigned int getFileSize(const string& filePath){
  // Get the size of a file
  if(!isValidPath(filePath)){
    throw std::runtime_error("Given path is invalid.");
  }

  ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
  return in.tellg();
}
