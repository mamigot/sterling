#include <iostream>
#include <sstream>
#include <thread>
#include <cassert>
#include <stdlib.h>
#include "config.h"
#include "user.h"
#include "serializers.h"
using namespace std;

string randomString(int length){
  static const char allowed[] = // http://www.cplusplus.com/forum/windows/88843/
    "0123456789"
    "!@#$^&*"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz ";

  ostringstream random;
  for(int i = 0; i < length; i++)
    random << allowed[rand() % length];

  return random.str();
}

bool testConcurrency(){
  // Determine whether threads deadlock / are able to execute successfully
  // when concurrently accessing a data-file.

  // Save many posts for the same user
  string text, username = "bobbyTables";
  int numPosts = 500;

  cerr << "Saving " << numPosts << " posts..." << endl;

  for(int i = 0; i < numPosts; i++)
    savePost(username, randomString(50));

  // Access them through many threads
  int numThreads = 1000;
  vector<thread> testers;

  cerr << "Accessing them through " << numThreads << " threads..." << endl;

  for(int i = 0; i < numThreads; i++)
    testers.push_back(
      thread([&] { getProfilePosts(username, -1); })
    );

  for(auto& th:testers){ th.join(); }

  // Successful execution implies that no deadlock took place
  return true;
}

int main(){
  configServer();

  // Register test functions
  vector<bool (*)()> testFunctions;
  testFunctions.push_back(testConcurrency);

  // Execute each test function and abort if there's a failure
  for(auto func:testFunctions){ assert(func()); }
}
