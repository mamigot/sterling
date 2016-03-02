# -*- coding: utf-8 -*-
"""API to manage stored users"""

import os
import time
from . import utils
from .config import StoredFileType


class UsernameAlreadyExists(Exception):
    pass

class CannotVerifyCredential(Exception):
    pass

class User:
    def __init__(self, username, password=None):
        self.username = username
        self.password = password

    def exists(self):
        """
        Find the relevant file with the accounts and check if the wanted account
        exists and it's active. If so, output True.
        """
        credential_path = utils.get_path(self.username, StoredFileType.credential)

        match_location = utils.item_match(
            file_path=credential_path,
            item_size=utils.SerializedSizeBytes.credential,
            compare_func=utils.matches_credential,
            compare_kwargs={'active':True, 'username':self.username}
        )

        return match_location is not None

    def save_credential(self):
        """
        Find the relevant file with the accounts and check if the wanted account
        exists (either active or inactive). If so, raise a UsernameAlreadyExists
        exception.

        If the relevant file does not contain a match for that username, serialize
        the login data (active, username, password) and append it to the file.

        Raises:
            AttributeError: If the User object does not contain a password
            UsernameAlreadyExists: If the provided username already exists in
                the system
        """
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

        credential_path = utils.get_path(self.username, StoredFileType.credential)

        match_location = utils.item_match(
            file_path=credential_path,
            item_size=utils.SerializedSizeBytes.credential,
            compare_func=utils.matches_credential,
            compare_kwargs={
                'username':self.username,
                'password':self.password
            }
        )

        if match_location:
            raise UsernameAlreadyExists('Username "%s"' % self.username)
        else:
            # Haven't found it, so just append it to the end of the file
            with open(credential_path, 'a') as f:
                f.write(utils.serialize_credential(
                    active=True,
                    username=self.username,
                    password=self.password
                ))

    def verify_credential(self):
        """
        Find the relevant file with the accounts and return True iff there's an
        entry marked "active" that matches the user's username and password.
        Otherwise, return False.

        Raises:
            AttributeError: If the User object does not contain a password
            CannotVerifyCredential: If there's no active match for the given
                (username, password) combination
        """
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

        match_location = utils.item_match(
            file_path=utils.get_path(self.username, StoredFileType.credential),
            item_size=utils.SerializedSizeBytes.credential,
            compare_func=utils.matches_credential,
            compare_kwargs={
                'active':True,
                'username':self.username,
                'password':self.password
            }
        )

        if match_location is None:
            raise CannotVerifyCredential()

    def delete_credential(self):
        """
        Find the relevant file with the accounts and, if there is a match for
        the user's username and password, mark it as "inactive" (overwrite this
        byte).

        Before deactivating the account, it deletes all posts written by the user.

        Raises:
            AttributeError: If the User object does not contain a password
            CannotVerifyCredential: If there's no active match for the given
                (username, password) combination
        """
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

        for post in self.get_profile_posts(limit=None):
            self.delete_post(post.timestamp)

        num_modified = utils.set_active_flag(
            active_flag=False,
            file_path=utils.get_path(self.username, StoredFileType.credential),
            item_size=utils.SerializedSizeBytes.credential,
            compare_func=utils.matches_credential,
            compare_kwargs={
                'active':True,
                'username':self.username,
                'password':self.password
            }
        )

        if num_modified == 0:
            raise CannotVerifyCredential()

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
        post_timestamp = time.time()

        profile_path = utils.get_path(self.username, StoredFileType.post_profile)

        with open(profile_path, 'a') as f:
            profile_post = utils.serialize_profile_post(
                active=True,
                username=self.username,
                timestamp=post_timestamp,
                text=text
            )
            f.write(profile_post)

        for follower in self.get_followers():
            timeline_path = utils.get_path(follower.username, StoredFileType.post_timeline)

            with open(timeline_path, 'a') as f:
                f.write(utils.serialize_timeline_post(
                    active=True,
                    username=follower.username,
                    author=self.username,
                    timestamp=post_timestamp,
                    text=text
                ))

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
        profile_path = utils.get_path(self.username, StoredFileType.post_profile)

        utils.set_active_flag(
            active_flag=False,
            file_path=utils.get_path(self.username, StoredFileType.post_profile),
            item_size=utils.SerializedSizeBytes.profile_post,
            compare_func=utils.matches_profile_post,
            compare_kwargs={
                'active':True,
                'username':self.username,
                'timestamp':timestamp
            }
        )

        for follower in self.get_followers():
            utils.set_active_flag(
                active_flag=False,
                file_path=utils.get_path(follower.username, StoredFileType.post_timeline),
                item_size=utils.SerializedSizeBytes.timeline_post,
                compare_func=utils.matches_timeline_post,
                compare_kwargs={
                    'active':True,
                    'username':follower.username,
                    'author':self.username,
                    'timestamp':timestamp
                }
            )

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
        serialized_posts = utils.item_match_sweep(
            file_path=utils.get_path(self.username, StoredFileType.post_timeline),
            item_size=utils.SerializedSizeBytes.timeline_post,
            compare_func=utils.matches_timeline_post,
            compare_kwargs={'active':True, 'username':self.username},
            limit=limit
        )

        timeline_posts = []

        for serialized in serialized_posts:
            deserialized = utils.deserialize_timeline_post(serialized)

            timeline_posts.append(Post(
                active=deserialized.get('active'),
                username=deserialized.get('author'),
                timestamp=deserialized.get('timestamp'),
                text=deserialized.get('text')
            ))

        return timeline_posts

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
        serialized_posts = utils.item_match_sweep(
            file_path=utils.get_path(self.username, StoredFileType.post_profile),
            item_size=utils.SerializedSizeBytes.profile_post,
            compare_func=utils.matches_profile_post,
            compare_kwargs={'active':True, 'username':self.username},
            limit=limit
        )

        return [Post(**utils.deserialize_profile_post(sp)) for sp in serialized_posts]

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
        if not friend.exists(): return

        user_relation_path = utils.get_path(self.username, StoredFileType.relation)
        user_num_modified = utils.set_active_flag(
            active_flag=True,
            file_path=user_relation_path,
            item_size=utils.SerializedSizeBytes.relation,
            compare_func=utils.matches_relation,
            compare_kwargs={
                'active':False,
                'first_username':self.username,
                'direction':'>',
                'second_username':friend.username
            }
        )

        if user_num_modified == 0:
            with open(user_relation_path, 'a') as f:
                f.write(utils.serialize_relation(
                    active=True,
                    first_username=self.username,
                    direction='>',
                    second_username=friend.username
                ))

        friend_relation_path = utils.get_path(friend.username, StoredFileType.relation)
        friend_num_modified = utils.set_active_flag(
            active_flag=True,
            file_path=friend_relation_path,
            item_size=utils.SerializedSizeBytes.relation,
            compare_func=utils.matches_relation,
            compare_kwargs={
                'active':False,
                'first_username':friend.username,
                'direction':'<',
                'second_username':self.username
            }
        )

        if friend_num_modified == 0:
            with open(friend_relation_path, 'a') as f:
                f.write(utils.serialize_relation(
                    active=True,
                    first_username=friend.username,
                    direction='<',
                    second_username=self.username
                ))

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
        utils.set_active_flag(
            active_flag=False,
            file_path=utils.get_path(self.username, StoredFileType.relation),
            item_size=utils.SerializedSizeBytes.relation,
            compare_func=utils.matches_relation,
            compare_kwargs={
                'active':True,
                'first_username':self.username,
                'direction':'>',
                'second_username':friend.username
            }
        )

        utils.set_active_flag(
            active_flag=False,
            file_path=utils.get_path(friend.username, StoredFileType.relation),
            item_size=utils.SerializedSizeBytes.relation,
            compare_func=utils.matches_relation,
            compare_kwargs={
                'active':True,
                'first_username':friend.username,
                'direction':'<',
                'second_username':self.username
            }
        )

        utils.set_active_flag(
            active_flag=False,
            file_path=utils.get_path(self.username, StoredFileType.post_timeline),
            item_size=utils.SerializedSizeBytes.timeline_post,
            compare_func=utils.matches_timeline_post,
            compare_kwargs={
                'active':True,
                'username':self.username,
                'author':friend.username,
            }
        )

    def get_followers(self, limit=20):
        """
        (Followers are users who follow this one)

        Iterate over the user's relations file and return all which are
        active and contain an inbound link (direction "<").
        """
        serialized_relations = utils.item_match_sweep(
            file_path=utils.get_path(self.username, StoredFileType.relation),
            item_size=utils.SerializedSizeBytes.relation,
            compare_func=utils.matches_relation,
            compare_kwargs={
                'active':True,
                'first_username':self.username,
                'direction':'<'
            },
            limit=limit
        )

        followers = []

        for serialized in serialized_relations:
            deserialized = utils.deserialize_relation(serialized)
            followers.append(User(username=deserialized.get('second_username')))

        return followers

    def get_friends(self, limit=20):
        """
        (Friends are users who this one follows)

        Iterate over the user's relations file and return all which are
        active and contain an outbound link (direction ">").
        """
        serialized_relations = utils.item_match_sweep(
            file_path=utils.get_path(self.username, StoredFileType.relation),
            item_size=utils.SerializedSizeBytes.relation,
            compare_func=utils.matches_relation,
            compare_kwargs={
                'active':True,
                'first_username':self.username,
                'direction':'>'
            },
            limit=limit
        )

        friends = []

        for serialized in serialized_relations:
            deserialized = utils.deserialize_relation(serialized)
            friends.append(User(username=deserialized.get('second_username')))

        return friends

    def is_following(self, friend):
        """
        Iterate over the user's relations file and return True if the user
        is following his friend –marked by an outbound link (direction ">").
        """
        relation_path = utils.get_path(self.username, StoredFileType.relation)

        match_location = utils.item_match(
            file_path=relation_path,
            item_size=utils.SerializedSizeBytes.relation,
            compare_func=utils.matches_relation,
            compare_kwargs={
                'active':True,
                'first_username':self.username,
                'direction':'>',
                'second_username':friend.username
            }
        )

        return match_location is not None

class Post:
    def __init__(self, active, username, timestamp, text):
        self.active = active
        self.username = username
        self.timestamp = timestamp
        self.text = text
