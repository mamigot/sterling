from . import client, serializers


class UsernameAlreadyExists(Exception):
    pass

class CannotVerifyCredential(Exception):
    pass

class ErrorProcessingRequest(Exception):
    pass


class User:
    def __init__(self, username, password=None):
        self.username = username
        self.password = password

    def exists(self): # GET/credential/self.username\0
        """
        True if there's an active user with the provided username in the system.
        Otherwise, False.
        """
        command = get_request_command('exists', dict(username=self.username))

        output = client.request(command)
        return True if output == 'true' else False

    def verify_credential(self): # GET/credential/self.username:self.password\0
        """
        True if there's an active entry in the system that matches the user's
        username and password.

        Raises:
            AttributeError: If the User object does not contain a password
            CannotVerifyCredential: If there's no active match for the given
                (username, password) combination
        """
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

        command = get_request_command('verify_credential', dict(
            username=self.username, password=self.password
        ))

        output = client.request(command)

        if output == 'true':
            return True
        else:
            raise CannotVerifyCredential()

    def save_credential(self): # SAVE/credential/self.username:self.password\0
        """
        Save the credential (username, password) as an active account in the
        system.

        Raises:
            AttributeError: If the User object does not contain a password
            UsernameAlreadyExists: If the provided username already exists in
                the system
        """
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

        command = get_request_command('save_credential', dict(
            username=self.username, password=self.password
        ))

        output = client.request(command)

        if output == 'true':
            return True
        else:
            raise UsernameAlreadyExists()

    def delete_credential(self): # DELETE/credential/self.username:self.password\0
        """
        Delete all posts written by the user and deactivate his account if his
        credential (username, password) exists in the system.

        Raises:
            AttributeError: If the User object does not contain a password
            CannotVerifyCredential: If there's no active match for the given
                (username, password) combination
        """
        if not hasattr(self, 'password'):
            raise AttributeError("Provide the user's password.")

        command = get_request_command('delete_credential', dict(
            username=self.username, password=self.password
        ))

        output = client.request(command)

        if output != 'true':
            raise CannotVerifyCredential()

    def save_post(self, text): # SAVE/posts/self.username:text\0
        """
        Save a user's post to the system (will show up on his profile as well
        as on his followers' timelines).

        Args:
            text (str): text content of the post
        """
        command = get_request_command('save_post', dict(
            username=self.username, text=text
        ))

        output = client.request(command)

        if output != 'true':
            raise ErrorProcessingRequest()

    def delete_post(self, timestamp): # DELETE/posts/self.username:timestamp\0
        """
        Delete a user's post from the sytem (his profile as well as on his
        followers' timelines). Note that each post is identified by its user and
        the timestamp (a single user cannot write more than one post at once).

        Args:
            timestamp (type(time.time()) == float): UTC timestamp identifying
                the post
        """
        command = get_request_command('delete_post', dict(
            username=self.username, timestamp=timestamp
        ))

        output = client.request(command)

        if output != 'true':
            raise ErrorProcessingRequest()

    def get_timeline_posts(self, limit=20): # GET/posts/timeline/self.username:limit\0
        """
        Returns active posts from the user's timeline.

        Args:
            limit (int): the max. number of returned posts
        """
        command = get_request_command('get_timeline_posts', dict(
            username=self.username, limit=limit
        ))

        output = client.request(command)
        serialized_posts = split_chunk(output, config.SERIAL_SIZE_TIMELINE_POST)

        posts = []
        for serialized_post in serialized_posts:
            posts.append(Post(**serializers.deserialize_timeline_post()))

        return posts

    def get_profile_posts(self, limit=20): # GET/posts/profile/self.username:limit\0
        """
        Returns active posts from the user's timeline.

        Args:
            limit (int): the max. number of returned posts
        """
        command = get_request_command('get_profile_posts', dict(
            username=self.username, limit=limit
        ))

        output = client.request(command)
        serialized_posts = split_chunk(output, config.SERIAL_SIZE_PROFILE_POST)

        posts = []
        for serialized_post in serialized_posts:
            posts.append(Post(**serializers.deserialize_timeline_post()))

        return posts

    def is_following(self, friend): # GET/relations/self.username:friend.username\0
        """
        True if the user is following "friend". Otherwise, False.
        """
        command = get_request_command('is_following', dict(
            username=self.username, friend_username=friend.username
        ))

        output = client.request(command)
        return output == 'true'

    def follow(self, friend): # SAVE/relations/self.username:friend.username\0
        """
        Record the user's follow to "friend".
        """
        command = get_request_command('follow', dict(
            username=self.username, friend_username=friend.username
        ))

        output = client.request(command)

        if output != 'true':
            raise ErrorProcessingRequest()

    def unfollow(self, friend): # DELETE/relations/self.username:friend.username\0
        """
        Record the user's unfollow to "friend".
        """
        command = get_request_command('unfollow', dict(
            username=self.username, friend_username=friend.username
        ))

        output = client.request(command)

        if output != 'true':
            raise ErrorProcessingRequest()

    def get_followers(self, limit=20): # GET/relations/followers/self.username:limit\0
        """
        Returns profiles of people who follow the user.

        Args:
            limit (int): the max. number of returned profiles
        """
        command = get_request_command('get_followers', dict(
            username=self.username, limit=limit
        ))

        output = client.request(command)
        serialized_relations = split_chunk(output, config.SERIAL_SIZE_RELATION)

        usernames = []
        for serialized_relation in serialized_relations:
            relation = serializers.deserialize_relation(serialized_relation)
            usernames.append(relation['second_username'])

        return usernames

    def get_friends(self, limit=20): # GET/relations/friends/self.username:limit\0
        """
        Returns profiles of people who the user is following.

        Args:
            limit (int): the max. number of returned profiles
        """
        command = get_request_command('get_friends', dict(
            username=self.username, limit=limit
        ))

        output = client.request(command)
        serialized_relations = split_chunk(output, config.SERIAL_SIZE_RELATION)

        usernames = []
        for serialized_relation in serialized_relations:
            relation = serializers.deserialize_relation(serialized_relation)
            usernames.append(relation['second_username'])

        return usernames

