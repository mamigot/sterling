#ifndef SERIALIZERS_H_
#define SERIALIZERS_H_

#include <string>
#include "config.h"
using namespace std;


string serializeCredential(Credential& credential);

string serializeRelation(Relation& relation);

string serializeProfilePost(ProfilePost& profilePost);

string serializeTimelinePost(TimelinePost& timelinePost);

bool matchesCredential(const string& serialized, Credential& credential);

bool matchesRelation(const string& serialized, Relation& relation);

bool matchesProfilePost(const string& serialized, ProfilePost& profilePost);

bool matchesTimelinePost(const string& serialized, TimelinePost& timelinePost);

// Pad value with as many instances of fillerChar as fieldSize requires
string pad(const string& value, unsigned int fieldSize);

// Remove all starting instances of fillerChar from value
string unpad(const string& value);

#endif
