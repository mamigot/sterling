# -*- coding: utf-8 -*-
from ..config import UserFieldSizes


class SerializedSizeBytes:
    credential = 1 + UserFieldSizes.username + UserFieldSizes.password
    post = 1 + UserFieldSizes.username + 10 + UserFieldSizes.text
    relation = 1 + UserFieldSizes.username + 1 + UserFieldSizes.username

class SerializedRelationBounds:
    active = (0,)
    first_username = (1, UserFieldSizes.username + 1)
    direction = (UserFieldSizes.username + 1,)
    second_username = (
        UserFieldSizes.username + 2,
        2 * UserFieldSizes.username + 2
    )

class SerializedProfilePostBounds:
    active = (0,)
    username = (1, UserFieldSizes.username + 1)
    timestamp = (
        UserFieldSizes.username + 1,
        UserFieldSizes.username + 11
    )
    text = (
        UserFieldSizes.username + 11,
        UserFieldSizes.username + 11 + UserFieldSizes.text
    )

class SerializedTimelinePostBounds:
    active = (0,)
    username = (1, UserFieldSizes.username + 1)
    author = (
        UserFieldSizes.username + 1,
        2 * UserFieldSizes.username + 1
    )
    timestamp = (
        2 * UserFieldSizes.username + 1,
        2 * UserFieldSizes.username + 11
    )
    text = (
        2 * UserFieldSizes.username + 11,
        2 * UserFieldSizes.username + 11 + UserFieldSizes.text
    )

def serialize_credential(active, username, password):
    """<active><username><password>"""
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')

    elif not isinstance(username, str):
        raise TypeError('"username" must be a string"')

    elif len(username) > UserFieldSizes.username:
        raise ValueError('max. size of username = %d' % UserFieldSizes.username)

    elif not isinstance(password, str):
        raise TypeError('"password" must be a string"')

    elif len(password) > UserFieldSizes.password:
        raise ValueError('max. size of password = %d' % UserFieldSizes.password)

    active = '1' if active else '0'
    username = pad(username, UserFieldSizes.username)
    password = pad(password, UserFieldSizes.password)

    return '%s%s%s' % (active, username, password)

def matches_credential(serialized, username, password, active=None):
    if active is None:
        # Pass a dummy value for "active" and ignore that byte when matching
        return serialized[1:] == serialize_credential(True, username, password)[1:]

    return serialized == (serialize_credential(active, username, password))

def serialize_profile_post(active, username, timestamp, text):
    """<active><username><timestamp><text>"""
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')

    elif not isinstance(username, str):
        raise TypeError('"username" must be a string"')

    elif len(username) > UserFieldSizes.username:
        raise ValueError('max. size of username = %d' % UserFieldSizes.username)

    elif not isinstance(text, str):
        raise TypeError('"text" must be a string"')

    elif len(text) > UserFieldSizes.text:
        raise ValueError('max. size of text = %d' % UserFieldSizes.text)

    active = '1' if active else '0'
    username = pad(username, UserFieldSizes.username)
    timestamp = str(int(timestamp))
    text = pad(text, UserFieldSizes.text)

    return '%s%s%s%s' % (active, username, timestamp, text)

def deserialize_profile_post(serialized):
    Bounds = SerializedProfilePostBounds

    return dict(
        active=True if serialized[Bounds.active[0]] == '1' else False,
        username=unpad(serialized[Bounds.username[0]:Bounds.username[1]]),
        timestamp=serialized[Bounds.timestamp[0]:Bounds.timestamp[1]],
        text=unpad(serialized[Bounds.text[0]:Bounds.text[1]])
    )

def matches_profile_post(serialized, active=None, username=None, timestamp=None, text=None):
    Bounds = SerializedProfilePostBounds

    if active is not None:
         if (active == True and serialized[Bounds.active[0]] != '1') or \
            (active == False and serialized[Bounds.active[0]] != '0'):
            return False

    if username is not None:
        if pad(username, UserFieldSizes.username) != \
            serialized[Bounds.username[0]:Bounds.username[1]]:
            return False

    if timestamp is not None:
        if str(int(timestamp)) != \
            serialized[Bounds.timestamp[0]:Bounds.timestamp[1]]:
            return False

    if text is not None:
        if pad(text, UserFieldSizes.text) != serialized[Bounds.text[0]:Bounds.text[1]]:
            return False

    return True

