#include <iostream>
#include <sstream>
#include <thread>
#include <cassert>
#include <chrono>
#include <ctime>
#include <stdlib.h>
#include "config.h"
#include "user.h"
#include "serializers.h"
using namespace std;

string randomString(int length){
  static const char allowed[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz ";

  ostringstream random;
  for(int i = 0; i < length; i++)
    random << allowed[rand() % length];

  return random.str();
}

bool testCredentials(){
  int numThreads = 100;
  vector<thread> testers;

  // std::this_thread::sleep_for(std::chrono::seconds(1));

  cerr << "START:\t credentials: " << numThreads << " threads." << endl;

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  for(int i = 0; i < numThreads; i++){
    string username = randomString(10), password = randomString(10);

    testers.push_back(
      thread([&] { saveCredential(username, password); })
    );
  }

  for(auto& th:testers){ th.join(); }
  end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end - start;
  cerr << "END:\t credentials: " << elapsed_seconds.count() << " seconds." << endl;

  return true;
}

bool testPosts(){
  int numThreads = 100;
  vector<thread> testers;

  cerr << "START:\t posts: " << numThreads << " threads." << endl;

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  for(int i = 0; i < numThreads; i++){
    string username = randomString(10), text = randomString(20);

    testers.push_back(
      thread([&] { savePost(username, text); })
    );
  }

  for(auto& th:testers){ th.join(); }
  end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end - start;
  cerr << "END:\t posts: " << elapsed_seconds.count() << " seconds." << endl;

  return true;
}

int main(){
  configServer();

  // Register test functions
  vector<bool (*)()> testFunctions;
  testFunctions.push_back(testCredentials);
  testFunctions.push_back(testPosts);

  // Execute each test function and abort if there's a failure
  for(auto func:testFunctions){ assert(func()); }
}
