# -*- coding: utf-8 -*-
"""API to manage stored users"""

import os
import time
from . import utils
from .config import StoredFileType


class UsernameAlreadyExists(Exception):
    pass

class User:
    def __init__(self, username, password=None):
        self.username = username
        self.password = password

    def save_credential(self):
        """
        Find the relevant file with the accounts and check if the wanted account
        exists (either active or inactive). If so, raise a UsernameAlreadyExists
        exception.

        If the relevant file does not contain a match for that username, serialize
        the login data (active, username, password) and append it to the file.
        """
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

        file_path = utils.get_path(self.username, StoredFileType.credential)
        file_size = os.path.getsize(file_path)
        item_size = utils.SerializedSizeBytes.credential

        credential = utils.serialize_credential(active=True, username=self.username,
            password=self.password)

        with open(file_path, 'r+') as f:
            read_ptr = item_size

            while abs(read_ptr) <= abs(file_size):
                f.seek(-read_ptr, os.SEEK_END)
                item = f.read(item_size)

                if utils.matches_credential(item, self.username, self.password):
                    raise UsernameAlreadyExists('Username "%s"' % self.username)

                read_ptr += item_size

            # Haven't found it, so just append it to the end of the file
            f.seek(0, os.SEEK_END)
            f.write(credential)

    def verify_credential(self):
        """
        Find the relevant file with the accounts and return True iff there's an
        entry marked "active" that matches the user's username and password.
        Otherwise, return False.
        """
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

    def delete_credential(self):
        """
        Find the relevant file with the accounts and, if there is a match for
        the user's username and password, mark it as "inactive" (overwrite this
        byte).
        """
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

    def save_post(self, text):
        """
        When a user creates a post, it'll be saved under his profile file as well
        as under all of his followers' timelines'. This makes writes slow but
        reads fast. Note: the fully-serialized post is copied across followers'
        files (not just references, which would prompt unnecessary I/O reads).

        This function serializes the post and appends it to the following:
        - The author's "profile" file
        - Each followers' "timeline" file
        """
        pass

    def delete_post(self, timestamp):
        """
        Read self.save_post() to understand how posts are saved first.

        Each post is identified by its user and the timestamp (a single user
        cannot write more than one post at once).

        This function marks as "inactive" entries in the following:
        - The author's "profile" file
        - Each followers' "timeline" file

        Marking a single post as "inactive" in a given file:
        - Search over the file starting from the end (the likelihood that older
        posts are interacted with is lower than for newer posts)
        - If the first three fields' (active, timestamp, username) values match
        the user input (for active = True), then the active byte is overwritten
        to denote "False" and the function returns.

        Matching the provided values (active, username, timestamp) to a given
        entry in the file:
        - Serialize the provided values as a string according to the format
        with which posts are serialized.
        - Match this string to each post's serialized string (will be the first
        characters).
        """
        pass

    def get_timeline_posts(self, limit=20):
        """
        Iterate over the entries from the given user's timeline posts file from
        the back and return the first `limit` matches that are marked as "active".

        Matching the provided values (active, username) to a given entry in the
        file:
        - Serialize the provided values as a string according to the format
        with which posts are serialized.
        - Match this string to each post's serialized string (will be the first
        characters).
        """
        pass

    def get_profile_posts(self, limit=20):
        """
        Iterate over the entries from the given user's profile posts file from
        the back and return the first `limit` matches that are marked as "active".

        Matching the provided values (active, username) to a given entry in the
        file:
        - Serialize the provided values as a string according to the format
        with which posts are serialized.
        - Match this string to each post's serialized string (will be the first
        characters).
        """
        pass

    def follow(self, friend):
        """
        Save the user's "follow" under his "relations" file, as well as in
        his friend's file (serialized differently).

        Before modifying a relation, iterate over the relevant file to make
        sure that it has not existed in the past (this entry would now be
        marked as "inactive"). If this happens, mark said entry to "active"
        and end.

        If the user's relation to his friend is new (would know after iterating
        through the relevant file), serialize it and append it to that file.
        """
        pass

    def unfollow(self, friend):
        """
        Read self.follow() to understand how relations are recorded.

        Need to mark as inactive the user's outbound link (direction ">") as
        well as the relation's inbound link (direction "<"). These will most
        likely be stored in different files.

        Find the relevant file with the relations and, if there is a match for
        the user's and the friend's usernames, mark it as "inactive" (overwrite
        this byte).
        """
        pass

    def get_followers(self, limit=20):
        """
        (Followers are users who follow this one)

        Iterate over the user's relations file and return all which are
        active and contain an inbound link (direction "<").
        """
        pass

    def get_friends(self, limit=20):
        """
        (Friends are users who this one follows)

        Iterate over the user's relations file and return all which are
        active and contain an outbound link (direction ">").
        """
        pass

    def is_following(self, friend):
        """
        Iterate over the user's relations file and return True if the user
        is following his friend –marked by an outbound link (direction ">").
        """
        pass