def serialize_timeline_post(active, username, author, timestamp, text):
    """<active><username><author><timestamp><text>"""
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')

    elif not isinstance(username, str):
        raise TypeError('"username" must be a string"')

    elif len(username) > UserFieldSizes.username:
        raise ValueError('max. size of username = %d' % UserFieldSizes.username)

    elif not isinstance(author, str):
        raise TypeError('"author" must be a string"')

    elif len(author) > UserFieldSizes.username:
        raise ValueError('max. size of author = %d' % UserFieldSizes.username)

    elif not isinstance(text, str):
        raise TypeError('"text" must be a string"')

    elif len(text) > UserFieldSizes.text:
        raise ValueError('max. size of text = %d' % UserFieldSizes.text)

    active = '1' if active else '0'
    username = pad(username, UserFieldSizes.username)
    author = pad(author, UserFieldSizes.username)
    timestamp = str(int(timestamp))
    text = pad(text, UserFieldSizes.text)

    return '%s%s%s%s%s' % (active, username, author, timestamp, text)

def deserialize_timeline_post(serialized):
    Bounds = SerializedTimelinePostBounds

    return dict(
        active=True if serialized[Bounds.active[0]] == '1' else False,
        username=unpad(serialized[Bounds.username[0]:Bounds.username[1]]),
        author=unpad(serialized[Bounds.author[0]:Bounds.author[1]]),
        timestamp=serialized[Bounds.timestamp[0]:Bounds.timestamp[1]],
        text=unpad(serialized[Bounds.text[0]:Bounds.text[1]])
    )

def matches_timeline_post(serialized, active=None, username=None, author=None,
    timestamp=None, text=None):

    Bounds = SerializedTimelinePostBounds

    if active is not None:
         if (active == True and serialized[Bounds.active[0]] != '1') or \
            (active == False and serialized[Bounds.active[0]] != '0'):
            return False

    if username is not None:
        if pad(username, UserFieldSizes.username) != \
            serialized[Bounds.username[0]:Bounds.username[1]]:
            return False

    if author is not None:
        if pad(author, UserFieldSizes.username) != \
            serialized[Bounds.author[0]:Bounds.author[1]]:
            return False

    if timestamp is not None:
        if str(int(timestamp)) != \
            serialized[Bounds.timestamp[0]:Bounds.timestamp[1]]:
            return False

    if text is not None:
        if pad(text, UserFieldSizes.text) != serialized[Bounds.text[0]:Bounds.text[1]]:
            return False

    return True

def serialize_relation(active, first_username, direction, second_username):
    """<active><first_username><direction><second_username>"""
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')

    elif not (isinstance(u, str) for u in (first_username, second_username)):
        raise TypeError('"username" fields must be strings"')

    elif len(first_username) + len(second_username) > 2 * UserFieldSizes.username:
        raise ValueError('max. size of username = %d' % UserFieldSizes.username)

    elif direction != '>' and direction != '<':
        raise ValueError('"direction" must either be ">" or "<"')

    active = '1' if active else 0
    first_username = pad(first_username, UserFieldSizes.username)
    second_username = pad(second_username, UserFieldSizes.username)

    return '%s%s%s%s' % (active, first_username, direction, second_username)

def deserialize_relation(serialized):
    SRB = SerializedRelationBounds

    return dict(
        active=True if serialized[SRB.active[0]] == '1' else False,
        first_username=unpad(serialized[SRB.first_username[0]:SRB.first_username[1]]),
        direction=serialized[SRB.direction[0]],
        second_username=unpad(serialized[SRB.second_username[0]:SRB.second_username[1]])
    )

def matches_relation(serialized, active, first_username, direction, second_username=None):
    if second_username:
        return serialized == serialize_relation(active, first_username, \
            direction, second_username)

    exclude = -1 * (UserFieldSizes.username + 1)

    return serialized[:exclude] == serialize_relation(active, first_username, \
        direction, second_username='dummy')[:exclude]

def pad(value, field_size, filler_char='~'):
    """Pad value with as many filler_chars as field_size requires"""
    extra_count = field_size - len(value)

    if extra_count < 0:
        raise ValueError('Given value exceeds specified field_size')

    return (filler_char * extra_count) + value

def unpad(value, filler_char='~'):
    """Remove all starting filler_chars from value"""
    bad_count = 0

    for char in value:
        if char == filler_char:
            bad_count += 1
        else:
            break

    return value[bad_count:]
