Sterling
========

- **NYU CS3254**: Project (Part I)
- Miguel Amigot [m.amigot@nyu.edu](m.amigot@nyu.edu)

## Functionality of the App
A simple version of Twitter. Users are able to follow each other and write posts, which are reflected on their own profiles as well as on their followers' timelines.

A user's timeline contains the posts from the people he follows. One caveat is that _a user's timeline only shows the posts from the people he follows that are written after the "follow" takes place._

The following features are supported:
- Create an account / register
- Log in with an existing account
- Deactivate an account
- Save posts
- Delete posts
- View the posts of people you follow in your "timeline"
- View your own posts in your "profile"
- Browse through friends (people you follow)
- Browse through followers (people who follow you)

### Testing
Start by creating the files which are used for storage and start the server by running the following on the root directory of the project:

```bash
. start.sh
```

Go to [http://127.0.0.1:5000/](http://127.0.0.1:5000/).

#### Possible workflow:
- Create `$ACCOUNT_1`
- Sign out and create `$ACCOUNT_2`
- Go to `$ACCOUNT_1`'s profile (`http://127.0.0.1:5000/profile/$ACCOUNT_1`) and follow him
- Sign out and log in with `$ACCOUNT_1` to write a post
- Sign out and log in with `$ACCOUNT_2` to view that post on your timeline

## Architecture
Flask application communicating with a Python-based backend whose data is stored across several files of multiple types –there's a type for credential information (usernames and passwords), another type for relations (friends and followers), and two that store the actual posts.

### Reads faster than writes
When developing a feed-based application like this one, Twitter or Facebook,  where users expect to see an aggregation of the posts of the people they follow, the developer must choose whether to prioritize the speed of reading or writing posts because, at scale, one will be drastically faster than the other.

Moreover, since reads are more important to the user experience than writes (it's impossible to know if other users are reading what you're writing immediately, but easy to tell if it takes a while for your feed to assemble), reads tend to have preference.

The following source, in addition to [ref.1](http://dl.acm.org/citation.cfm?id=1807257), [ref.2](http://www.slideshare.net/nkallen/q-con-3770885/) and [ref.3](https://news.ycombinator.com/item?id=6007650), provides a clear motivation and overview in terms of architectures that prioritize reads.

> Do a lot of processing when tweets arrive to figure out where tweets should go. This makes read time access fast and easy. **Don’t do any computation on reads.** With all the work being performed on the write path ingest rates are slower than the read path, on the order of 4000 QPS.

> -[The Architecture Twitter Uses...](http://highscalability.com/blog/2013/7/8/the-architecture-twitter-uses-to-deal-with-150m-active-users.html)

Simply, Twitter, etc. are able to maximize efficiency by "copying" the posts that a user writes to all of his followers' timelines (technically they store references into in-memory databases, but the point remains). That is why a post by Lady Gaga who has 31M followers might take as long as [five minutes](http://highscalability.com/blog/2013/7/8/the-architecture-twitter-uses-to-deal-with-150m-active-users.html) to show up across all of her followers' timelines.

Though only through text files, this project strives for a similar objective. That is, instead of responding to your `timeline/` request by assembling a list of all of the users you follow, aggregating their most recent posts and ordering them chronologically, it _only has to read from a single file_.

Consequently, writes are designed to be fairly slow in comparison. When a user saves a post, it has to be stored in not only the file wherein he keeps his own posts, but also in his users' timeline files. Thus, each user keeps track of the posts he authored in one file (`post_profile_*.txt`) and of those that he is supposed to see in another (`post_timeline_*.txt`).

In order to further increase the efficiency with which reads are made, the files with the relevant posts are read backwards. As shown in "Serialization" below, this is simple once an entry is programmed to be of a certain length (iterating is as simply as decrementing a pointer by a fixed number of bytes).

### Storage
The `start.sh` script which initializes the files used for storage, under `storage/volumes/`. These are divided into the following categories, determined by `storage/config.py`:

- `credential_*.txt`
  - Usernames and passwords
- `post_timeline_*.txt`
  - Timeline posts: aggregated from all of the people the user follows
- `post_profile_*.txt`
  - Profile posts: contain only the posts that a user has written (this makes reads on `profile/` pages as fast as reads on `timeline/` pages)
- `relation_*.txt`
  - Friends and followers: when `$USER_A` follows `$USER_B`, this is encoded as two "edges": from `$USER_A` to `$USER_B` and from `$USER_B` to `$USER_A`

Since each user is expected to have some data in each of those files and there are, by no means, as many files as there are users, a hashing function is defined to determine the file number that corresponds to a user based on his username. The function that maps usernames and file types (credential, post_timeline, etc.) to a file's absolute path is `get_path(username, stored_type)` in `storage/utils/file_handler.py`.

The number of files that exist per type (also configurable through `storage/config.py`) was arbitrarily chosen based on how large they are expected to be and how frequently they will be accessed. This is why the number of files that hold credentials is smaller than the number of files which hold posts on a user's timeline.

This is believed to be more efficient than two extreme alternatives:
1. Put all users' information of a given type (maintain the distinction between credentials, relations, etc.) in the same file.
2. Have one type of file per user.

Though the first case would be beneficial because it is likely that all users' data will be contained in memory (once the OS reads the file into memory, it will remain there as long as it is not too large, and users will be able to reference its data without prompting additional I/O operations), it will require iterations over a lot of entries that will ultimately be discarded. The second alternative would prompt an I/O operation for essentially every user, which would be more costly than iterating through many irrelevant entries (as long as we are iterating from the back and retrieving tens of posts as opposed to thousands).

### Serialization
In order to efficiently iterate over the data in the text files, each type of data is serialized according to a given format that keeps its size uniform. For example, regardless of whether a post takes up 4 or 40 characters, it will be stored as a 100-character one. The same applies to usernames and passwords. (These sizes are specified by `UserFieldSizes` in `storage/config.py`.)

The following serialization formats are enforced by `storage/utils/serializers.py`, which convert sets of parameters to strings depending on the relevant field (documented in this module):

- Credential:
  - `<active><username><password>`
- Profile post:
  - `<active><username><timestamp><text>`
- Timeline post:
  - `<active><username><author><timestamp><text>`
- Relation:
  - `<active><first_username><direction><second_username>`
