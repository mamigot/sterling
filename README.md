Sterling
========

- **NYU CS3254**: Project (Part I)
- Miguel Amigot

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

### Storage
The `start.sh` script which initializes the files used for storage, under `storage/volumes/`. These are divided into the following categories, determined by `storage/config.py`:
- `credential_*.txt`
- `post_timeline_*.txt`
- `post_profile_*.txt`
- `credential_*.txt`

Since each user is expected to have some data in each of those files and there are, by no means, as many files as there will be users, a hashing function is defined to determine the file number that corresponds to a user based on his username. The function that maps usernames and file types (credential, post_timeline, etc.) to a file's absolute path is `get_path(username, stored_type)` in `storage/utils/file_handler.py`.

The number of files that exist per type (also configurable through `storage/config.py`) was arbitrarily chosen based on how large they are expected to be and how frequently they will be accessed.
