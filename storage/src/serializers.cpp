#include <iostream>
#include <sstream>
#include <stdexcept>
#include "serializers.h"
using namespace std;


// The serializers follow the formats that are specified by configParams
string serializeCredential(Credential& credential){
  string active = credential.active == Active::Yes ? "1" : "0";
  string username = pad(credential.username, configParams["FIELD_SIZE_USERNAME"]);
  string password = pad(credential.password, configParams["FIELD_SIZE_PASSWORD"]);

  return active + username + password;
}

string serializeRelation(Relation& relation){
  string active = relation.active == Active::Yes ? "1" : "0";
  string firstUsername = pad(relation.firstUsername, configParams["FIELD_SIZE_USERNAME"]);
  string secondUsername = pad(relation.secondUsername, configParams["FIELD_SIZE_USERNAME"]);

  return active + firstUsername + relation.direction + secondUsername;
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

bool matchesSerialized(const string& serialized, string& dataType, map<string, string> matchArgs){
  // Determine whether the provided parameters match the serialized item
  if(!configParams.count("FILE_COUNT_" + dataType)){
    throw std::runtime_error("Given dataType is unknown");
  }

  string ser, fieldType, potentialMatch;
  for(auto const& x : matchArgs){
    fieldType = x.first;
    potentialMatch = x.second;
    ser = extractField(serialized, dataType, fieldType);

    // Pad fields accordingly before comparing them to their serialized
    // counterparts. Each type of field requires a different type of padding.
    if(!fieldType.compare("USERNAME") || \
       !fieldType.compare("AUTHOR") || \
       !fieldType.compare("FIRST_USERNAME") || \
       !fieldType.compare("SECOND_USERNAME")){
      potentialMatch = pad(potentialMatch, configParams["FIELD_SIZE_USERNAME"]);

    }else if(!fieldType.compare("PASSWORD")){
      potentialMatch = pad(potentialMatch, configParams["FIELD_SIZE_PASSWORD"]);

    }else if(!fieldType.compare("TEXT")){
      potentialMatch = pad(potentialMatch, configParams["FIELD_SIZE_TEXT"]);
    }

    if(ser.compare(potentialMatch)){
      return false;
    }
  }

  return true;
}

string pad(const string& value, unsigned int fieldSize){
  // Create a string of size fieldSize where value is at the right and
  // the rest of the characters are fillers (padded)
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
  // Removed filler characters from a string
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
