#include <iostream>
#include <sstream>
#include "config.h"
#include "serializers.h"
using namespace std;


string extractField(const string& serialized, string& dataType, string& fieldType);


string serializeCredential(Credential& credential){
  string active = credential.active == Active::Yes ? "1" : "0";
  string username = pad(credential.username, configParams["FIELD_SIZE_USERNAME"]);
  string password = pad(credential.password, configParams["FIELD_SIZE_PASSWORD"]);

  return active + username + password;
}

bool matchesCredential(const string& serialized, Credential& credential){
  // Determine whether the provided parameters match the serialized credential
  string ser, fieldType, dataType = "CREDENTIAL";

  if(credential.active != Active::All){
    fieldType = "ACTIVE";

    ser = extractField(serialized, dataType, fieldType);
    if((credential.active == Active::Yes && ser.compare("1")) || \
       (credential.active == Active::No && ser.compare("0"))){
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

string serializeRelation(Relation& relation){
  string active = relation.active == Active::Yes ? "1" : "0";
  string firstUsername = pad(relation.firstUsername, configParams["FIELD_SIZE_USERNAME"]);
  string secondUsername = pad(relation.secondUsername, configParams["FIELD_SIZE_USERNAME"]);

  return active + firstUsername + relation.direction + secondUsername;
}

bool matchesRelation(const string& serialized, Relation& relation){
  // Determine whether the provided parameters match the serialized relation
  string ser, fieldType, dataType = "RELATION";

  if(relation.active != Active::All){
    fieldType = "ACTIVE";

    ser = extractField(serialized, dataType, fieldType);
    if((relation.active == Active::Yes && ser.compare("1")) || \
       (relation.active == Active::No && ser.compare("0"))){
      return false;
    }
  }

  if(!relation.firstUsername.empty()){
    fieldType = "FIRST_USERNAME";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(relation.firstUsername, configParams["FIELD_SIZE_USERNAME"]))){
      return false;
    }
  }

  if(relation.direction != '\0'){
    fieldType = "DIRECTION";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.at(0) != relation.direction){
      return false;
    }
  }

  if(!relation.secondUsername.empty()){
    fieldType = "SECOND_USERNAME";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(relation.secondUsername, configParams["FIELD_SIZE_USERNAME"]))){
      return false;
    }
  }

  return true;
}

string serializeProfilePost(ProfilePost& profilePost){
  // Combine the provided parameters into the following format, which is
  // returned as a string
  if(profilePost.timestamp.length() != 10){
    throw std::runtime_error("Length of the str. version of the timestamp must be 10");
  }

  string active = profilePost.active == Active::Yes ? "1" : "0";
  string username = pad(profilePost.username, configParams["FIELD_SIZE_USERNAME"]);
  string text = pad(profilePost.text, configParams["FIELD_SIZE_TEXT"]);

  return active + username + profilePost.timestamp + text;
}

bool matchesProfilePost(const string& serialized, ProfilePost& profilePost){
  string ser, fieldType, dataType = "PROFILE_POST";

  if(profilePost.active != Active::All){
    fieldType = "ACTIVE";

    ser = extractField(serialized, dataType, fieldType);
    if((profilePost.active == Active::Yes && ser.compare("1")) || \
       (profilePost.active == Active::No && ser.compare("0"))){
      return false;
    }
  }

  if(!profilePost.username.empty()){
    fieldType = "USERNAME";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(profilePost.username, configParams["FIELD_SIZE_USERNAME"]))){
      return false;
    }
  }

  if(!profilePost.timestamp.empty()){
    fieldType = "TIMESTAMP";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(profilePost.timestamp, configParams["FIELD_SIZE_TIMESTAMP"]))){
      return false;
    }
  }

  if(!profilePost.text.empty()){
    fieldType = "TEXT";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(profilePost.text, configParams["FIELD_SIZE_TEXT"]))){
      return false;
    }
  }

  return true;
}

string serializeTimelinePost(TimelinePost& timelinePost){
  // Combine the provided parameters into the following format, which is
  // returned as a string
  if(timelinePost.timestamp.length() != 10){
    throw std::runtime_error("Length of the str. version of the timestamp must be 10");
  }

  string active = timelinePost.active == Active::Yes ? "1" : "0";
  string username = pad(timelinePost.username, configParams["FIELD_SIZE_USERNAME"]);
  string author = pad(timelinePost.author, configParams["FIELD_SIZE_USERNAME"]);
  string text = pad(timelinePost.text, configParams["FIELD_SIZE_TEXT"]);

  return active + username + author + timelinePost.timestamp + text;
}

bool matchesTimelinePost(const string& serialized, TimelinePost& timelinePost){
  string ser, fieldType, dataType = "TIMELINE_POST";

  if(timelinePost.active != Active::All){
    fieldType = "ACTIVE";

    ser = extractField(serialized, dataType, fieldType);
    if((timelinePost.active == Active::Yes && ser.compare("1")) || \
       (timelinePost.active == Active::No && ser.compare("0"))){
      return false;
    }
  }

  if(!timelinePost.username.empty()){
    fieldType = "USERNAME";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(timelinePost.username, configParams["FIELD_SIZE_USERNAME"]))){
      return false;
    }
  }

  if(!timelinePost.author.empty()){
    fieldType = "AUTHOR";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(timelinePost.author, configParams["FIELD_SIZE_USERNAME"]))){
      return false;
    }
  }

  if(!timelinePost.timestamp.empty()){
    fieldType = "TIMESTAMP";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(timelinePost.timestamp, configParams["FIELD_SIZE_TIMESTAMP"]))){
      return false;
    }
  }

  if(!timelinePost.text.empty()){
    fieldType = "TEXT";

    ser = extractField(serialized, dataType, fieldType);
    if(ser.compare(pad(timelinePost.text, configParams["FIELD_SIZE_TEXT"]))){
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

/*
int main(){
  configServer();
}
/*
