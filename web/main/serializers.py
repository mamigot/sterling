import config


def serialize_credential(active, username, password):
    """Combine the provided parameters into the following format, which is
    returned as a string:

    <active><username><password>

    Args:
        active (bool): True if the credential is valid, False otherwise
        username (str): username of the user
        password (str): password of the user
    """
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')

    elif not isinstance(username, str):
        raise TypeError('"username" must be a string"')

    elif len(username) > config.FIELD_SIZE_USERNAME:
        raise ValueError('max. size of username = %d' % config.FIELD_SIZE_USERNAME)

    elif not isinstance(password, str):
        raise TypeError('"password" must be a string"')

    elif len(password) > config.FIELD_SIZE_PASSWORD:
        raise ValueError('max. size of password = %d' % config.FIELD_SIZE_PASSWORD)

    active = '1' if active else '0'
    username = pad(username, config.FIELD_SIZE_USERNAME)
    password = pad(password, config.FIELD_SIZE_PASSWORD)

    return '%s%s%s' % (active, username, password)

def deserialize_credential(serialized):
    """Parse the serialized relation and build a dictionary with its parameters."""
    active = extract_field(serialized, 'credential', 'active')
    username = extract_field(serialized, 'credential', 'username')
    password = extract_field(serialized, 'credential', 'password')

    return dict(
        active=True if active == '1' else False,
        username=unpad(username),
        password=unpad(password)
    )

def matches_credential(serialized, active=None, username=None, password=None):
    """Determine whether the provided parameters match the serialized
    credential.
    """
    if active is not None:
        ser = extract_field(serialized, 'credential', 'active')
        if(active == True and ser != '1' or active == False and ser != '0'):
            return False

    if username is not None:
        ser = extract_field(serialized, 'credential', 'username')
        if pad(username, config.FIELD_SIZE_USERNAME) != ser:
            return False

    if password is not None:
        ser = extract_field(serialized, 'credential', 'password')
        if pad(password, config.FIELD_SIZE_PASSWORD) != ser:
            return False

    return True

def serialize_relation(active, first_username, direction, second_username):
    """Combine the provided parameters into the following format, which is
    returned as a string:

    <active><first_username><direction><second_username>

    This is like a directed edge on a graph of relationships, where its direction
    is determined by the `direction` param.: either '>' (forward) or '<' (backward).

    Args:
        active (bool): True if the credential is valid, False otherwise
        first_username (str): user on whose timeline this post belongs
        direction (str): author of the post
        second_username (str): user on whose timeline this post belongs
    """
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')

    elif not (isinstance(u, str) for u in (first_username, second_username)):
        raise TypeError('"username" fields must be strings"')

    elif len(first_username) + len(second_username) > 2 * config.FIELD_SIZE_USERNAME:
        raise ValueError('max. size of username = %d' % config.FIELD_SIZE_USERNAME)

    elif direction != '>' and direction != '<':
        raise ValueError('"direction" must either be ">" or "<"')

    active = '1' if active else 0
    first_username = pad(first_username, config.FIELD_SIZE_USERNAME)
    second_username = pad(second_username, config.FIELD_SIZE_USERNAME)

    return '%s%s%s%s' % (active, first_username, direction, second_username)

def deserialize_relation(serialized):
    """Parse the serialized relation and build a dictionary with its parameters."""
    active = extract_field(serialized, 'relation', 'active')
    first_username = extract_field(serialized, 'relation', 'first_username')
    direction = extract_field(serialized, 'relation', 'direction')
    second_username = extract_field(serialized, 'relation', 'second_username')

    return dict(
        active=True if active == '1' else False,
        first_username=unpad(first_username),
        direction=direction,
        second_username=unpad(second_username)
    )

def matches_relation(serialized, active=None, first_username=None, direction=None, second_username=None):
    """Determine whether the provided parameters match the serialized
    relation.
    """
    if active is not None:
        ser = extract_field(serialized, 'relation', 'active')
        if(active == True and ser != '1' or active == False and ser != '0'):
            return False

    if first_username is not None:
        ser = extract_field(serialized, 'relation', 'first_username')
        if pad(first_username, config.FIELD_SIZE_USERNAME) != ser:
            return False

    if direction is not None:
        ser = extract_field(serialized, 'relation', 'direction')
        if direction != ser:
            return False

    if second_username is not None:
        ser = extract_field(serialized, 'relation', 'second_username')
        if pad(second_username, config.FIELD_SIZE_USERNAME) != ser:
            return False

    return True

def serialize_profile_post(active, username, timestamp, text):
    """Combine the provided parameters into the following format, which is
    returned as a string:

    <active><username><timestamp><text>

    Args:
        active (bool): True if the credential is valid, False otherwise
        username (str): author of the post
        timestamp (time.time): timestamp of the post
        text (str): text of the post
    """
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')

    elif not isinstance(username, str):
        raise TypeError('"username" must be a string"')

    elif len(username) > config.FIELD_SIZE_USERNAME:
        raise ValueError('max. size of username = %d' % config.FIELD_SIZE_USERNAME)

    elif not isinstance(text, str):
        raise TypeError('"text" must be a string"')

    elif len(text) > config.FIELD_SIZE_TEXT:
        raise ValueError('max. size of text = %d' % config.FIELD_SIZE_TEXT)

    active = '1' if active else '0'
    username = pad(username, config.FIELD_SIZE_USERNAME)
    timestamp = str(int(timestamp))
    text = pad(text, config.FIELD_SIZE_TEXT)

    return '%s%s%s%s' % (active, username, timestamp, text)

