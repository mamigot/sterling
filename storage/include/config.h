#include <string>
#include <map>
using namespace std;


// Configuration parameters/constants of the application
extern map<string, int> configParams;

// Absolute path to the user data files of the application
extern string STORAGE_FILES_PATH;

// Create the user data files of the application
void initiateStorage(void);
