#ifndef UTILS_H_
#define UTILS_H_

#include <string>
using namespace std;


// True if str starts with match
bool startswith(const string& str, const string& match);

string getTimeNow(void);

// True if the path points to a file or a dir, false otherwise
// http://stackoverflow.com/a/18101042/2708484
bool isValidPath(const char* path);

// See "bool isValidPath(const char* path)"
bool isValidPath(const string& path);

unsigned int getFileSize(const string& path);

void appendToFile(const string& path, const string& content);


#endif
