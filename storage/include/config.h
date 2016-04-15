#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <map>
using namespace std;


// Server connection parameters (get from environment variables)
const unsigned int PORT_NUM = stoi(getenv("DATASERVER_PORT"), NULL, 10);
// Max. size of the buffer used to transport data between the server and client
const unsigned int BUFFSIZE = stoi(getenv("DATASERVER_BUFFSIZE"), NULL, 10);

// Used to pad serialized values
const char fillerChar = '~';

// Data that is marked as inactive is effectively "deleted"
enum Active {Yes, No, All};

// The following structs define the types of stored fields
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
extern const string STORAGE_FILES_PATH;

// Maps StoredFileType enum to string version of filename
extern const map<StoredFileType, string> storedFileTypes;

// Configuration parameters/constants of the application
extern map<string, int> configParams;

// Before each session: creates the user data files of the application if they
// don't exist and makes other relevant params available
void configServer();

// Get the absolute path to a stored file, provided its type as well as the
// username of the user
string getStoredFilePath(StoredFileType storedFileType, const string& username);


#endif
