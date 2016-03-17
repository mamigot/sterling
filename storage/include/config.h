#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <map>
using namespace std;


const char fillerChar = '~';

enum Active {Yes, No, All};

struct Credential{
  Active active;
  string username;
  string password;
};

struct Relation{
  Active active;
  string firstUsername;
  char direction;
  string secondUsername;
};

struct ProfilePost{
  Active active;
  string username;
  string timestamp;
  string text;
};

struct TimelinePost{
  Active active;
  string username;
  string author;
  string timestamp;
  string text;
};

enum StoredFileType{
  CredentialFile,
  RelationFile,
  ProfilePostFile,
  TimelinePostFile,
};

// Absolute path to the user data files of the application
extern string STORAGE_FILES_PATH;

// Maps StoredFileType enum to string version of filename
extern map<StoredFileType, string> storedFileTypes;

// Configuration parameters/constants of the application
extern map<string, int> configParams;


// Before each session: creates the user data files of the application if they
// don't exist and makes other relevant params available
void configServer(void);

// Get the absolute path to a stored file, provided its type as well as the
// username of the user
string getStoredFilePath(StoredFileType storedFileType, const string& username);

#endif
