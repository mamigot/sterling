# -*- coding: utf-8 -*-
from ..config import FIELD_SIZES


def pad(value, field_size, filler_char='\0'):
    """Pad value with as many filler_chars as field_size requires"""
    extra_count = field_size - len(value)

    if extra_count < 0:
        raise ValueError('Given value exceeds specified field_size')

    return (filler_char * extra_count) + value

def unpad(value, filler_char='\0'):
    """Remove all starting filler_chars from value"""
    bad_count = 0

    for char in value:
        if char == filler_char:
            bad_count += 1
        else:
            break

    return value[bad_count:]

def serialize_credential(active, username, password):
    """<active><username><password>"""
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')
    elif not isinstance(username, str):
        raise TypeError('"username" must be a string"')
    elif not isinstance(password, str):
        raise TypeError('"password" must be a string"')

    active = '1' if active else '0'
    username = utils.pad(username, FIELD_SIZES['username'])
    password = utils.pad(password, FIELD_SIZES['password'])

    return '%s%s%s\n' % (active, username, password)

def matches_credential(serialized, active, username, password):
    return serialized == serialize_credential(active, username, password)

def serialize_post(active, username, timestamp, text):
    """<active><username><timestamp><text>"""
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')
    elif not isinstance(username, str):
        raise TypeError('"username" must be a string"')
    elif not isinstance(text, str):
        raise TypeError('"text" must be a string"')

    active = '1' if active else '0'
    username = utils.pad(username, FIELD_SIZES['username'])
    timestamp = str(int(timestamp))
    text = utils.pad(text, FIELD_SIZES['text'])

    return '%s%s%s%s\n' % (active, username, timestamp, text)

def matches_post(serialized, active, username, timestamp, text=None):
    if text:
        return serialized == serialize_post(active, username, timestamp, text)

    exclude = -1 * (FIELD_SIZES['text'] + 1)

    return serialized[:exclude] == \
        serialize_post(active, username, timestamp, text='dummy')[:exclude]

def serialize_relation(active, first_username, direction, second_username):
    """<active><first_username><direction><second_username>"""
    if not isinstance(active, bool):
        raise TypeError('"active" must be a boolean')
    elif not [isinstance(u, str) for u in (first_username, second_username)]:
        raise TypeError('"username" fields must be strings"')
    elif direction != '>' and direction != '<':
        raise ValueError('"direction" must either be ">" or "<"')

    active = '1' if active else 0
    first_username = utils.pad(first_username, FIELD_SIZES['username'])
    second_username = utils.pad(second_username, FIELD_SIZES['username'])

    return '%s%s%s%s\n' % (active, first_username, direction, second_username)

def matches_relation(serialized, active, first_username, direction, second_username=None):
    if second_username:
        return serialized == serialize_relation(active, first_username, \
            direction, second_username)

    exclude = -1 * (FIELD_SIZES['username'] + 1)

    return serialized[:exclude] == serialize_relation(active, first_username, \
        direction, second_username='dummy')[:exclude]
