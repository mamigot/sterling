#ifndef UTILS_H_
#define UTILS_H_

#include <string>
using namespace std;


// True if str starts with match
bool startswith(const string& str, const string& match);

string getTimeNow(void);

// True if the path points to a file or a dir, false otherwise
bool isValidPath(const string& filePath);

unsigned int getFileSize(const string& filePath);


#endif
