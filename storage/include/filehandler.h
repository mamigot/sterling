#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <string>
#include <map>
#include "config.h"
using namespace std;


// True if the path points to a file or a dir, false otherwise
// http://stackoverflow.com/a/18101042/2708484
bool isValidPath(const string& path);

// True if the path points to a file or a dir, false otherwise
// http://stackoverflow.com/a/18101042/2708484
bool isValidPath(const char* path);

unsigned int getFileSize(const string& path);

int itemMatch(const string& storedFilePath, string& dataType, map<string, string> matchArgs);

#endif
