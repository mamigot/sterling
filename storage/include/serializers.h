#ifndef SERIALIZERS_H_
#define SERIALIZERS_H_

#include <string>
#include <map>
#include "config.h"
using namespace std;


string serializeCredential(Credential& credential);

string serializeRelation(Relation& relation);

string serializeProfilePost(ProfilePost& profilePost);

string serializeTimelinePost(TimelinePost& timelinePost);

bool matchesSerialized(const string& serialized, string& dataType, map<string, string> matchArgs);

// Pad value with as many instances of fillerChar as fieldSize requires
string pad(const string& value, unsigned int fieldSize);

// Remove all starting instances of fillerChar from value
string unpad(const string& value);

#endif
