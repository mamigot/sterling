#include <sstream>
#include <fstream>
#include <ctime>
#include <sys/stat.h>
#include "utils.h"
using namespace std;


bool startswith(const string& str, const string& match){
  return str.substr(0, match.length()).compare(match) == 0;
}

string getTimeNow(void){
  time_t now = time(NULL);

  stringstream strm;
  strm << now;

  return strm.str();
}

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
  ofstream outfile;

  outfile.open(path, ios_base::app);
  outfile << content;
}
