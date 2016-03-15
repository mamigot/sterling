#include <sys/stat.h>
#include "filehandler.h"
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
