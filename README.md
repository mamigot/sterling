Sterling
========

- **NYU CS3254**: Project (Part I)
- Miguel Amigot [m.amigot@nyu.edu](m.amigot@nyu.edu)

## Functionality of the App
A simple version of Twitter. Users are able to follow each other and write posts, which are reflected on their own profiles as well as on their followers' timelines.

A user's timeline contains the posts from the people he follows. One caveat is that _a user's timeline only shows the posts from the people he follows that are written after the "follow" takes place. That is, the timeline is not supposed to aggregate historical posts._

The following features are supported:
- Create an account / register
- Log in with an existing account
- Deactivate an account
- Save posts
- Delete posts (as long as you're the owner)
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

## Architecture
Flask application communicating with a Python-based backend whose data is stored across several files of multiple types –there's a type for credential information (usernames and passwords), another type for relations (friends and followers), and two that store the actual posts.

### Reads faster than writes
When developing a feed-based application like this one, Twitter or Facebook,  where users expect to see an aggregation of the posts of the people they follow, the developer must choose whether to prioritize the speed of reading or writing posts because, at scale, one will be drastically faster than the other.

Moreover, since reads are more important to the user experience than writes (it's impossible to know if other users are reading what you're writing immediately, but easy to tell if it takes a while for your feed to assemble), reads tend to take preference.

The following source, in addition to [ref.1](http://www.slideshare.net/nkallen/q-con-3770885/), [ref.2](https://news.ycombinator.com/item?id=6007650) and [ref.3](http://dl.acm.org/citation.cfm?id=1807257), provides a clear motivation and starting-point in terms of the architecture to prioritize reads.

> Do a lot of processing when tweets arrive to figure out where tweets should go. This makes read time access fast and easy. **Don’t do any computation on reads.** With all the work being performed on the write path ingest rates are slower than the read path, on the order of 4000 QPS.

> -[The Architecture Twitter Uses...](http://highscalability.com/blog/2013/7/8/the-architecture-twitter-uses-to-deal-with-150m-active-users.html)

Simply, Twitter, etc. are able to maximize efficiency by "copying" the posts that a user writes to all of his followers' timelines (technically they store references into in-memory databases, but the point remains). That is why a tweet by Lady Gaga who has 31M followers might take as long as [five minutes](http://highscalability.com/blog/2013/7/8/the-architecture-twitter-uses-to-deal-with-150m-active-users.html) to show up in each of her follower's timeline.

Though only through text files, this project strives for a similar objective. That is, instead of responding to your `timeline/` request by assembling a list of all of the users you follow, aggregating their most recent posts and ordering them chronologically, it _only has to read from a single file_. (As will be explained below, every file is shared by multiple users, so each item must be properly encoded to ensure that a user only sees what he is intended to see.)

Consequently, writes are designed to be fairly slow in comparison. When a user saves a post, it has to be stored in not only the file wherein he keeps his own tweets, but also in his users' timeline files. Thus, each user keeps track of the tweets he authored in one file (`post_profile_*.txt`) and of those that he is supposed to see in another (`post_timeline_*.txt`).

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

Since each user is expected to have some data in each of those files and there are, by no means, as many files as there will be users, a hashing function is defined to determine the file number that corresponds to a user based on his username. The function that maps usernames and file types (credential, post_timeline, etc.) to a file's absolute path is `get_path(username, stored_type)` in `storage/utils/file_handler.py`.

The number of files that exist per type (also configurable through `storage/config.py`) was arbitrarily chosen based on how large they are expected to be and how frequently they will be accessed. This is why the number of files that hold credentials is smaller than the number of files which hold posts on a user's timeline.
