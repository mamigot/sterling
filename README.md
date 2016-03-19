Sterling
========

- **NYU CS3254**: Project (Part II)
- Miguel Amigot [m.amigot@nyu.edu](m.amigot@nyu.edu)

#### _Modifications with respect to Part I_

##### _Architecture_
See "Architecture" below. Backend no longer runs on the same process as the Python-based client –it is now written in C++ and communicates over sockets.

##### _User Interface_
- User feedback when logging in, registering and deactivating accounts
- Users' posts are deleted after their accounts are deactivated
- Ability to search for users
- Default post size is now 100 characters
- Minor fixes (typos, unused args to functions, etc.)

## Functionality
A simple version of Twitter. Users are able to follow each other and write posts, which are reflected on their own profiles as well as on their followers' timelines.

A user's timeline contains the posts from the people he follows. One caveat is that _a user's timeline only shows the posts from the people he follows that are written after the "follow" takes place._

### Supported features:
- Create an account / register
- Log in with an existing account
- Deactivate an account
- Save posts
- Delete posts
- View the posts of people you follow in your "timeline"
- View your own posts in your "profile"
- Browse through friends (people you follow)
- Browse through followers (people who follow you)
- Search for other users

## Architecture
The client is a Flask (Python) application. It communicates with a server written in C++ over sockets. The server does not have to run in the same machine as the client. All that's needed is to specify the hostname and port number that will be used via environment variables, as specified under "Configuration and testing" below.

Parameters and constants that need to be shared between the client and the server are specified under a global config. file (`config.txt`) on their respective root directories.

### Reads faster than writes
When developing a feed-based application like this one, Twitter or Facebook, where users expect to see an aggregation of the posts of the people they follow, the developer must choose whether to prioritize the speed of reading or writing posts because, at scale, one will be drastically faster than the other.

Moreover, since reads are more important to the user experience than writes (it's impossible to know if other users are reading what you're writing immediately, but easy to tell if it takes a while for your feed to assemble), reads tend to have preference.

