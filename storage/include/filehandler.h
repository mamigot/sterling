#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <string>
#include <map>
#include <vector>
#include "config.h"
using namespace std;


int itemMatch(const string& filePath, string& dataType, map<string, string> matchArgs);

vector<string> itemMatchSweep(const string& filePath, string& dataType, map<string, string> matchArgs, int limit);

int setActiveFlag(bool active, const string& filePath, string& dataType, map<string, string> matchArgs);


#endif
