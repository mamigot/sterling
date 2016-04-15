#ifndef SERIALIZERS_H_
#define SERIALIZERS_H_

#include <string>
#include <map>
#include "config.h"
using namespace std;


// The following serializers convert the relevant data types into uniform strings
string serializeCredential(Credential& credential);

string serializeRelation(Relation& relation);

string serializeProfilePost(ProfilePost& profilePost);

string serializeTimelinePost(TimelinePost& timelinePost);

// Knowing what type of data was serialized, match it against a set of arguments
bool matchesSerialized(const string& serialized, string& dataType, const map<string, string> matchArgs);

// Pad value with as many instances of fillerChar as fieldSize requires
string pad(const string& value, unsigned int fieldSize);

// Remove all starting instances of fillerChar from value
string unpad(const string& value);

// Knowing what type of data was serialized, extract a given field from the string
string extractField(const string& serialized, string& dataType, string& fieldType);


#endif
