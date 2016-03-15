#include "utils.h"
using namespace std;


bool startswith(const string& str, const string& match){
  return str.substr(0, match.length()).compare(match) == 0;
}
