"""API to manage stored users and posts.

Wanted functionality:
---------------------

user = User(username='mikel')
friend = User(username='pete')

user.save_post(post)
user.delete_post(post)

timeline_posts = user.get_timeline_posts()
profile_posts = user.get_profile_posts()

user.follow(friend)
user.unfollow(friend)

followers = user.get_followers()
friends = user.get_friends()

user.is_following(friend)
user.is_followed_by(friend)
"""

import os
import time
from .config import STORAGE_ROOT_PATH, STORED_FILE_TYPES, FIELD_SIZES
from . import utils


class User:
    def __init__(self, username, password=None):
        self.username = username
        self.password = password

    def save_login_data(self):
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

    def verify_login_data(self):
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

    def save_post(self, post):
        # Add the required attributes to the post
        # Serialize it
        # Append it to the correct files
        pass

    def delete_post(self, post):
        pass

    def get_timeline_posts(self, limit=20):
        timeline = []
        reader = utils.read_lines_backwards('test.txt', line_size=Post.BYTES_PER_POST)

        for serialized_posts in reader:
            for serialized_post in serialized_posts:
                if len(timeline) < limit:
                    post = Post.deserialize(serialized_post)

                    if post.active and post.post_type == 't' and post.username == self.username:
                        timeline.append(post)
                else:
                    return timeline

    def get_profile_posts(self, limit=20):
        pass

    def follow(self, friend):
        """Append an active relation to the end of the file"""
        pass

    def unfollow(self, friend):
        pass

    def get_followers(self, limit=20):
        pass

    def get_friends(self, limit=20):
        pass

    def is_following(self, friend):
        """Stick with the first relevant relation from the end of the file"""
        pass

    def is_followed_by(self, friend):
        """Stick with the first relevant relation from the end of the file"""
        pass

    def serialize_relation(self, friend, active=True):
        pass

    def _hash_to_filename(self, filetype):
        if filetype not in FILETYPE_MAXCOUNT:
            raise ValueError('Type of the file is unknown.')

        numeric_hash = sum(ord(c) for c in self.username)
        filenumber = numeric_hash % STORED_FILE_TYPES[filetype]

        return '%s_%d.txt' % (filetype, filenumber)

class Post:
    """Number of bytes that a serialized post takes up"""
    BYTES_PER_POST = 17 + FIELD_SIZES['username'] + FIELD_SIZES['post_text']

    def __init__(self, text, active=True, timestamp=None, username=None, post_type=None):
        if post_type and post_type not in ['p', 't']:
            raise ValueError('post_type: either profile ("p") or timeline ("t")')

        self.text = text
        self.active = active
        self.timestamp = int(timestamp) or int(time.time())
        self.username = username
        self.post_type = post_type

    def serialize(self):
        # Check that all relevant properties are defined (neither None nor '')
        for param in filter(lambda param: getattr(self, param) in [None, ''],
            ('active', 'timestamp', 'username', 'post_type')):
            raise ValueError('"%s" cannot be undefined' % param)

        timestamp = str(self.timestamp)

        if len(timestamp) != 10:
            raise ValueError('Length of the timestamp must be 10')

        active = '1' if self.active else '0'
        username = utils.pad(self.username, FIELD_SIZES['username'])
        text = utils.pad(self.text, FIELD_SIZES['post_text'])

        return '%s_%s_%s_%s_%s\n' % (active, self.post_type, timestamp, username, text)

    @classmethod
    def deserialize(self, serialized):
        active = True if serialized[0] == '1' else False
        post_type = serialized[2]
        timestamp = serialized[4:14]

        start_idx_username = 16
        end_idx_username = 15 + FIELD_SIZES['username']

        username = utils.unpad(serialized[start_idx_username:end_idx_username])

        start_idx_text = 17 + FIELD_SIZES['username']
        end_idx_text = 16 + FIELD_SIZES['username'] + FIELD_SIZES['post_text']

        text = utils.unpad(serialized[start_idx_text:end_idx_text])

        return Post(text, active, timestamp, username, post_type)
