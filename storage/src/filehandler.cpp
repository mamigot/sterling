#include "filehandler.h"
using namespace std;


bool fileExists(const string& filename){
  // True if the file exists, false otherwise
  // http://stackoverflow.com/a/6296808/2708484
  struct stat buf;
  if(stat(filename.c_str(), &buf) != -1){
    return true;
  }
  return false;
}
