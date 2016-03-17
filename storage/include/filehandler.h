#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <string>
#include "config.h"
using namespace std;


// True if the path points to a file or a dir, false otherwise
// http://stackoverflow.com/a/18101042/2708484
bool isValidPath(const string& path);

// True if the path points to a file or a dir, false otherwise
// http://stackoverflow.com/a/18101042/2708484
bool isValidPath(const char* path);

unsigned int getFileSize(const string& path);

int itemMatchCredential(const string& storedFilePath, Credential& credential);

#endif
