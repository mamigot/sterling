#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <string>
#include <map>
#include <vector>
#include "config.h"
using namespace std;


// True if the path points to a file or a dir, false otherwise
// http://stackoverflow.com/a/18101042/2708484
bool isValidPath(const char* path);

// See "bool isValidPath(const char* path)"
bool isValidPath(const string& path);

void appendToFile(const string& path, const string& content);

unsigned int getFileSize(const string& path);

int itemMatch(const string& filePath, string& dataType, map<string, string> matchArgs);

vector<string> itemMatchSweep(const string& filePath, string& dataType, map<string, string> matchArgs, int limit);

int setActiveFlag(bool active, const string& filePath, string& dataType, map<string, string> matchArgs);


#endif