class Post:
    def __init__(self, active, username, timestamp, text):
        self.active = active
        self.username = username
        self.timestamp = timestamp
        self.text = text

def get_request_command(endpoint, **params):
    if endpoint == 'exists':
        return 'GET/credential/%s\0' % params.get('username')

    elif endpoint == 'verify_credential':
        return 'GET/credential/%s:%s\0' % (
            params.get('username'), params.get('password')
        )

    elif endpoint == 'save_credential':
        return 'SAVE/credential/%s:%s\0' % (
            params.get('username'), params.get('password')
        )

    elif endpoint == 'delete_credential':
        return 'DELETE/credential/%s:%s\0' % (
            params.get('username'), params.get('password')
        )

    elif endpoint == 'save_post':
        return 'SAVE/posts/%s:%s\0' % (
            params.get('username'), params.get('text')
        )

    elif endpoint == 'delete_post':
        return 'DELETE/posts/%s:%s\0' % (
            params.get('username'), params.get('timestamp')
        )

    elif endpoint == 'get_timeline_posts':
        return 'GET/posts/timeline/%s:%s\0' % (
            params.get('username'), params.get('limit')
        )

    elif endpoint == 'get_profile_posts':
        return 'GET/posts/profile/%s:%s\0' % (
            params.get('username'), params.get('limit')
        )

    elif endpoint == 'is_following':
        return 'GET/relations/%s:%s\0' % (
            params.get('username'), params.get('friend_username')
        )

    elif endpoint == 'follow':
        return 'SAVE/relations/%s:%s\0' % (
            params.get('username'), params.get('friend_username')
        )

    elif endpoint == 'unfollow':
        return 'DELETE/relations/%s:%s\0' % (
            params.get('username'), params.get('friend_username')
        )

    elif endpoint == 'get_followers':
        return 'GET/relations/followers/%s:%s\0' % (
            params.get('username'), params.get('limit')
        )

    elif endpoint == 'get_friends':
        return 'GET/relations/friends/%s:%s\0' % (
            params.get('username'), params.get('limit')
        )

def split_chunk(chunk, unit_size):
    return [chunk[i:i+unit_size] for i in range(0, len(chunk), unit_size)]
