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
        True if there's an active user with the provided username in the system.
        Otherwise, False.
        """
        pass

    def save_credential(self):
        """
        Save the credential (username, password) as an active account in the
        system.

        Raises:
            AttributeError: If the User object does not contain a password
            UsernameAlreadyExists: If the provided username already exists in
                the system
        """
        pass

    def verify_credential(self):
        """
        True if there's an active entry in the system that matches the user's
        username and password. Otherwise, False.

        Raises:
            AttributeError: If the User object does not contain a password
            CannotVerifyCredential: If there's no active match for the given
                (username, password) combination
        """
        pass

    def delete_credential(self):
        """
        Delete all posts written by the user and deactivate his account if his
        credential (username, password) exists in the system.

        Raises:
            AttributeError: If the User object does not contain a password
            CannotVerifyCredential: If there's no active match for the given
                (username, password) combination
        """
        pass

    def save_post(self, text):
        """
        Save a user's post to the system (will show up on his profile as well
        as on his followers' timelines).

        Args:
            text (str): text content of the post
        """
        pass

    def delete_post(self, timestamp):
        """
        Delete a user's post from the sytem (his profile as well as on his
        followers' timelines). Note that each post is identified by its user and
        the timestamp (a single user cannot write more than one post at once).

        Args:
            timestamp (type(time.time()) == float): UTC timestamp identifying
                the post
        """
        pass

    def get_timeline_posts(self, limit=20):
        """
        Returns active posts from the user's timeline.

        Args:
            limit (int): the max. number of returned posts
        """
        pass

    def get_profile_posts(self, limit=20):
        """
        Returns active posts from the user's timeline.

        Args:
            limit (int): the max. number of returned posts
        """
        pass

    def follow(self, friend):
        """
        Record the user's follow to "friend".
        """
        pass

    def unfollow(self, friend):
        """
        Record the user's unfollow to "friend".
        """
        pass

    def get_followers(self, limit=20):
        """
        Returns profiles of people who follow the user.

        Args:
            limit (int): the max. number of returned profiles
        """
        pass

    def get_friends(self, limit=20):
        """
        Returns profiles of people who the user is following.

        Args:
            limit (int): the max. number of returned profiles
        """
        pass

    def is_following(self, friend):
        """
        True if the user is following "friend". Otherwise, False.
        """
        pass

class Post:
    def __init__(self, active, username, timestamp, text):
        self.active = active
        self.username = username
        self.timestamp = timestamp
        self.text = text
