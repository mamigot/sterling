#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <string>
#include <map>
#include <vector>
#include "config.h"
using namespace std;


class LReader {
public:
  LReader(const string& filePath, const string& dataType);
  ~LReader();

  bool hasNext();
  string next();
  bool hasPrev();
  string prev();
  int getItemSize();
  int getReadPtr();
  FILE* getFilePtr();

private:
  FILE* matchedFile;
  int fileSize;
  int itemSize;
  int offsetFromEnd;

  string readItem(int offsetFromEnd);
};

// Iterates through the file backwards and returns an offset (in bytes) of the
// first matching entry
int itemMatch(const string& filePath, string& dataType, map<string, string> matchArgs);

// Iterates through the file backwards and returns relevant entries
vector<string> itemMatchSweep(const string& filePath, string& dataType, map<string, string> matchArgs, int limit);

// Iterates through the file backwards and modifies each entry's "active" status
int setActiveFlag(bool active, const string& filePath, string& dataType, map<string, string> matchArgs);


#endif
