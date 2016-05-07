#ifndef ACCESS_H_
#define ACCESS_H_

#include <string>
#include <vector>
using namespace std;


bool exists(const string& username);

bool verifyCredential(const string& username, const string& password);

// true if success, false otherwise
bool saveCredential(const string& username, const string& password);

// true if success, false otherwise
bool deleteCredential(const string& username, const string& password);

// true if success, false otherwise
bool savePost(const string& username, const string& text);
bool savePost(const string& username, const string& text, const string& postTimestamp);

// true if success, false otherwise
bool deletePost(const string& username, const string& timestamp);

vector<string> getTimelinePosts(const string& username, int limit);

vector<string> getProfilePosts(const string& username, int limit);

bool isFollowing(const string& username, const string& friendUsername);

// true if success, false otherwise
bool follow(const string& username, const string& friendUsername);

// true if success, false otherwise
bool unfollow(const string& username, const string& friendUsername);

vector<string> getFollowers(const string& username, int limit);

vector<string> getFriends(const string& username, int limit);


#endif
