import config, client, serializers


class ErrorProcessingRequest(Exception):
    pass


class User:
    """Used to make requests to the server and parse the outputs accordingly"""
    def __init__(self, username, password=None):
        self.username = username
        self.password = password

    def exists(self): # GET/credential/self.username\0
        """
        True if there's an active user with the provided username in the system.
        Otherwise, False.
        """
        command = 'GET/credential/%s\0' % self.username
        output = client.request(command)

        return output == 'true'

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

        command = 'GET/credential/%s:%s\0' % (self.username, self.password)
        output = client.request(command)

        if output != 'true':
            raise ErrorProcessingRequest()

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

        command = 'SAVE/credential/%s:%s\0' % (self.username, self.password)
        output = client.request(command)

        if output != 'success':
            raise ErrorProcessingRequest()

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

        command = 'DELETE/credential/%s:%s\0' % (self.username, self.password)
        output = client.request(command)

        if output != 'success':
            raise ErrorProcessingRequest()

    def save_post(self, text): # SAVE/posts/self.username:text\0
        """
        Save a user's post to the system (will show up on his profile as well
        as on his followers' timelines).

        Args:
            text (str): text content of the post
        """
        command = 'SAVE/posts/%s:%s\0' % (self.username, text)
        output = client.request(command)

        if output != 'success':
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
        command = 'DELETE/posts/%s:%s\0' % (self.username, timestamp)
        output = client.request(command)

        if output != 'success':
            raise ErrorProcessingRequest()

    def get_timeline_posts(self, limit=-1): # GET/posts/timeline/self.username:limit\0
        """
        Returns active posts from the user's timeline.

        Args:
            limit (int): the max. number of returned posts. -1 means no limit.
        """
        command = 'GET/posts/timeline/%s:%s\0' % (self.username, limit)
        output = client.request(command)

        serialized_posts = split_chunk(output, config.SERIAL_SIZE_TIMELINE_POST)

        posts = []
        for serialized_post in serialized_posts:
            deserialized = serializers.deserialize_timeline_post(serialized_post)
            posts.append(Post(
                active=deserialized.get('active'),
                username=deserialized.get('author'),
                timestamp=deserialized.get('timestamp'),
                text=deserialized.get('text')
            ))

        return posts

    def get_profile_posts(self, limit=-1): # GET/posts/profile/self.username:limit\0
        """
        Returns active posts from the user's timeline.

        Args:
            limit (int): the max. number of returned posts. -1 means no limit.
        """
        command = 'GET/posts/profile/%s:%s\0' % (self.username, limit)
        output = client.request(command)

        serialized_posts = split_chunk(output, config.SERIAL_SIZE_PROFILE_POST)

        posts = []
        for serialized_post in serialized_posts:
            deserialized = serializers.deserialize_profile_post(serialized_post)
            posts.append(Post(**deserialized))

        return posts

    def is_following(self, friend): # GET/relations/self.username:friend.username\0
        """
        True if the user is following "friend". Otherwise, False.
        """
        command = 'GET/relations/%s:%s\0' % (self.username, friend.username)
        output = client.request(command)

        return output == 'true'

    def follow(self, friend): # SAVE/relations/self.username:friend.username\0
        """
        Record the user's follow to "friend".
        """
        command = 'SAVE/relations/%s:%s\0' % (self.username, friend.username)
        output = client.request(command)

        if output != 'success':
            raise ErrorProcessingRequest()

    def unfollow(self, friend): # DELETE/relations/self.username:friend.username\0
        """
        Record the user's unfollow to "friend".
        """
        command = 'DELETE/relations/%s:%s\0' % (self.username, friend.username)
        output = client.request(command)

        if output != 'success':
            raise ErrorProcessingRequest()

    def get_followers(self, limit=-1): # GET/relations/followers/self.username:limit\0
        """
        Returns profiles of people who follow the user.

        Args:
            limit (int): the max. number of returned profiles. -1 means no limit.
        """
        command = 'GET/relations/followers/%s:%s\0' % (self.username, limit)
        output = client.request(command)

        serialized_relations = split_chunk(output, config.SERIAL_SIZE_RELATION)

        usernames = []
        for serialized_relation in serialized_relations:
            deserialized = serializers.deserialize_relation(serialized_relation)
            usernames.append(User(username=deserialized['second_username']))

        return usernames

    def get_friends(self, limit=-1): # GET/relations/friends/self.username:limit\0
        """
        Returns profiles of people who the user is following.

        Args:
            limit (int): the max. number of returned profiles. -1 means no limit.
        """
        command = 'GET/relations/friends/%s:%s\0' % (self.username, limit)
        output = client.request(command)

        serialized_relations = split_chunk(output, config.SERIAL_SIZE_RELATION)

        usernames = []
        for serialized_relation in serialized_relations:
            deserialized = serializers.deserialize_relation(serialized_relation)
            usernames.append(User(username=deserialized['second_username']))

        return usernames

class Post:
    def __init__(self, active, username, timestamp, text):
        self.active = active
        self.username = username
        self.timestamp = timestamp
        self.text = text

def split_chunk(chunk, unit_size):
    """Split string into items whose size is given by unit_size"""
    if not chunk:
        return ''

    return [chunk[i:i+unit_size] for i in range(0, len(chunk), unit_size)]
