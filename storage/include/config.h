#include <string>
#include <map>
using namespace std;


// Configuration parameters/constants of the application
extern map<string, int> configParams;

// Absolute path to the user data files of the application
extern string STORAGE_FILES_PATH;

// Before each session: creates the user data files of the application if they
// don't exist and makes other relevant params available
void configServer(void);
