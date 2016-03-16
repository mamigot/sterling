#include <string>
using namespace std;


const char fillerChar = '~';

enum Active {Yes, No, All};

struct Credential{
  Active active;
  string username;
  string password;
};

struct Relation{
  Active active;
  string firstUsername;
  char direction;
  string secondUsername;
};

struct ProfilePost{
  Active active;
  string username;
  string timestamp;
  string text;
};

struct TimelinePost{
  Active active;
  string username;
  string author;
  string timestamp;
  string text;
};


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