def deserialize_profile_post(serialized):
    """Parse the serialized profile post and build a dictionary with its parameters."""
    active = extract_field(serialized, 'profile_post', 'active')
    username = extract_field(serialized, 'profile_post', 'username')
    timestamp = extract_field(serialized, 'profile_post', 'timestamp')
    text = extract_field(serialized, 'profile_post', 'text')

    return dict(
        active=True if active == '1' else False,
        username=unpad(username),
        timestamp=timestamp,
        text=unpad(text)
    )

def matches_profile_post(serialized, active=None, username=None, timestamp=None, text=None):
    """Determine whether the provided parameters match the serialized profile post."""
    if active is not None:
        ser = extract_field(serialized, 'profile_post', 'active')
        if(active == True and ser != '1' or active == False and ser != '0'):
            return False

    if username is not None:
        ser = extract_field(serialized, 'profile_post', 'username')
        if pad(username, config.FIELD_SIZE_USERNAME) != ser:
            return False

    if timestamp is not None:
        ser = extract_field(serialized, 'profile_post', 'timestamp')
        if str(int(timestamp)) != ser:
            return False

    if text is not None:
        ser = extract_field(serialized, 'profile_post', 'text')
        if pad(text, config.FIELD_SIZE_TEXT) != ser:
            return False

    return True

def serialize_timeline_post(active, username, author, timestamp, text):
    """Combine the provided parameters into the following format, which is
    returned as a string:

    <active><username><author><timestamp><text>

    Args:
        active (bool): True if the credential is valid, False otherwise
        username (str): user on whose timeline this post belongs
        author (str): author of the post
        timestamp (time.time): timestamp of the post
        text (str): text of the post
    """
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')

    elif not isinstance(username, str):
        raise TypeError('"username" must be a string"')

    elif len(username) > config.FIELD_SIZE_USERNAME:
        raise ValueError('max. size of username = %d' % config.FIELD_SIZE_USERNAME)

    elif not isinstance(author, str):
        raise TypeError('"author" must be a string"')

    elif len(author) > config.FIELD_SIZE_USERNAME:
        raise ValueError('max. size of author = %d' % config.FIELD_SIZE_USERNAME)

    elif not isinstance(text, str):
        raise TypeError('"text" must be a string"')

    elif len(text) > config.FIELD_SIZE_TEXT:
        raise ValueError('max. size of text = %d' % config.FIELD_SIZE_TEXT)

    active = '1' if active else '0'
    username = pad(username, config.FIELD_SIZE_USERNAME)
    author = pad(author, config.FIELD_SIZE_USERNAME)
    timestamp = str(int(timestamp))
    text = pad(text, config.FIELD_SIZE_TEXT)

    return '%s%s%s%s%s' % (active, username, author, timestamp, text)

def deserialize_timeline_post(serialized):
    """Parse the serialized timeline post and build a dictionary with its parameters."""
    active = extract_field(serialized, 'timeline_post', 'active')
    username = extract_field(serialized, 'timeline_post', 'username')
    author = extract_field(serialized, 'timeline_post', 'author')
    timestamp = extract_field(serialized, 'timeline_post', 'timestamp')
    text = extract_field(serialized, 'timeline_post', 'text')

    return dict(
        active=True if active == '1' else False,
        username=unpad(username),
        author=unpad(author),
        timestamp=timestamp,
        text=unpad(text)
    )

def matches_timeline_post(serialized, active=None, username=None, author=None,
    timestamp=None, text=None):
    """Determine whether the provided parameters match the serialized
    timeline post.
    """
    if active is not None:
        ser = extract_field(serialized, 'timeline_post', 'active')
        if(active == True and ser != '1' or active == False and ser != '0'):
            return False

    if username is not None:
        ser = extract_field(serialized, 'timeline_post', 'username')
        if pad(username, config.FIELD_SIZE_USERNAME) != ser:
            return False

    if author is not None:
        ser = extract_field(serialized, 'timeline_post', 'author')
        if pad(author, config.FIELD_SIZE_USERNAME) != ser:
            return False

    if timestamp is not None:
        ser = extract_field(serialized, 'timeline_post', 'timestamp')
        if str(int(timestamp)) != ser:
            return False

    if text is not None:
        ser = extract_field(serialized, 'timeline_post', 'text')
        if pad(text, config.FIELD_SIZE_TEXT) != ser:
            return False

    return True

def extract_field(serialized, data_type, field_type):
    # Extract start and end bounds based on the type of the data and field
    # The constants with the index information are provided by the config module

    # Narrow down the config constants to the relevant ones
    wanted = 'SERIAL_%s_%s' % (data_type.upper(), field_type.upper())
    indexes = {var:getattr(config, var) for var in dir(config) if var.startswith(wanted)}

    if not indexes:
        raise ValueError('No known constants for (%s, %s)' % (data_type, field_type))

    start, end = indexes['%s_START' % wanted], indexes['%s_END' % wanted]
    return serialized[start:end]

def pad(value, field_size, filler_char='~'):
    """Pad value with as many filler_chars as field_size requires"""
    return value.rjust(field_size, filler_char)

def unpad(value, filler_char='~'):
    """Remove all starting filler_chars from value"""
    return value.lstrip(filler_char)