The following source, in addition to [ref.1](http://dl.acm.org/citation.cfm?id=1807257), [ref.2](http://www.slideshare.net/nkallen/q-con-3770885/) and [ref.3](https://news.ycombinator.com/item?id=6007650), provides a clear motivation and overview in terms of architectures that prioritize reads.

> Do a lot of processing when tweets arrive to figure out where tweets should go. This makes read time access fast and easy. **Don’t do any computation on reads.** With all the work being performed on the write path ingest rates are slower than the read path, on the order of 4000 QPS.

> -[The Architecture Twitter Uses...](http://highscalability.com/blog/2013/7/8/the-architecture-twitter-uses-to-deal-with-150m-active-users.html)

Simply, Twitter, etc. are able to maximize efficiency by "copying" the posts that a user writes to all of his followers' timelines (technically they store references into in-memory databases, but the point remains). That is why a post by Lady Gaga who has 31M followers might take as long as [five minutes](http://highscalability.com/blog/2013/7/8/the-architecture-twitter-uses-to-deal-with-150m-active-users.html) to show up across all of her followers' timelines.

Though only through text files, this project strives for a similar objective. That is, instead of responding to your `timeline/` request by assembling a list of all of the users you follow, aggregating their most recent posts and ordering them chronologically, it _only has to read from a single file_.

Consequently, writes are designed to be fairly slow in comparison. When a user saves a post, it has to be stored in not only the file wherein he keeps his own posts, but also in his users' timeline files. Thus, the application keeps track of the posts each user has stored in one file (`TIMELINE_POST_*.txt`) and of those that he is supposed to see in another (`TIMELINE_POST_*.txt`);

In order to further increase the efficiency with which reads are made, the files with the relevant posts are read backwards. As shown in "Serialization" below, this is simple once an entry is programmed to be of a certain length (iterating is as simply as decrementing a pointer by a fixed number of bytes).

### Communication protocol
There is a series of commands that the client uses to communicate with the server. They are categorized into the following verbs, which are loosely based on HTTP's:
- GET
  - Retrieve stored data. Make no modifications.
- SAVE
  - Add to the stored data.
- DELETE
  - Mark a stored piece of data as inactive (this is as far as deletes go; nothing is actually deleted).

Available commands, which are parsed using Regular Expressions by the server:
```
- GET/credential/username\0
- GET/credential/username:password\0
- GET/posts/profile/username:limit\0
- GET/posts/timeline/username:limit\0
- GET/relations/username:friendUsername\0
- GET/relations/followers/username:limit\0
- GET/relations/friends/username:limit\0
- SAVE/credential/username:password\0
- SAVE/posts/username:text\0
- SAVE/relations/username:friendUsername\0
- DELETE/credential/username:password\0
- DELETE/posts/username:timestamp\0
- DELETE/relations/username:friendUsername\0
```

#### Workflow
The following illustrates the workflow that the client undergoes to request data from the server:
- The client makes a request by sending one of the commands from above, containing the relevant fields (`web/main/access.py`).
- If the request is valid (`storage/src/protocol.cpp`), the server replies with `201: Expect packets: X`. "X" is an integer that denotes the number of packets that the user should expect to receive. If the request is invalid, the server replies with a string that starts with "500" and ends the connection to the user (starts waiting for a new one).
- Once the client knows how many packets to expect, it replies with an `ACK` denoting that it agrees to accept that number of packets (`web/main/client.py`). If the client responds with `STOP` instead, the server ends the connection and starts waiting for a new user.
- After receiving the `ACK`, the server starts to send its packets (tries to fit as much data as possible into a single packet, whose size is denoted by the `$DATASERVER_BUFFSIZE` environment variable). After it sends each packet, the server waits for an `ACK` by the client. Once the client has sent as many `ACK` messages as the server instructed it to at the beginning, the server ends the connection and starts waiting for a new one.

### Serialization (client-server)
A serialization format is defined for each data type that is shared by the client and the server. Pertinent parameters such as the length of each serialized string, integers marking the start and end of each field, etc. are written on the shared `config.txt`.

The following defines the serialized format of each data type:
- Relation:
  - `<active><first_username><direction><second_username>`
- Credential:
  - `<active><username><password>`
- Profile post:
  - `<active><username><timestamp><text>`
- Timeline post:
  - `<active><username><author><timestamp><text>`

### Storage (server)
The application divides all of its data into the following types of files:
- `CREDENTIAL_*.txt`
  - Usernames and passwords
- `RELATION_*.txt`
  - Friends and followers: when `$USER_A` follows `$USER_B`, this is encoded as two "edges": from `$USER_A` to `$USER_B` and from `$USER_B` to `$USER_A`
- `TIMELINE_POST_*.txt`
  - Timeline posts: aggregated from all of the people the user follows
- `PROFILE_POST_*.txt`
  - Profile posts: the posts that a user has written (this makes reads on `profile/` pages as fast as reads on `timeline/` pages)

The number of files of each type varies according to the size with which they're expected to grow (i.e. there are more files with posts than with credentials). These counts are specified under `config.txt`.

## Configuration and testing
The client and the server are configured and activated through `start.sh` scripts on their respective root directories. These take care of relevant environment variables and, in the case of the server's, create the text files that are used to store the application's data.

Once the client and server processes are running, go to [http://127.0.0.1:5000/](http://127.0.0.1:5000/).

### Possible testing workflow
- Create `$ACCOUNT_1`
- Sign out and create `$ACCOUNT_2`
- Go to `$ACCOUNT_1`'s profile (`http://127.0.0.1:5000/profile/$ACCOUNT_1`) and follow him
- Sign out and log in with `$ACCOUNT_1` to write a post
- Sign out and log in with `$ACCOUNT_2` to view that post on your timeline
