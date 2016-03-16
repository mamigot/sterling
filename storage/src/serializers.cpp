#include <iostream>
#include <sstream>
#include "config.h"
#include "serializers.h"
using namespace std;


string extractField(const string& serialized, string& dataType, string& fieldType);


string serializeCredential(Credential& credential){
  string active = credential.status == Status::Active ? "1" : "0";
  string username = pad(credential.username, configParams["FIELD_SIZE_USERNAME"]);
  string password = pad(credential.password, configParams["FIELD_SIZE_PASSWORD"]);

  return active + username + password;
}

bool matchesCredential(const string& serialized, Credential& credential){
  // Determine whether the provided parameters match the serialized credential
  string ser, fieldType, dataType = "CREDENTIAL";

  if(credential.status != Status::Unspecified){
    fieldType = "ACTIVE";

    ser = extractField(serialized, dataType, fieldType);
    if((credential.status == Status::Active && ser.compare("1")) || \
       (credential.status == Status::Inactive && ser.compare("0"))){
      return false;
    }
  }

  if(!credential.username.empty()){
    fieldType = "USERNAME";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(credential.username, configParams["FIELD_SIZE_USERNAME"]))){
      return false;
    }
  }

  if(!credential.password.empty()){
    fieldType = "PASSWORD";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(credential.password, configParams["FIELD_SIZE_PASSWORD"]))){
      return false;
    }
  }

  return true;
}

string pad(const string& value, unsigned int fieldSize){
  int extraCount = fieldSize - value.length();

  if(extraCount < 0){
    throw std::runtime_error("Given value exceeds specified fieldSize");
  }

  stringstream padding;
  while(extraCount--){
    padding << fillerChar;
  }

  return padding.str() + value;
}

string unpad(const string& value){
  size_t fillerPos = value.find_last_of(fillerChar);
  return value.substr(fillerPos + 1);
}

string extractField(const string& serialized, string& dataType, string& fieldType){
  // Extract start and end bounds based on the type of the data and field
  // The constants with the index information are provided by the config module

  string wanted = "SERIAL_" + dataType + "_" + fieldType;

  unsigned int startIdx = configParams[wanted + "_START"];
  unsigned int endIdx = configParams[wanted + "_END"];

  string match = serialized.substr(startIdx, (endIdx - startIdx));
  if(match.empty()){
    throw std::runtime_error("No config parameter found");
  }

  return match;
}


int main(){
  configServer();

  /*
  Credential credential = {
    Status::Active,
    "someone",
    "somepass"
  };
  string ser = serializeCredential(credential);
  cout << ser << "\n\n";

  Credential brosef = {
    Status::Inactive,
    "someone",
    "somepass"
  };
  bool matches = matchesCredential(ser, brosef);
  cout << matches << "\n";
  */
}
