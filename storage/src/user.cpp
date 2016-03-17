#include <iostream>
#include <map>
#include "config.h"
#include "filehandler.h"
#include "serializers.h"
#include "utils.h"
#include "user.h"
using namespace std;


bool exists(const string& username){
  string credentialPath = getStoredFilePath(StoredFileType::CredentialFile, username);
  string dataType = "CREDENTIAL";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username}
  };

  int matchLocation = itemMatch(credentialPath, dataType, matchArgs);
  return matchLocation != -1;
}

bool verifyCredential(const string& username, const string& password){
  string credentialPath = getStoredFilePath(StoredFileType::CredentialFile, username);
  string dataType = "CREDENTIAL";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username},
    {"PASSWORD", password}
  };

  int matchLocation = itemMatch(credentialPath, dataType, matchArgs);
  return matchLocation != -1;
}

void saveCredential(const string& username, const string& password){
  string credentialPath = getStoredFilePath(StoredFileType::CredentialFile, username);
  string dataType = "CREDENTIAL";

  map<string, string> matchArgs = {
    {"USERNAME", username},
    {"PASSWORD", password}
  };

  int matchLocation = itemMatch(credentialPath, dataType, matchArgs);

  if(matchLocation == -1){
    Credential credential = {Active::Yes, username, password};
    string serialized = serializeCredential(credential);

    appendToFile(credentialPath, serialized);
  }
}

void deleteCredential(const string& username, const string& password){
  string timestamp, dataType = "PROFILE_POST", fieldType = "TIMESTAMP";
  for(string& serializedPost : getProfilePosts(username, -1)){
    timestamp = extractField(serializedPost, dataType, fieldType);
    deletePost(username, timestamp);
  }

  string credentialPath = getStoredFilePath(StoredFileType::CredentialFile, username);
  dataType = "CREDENTIAL";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username},
    {"PASSWORD", password}
  };

  int numModified = setActiveFlag(false, credentialPath, dataType, matchArgs);
  if(!numModified){
    throw std::runtime_error("Cannot verify credential.");
  }
}

void savePost(const string& username, const string& text){
  // Record the user's profile file
  string postTimestamp = getTimeNow();

  string profilePostPath = getStoredFilePath(StoredFileType::ProfilePostFile, username);

  ProfilePost profilePost = {Active::Yes, username, postTimestamp, text};
  string serialized = serializeProfilePost(profilePost);
  appendToFile(profilePostPath, serialized);

  // Record to each follower's timeline file
  string followerUsername, timelinePostPath;
  for(string& paddedFollowerUsername : getFollowers(username, -1)){
    followerUsername = unpad(paddedFollowerUsername);
    timelinePostPath = getStoredFilePath(StoredFileType::TimelinePostFile, followerUsername);

    TimelinePost timelinePost = {Active::Yes, followerUsername, username, postTimestamp, text};
    serialized = serializeTimelinePost(timelinePost);
    appendToFile(timelinePostPath, serialized);
  }
}

void deletePost(const string& username, const string& timestamp){
  string profilePostPath = getStoredFilePath(StoredFileType::ProfilePostFile, username);
  string dataType = "PROFILE_POST";

  map<string, string> matchArgs = {
    {"ACTIVE", "1"},
    {"USERNAME", username},
    {"TIMESTAMP", timestamp}
  };

  setActiveFlag(false, credentialPath, dataType, matchArgs);

  string followerUsername, timelinePostPath;
  for(string& paddedFollowerUsername : getFollowers(username, -1)){
    followerUsername = unpad(paddedFollowerUsername);
    timelinePostPath = getStoredFilePath(StoredFileType::TimelinePostFile, followerUsername);
    dataType = "TIMELINE_POST";

    matchArgs = { /***************DOUBLECHECK IF THIS REDEFINITION WORKS ***************************************/
      {"ACTIVE", "1"},
      {"USERNAME", followerUsername},
      {"AUTHOR", username},
      {"TIMESTAMP", timestamp}
    };

    setActiveFlag(false, timelinePostPath, dataType, matchArgs);
  }
}

vector<string> getFollowers(void){
  vector<string> facts;
  facts.push_back("jomamma");
  facts.push_back("nooowayy");
  facts.push_back("btotaror");

  return facts;
}

int main(){
  configServer();

}
