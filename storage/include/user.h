#ifndef ACCESS_H_
#define ACCESS_H_

#include <string>
#include <vector>
using namespace std;


bool exists(const string& username);

bool verifyCredential(const string& username, const string& password);

void saveCredential(const string& username, const string& password);

void deleteCredential(const string& username, const string& password);

void savePost(const string& username, const string& text);

void deletePost(const string& username, const string& timestamp);

vector<string> getTimelinePosts(const string& username, unsigned int limit);

vector<string> getProfilePosts(const string& username, unsigned int limit);

bool isFollowing(const string& username, const string& friendUsername);

void follow(const string& username, const string& friendUsername);

void unfollow(const string& username, const string& friendUsername);

vector<string> getFollowers(const string& username, unsigned int limit);

vector<string> getFriends(const string& username, unsigned int limit);


#endif
