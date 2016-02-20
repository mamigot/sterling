import os
from ..config import STORAGE_ROOT_PATH, StoredFileType


def get_path(username, stored_type):
    """Gets the path of the file wherein the user's wanted data is stored.

    As ..config.py shows, each user's data is distributed across one of each
    of the StoredFileType files (there's a file for credentials, timeline
    posts, profile posts and relations --friends and followers).

    There's more than one file of each type, as ..config.py shows, so this
    function hashes the provided number to an integer and mods it by the
    total number of files.

    Combines the hashed path with the root of the stored volumes to return
    an absolute path.

    Args:
        username (str): username of the user
        stored_type (StoredFileType.*): denotes the wanted type of file
            (this is a property of StoredFileType)

    Returns:
        Absolute path to the relevant stored file.

        >> get_path('somebody', StoredFileType.credential)
        'storage/volumes/credential_0.txt'

        >> get_path('millionthuser', StoredFileType.post_timeline)
        'storage/volumes/post_timeline_1.txt'

    Raises:
        ValueError: If the value of `stored_type` is unknown
    """
    filetype, count = stored_type

    if not hasattr(StoredFileType, filetype):
        raise ValueError('Type of the file is unknown.')

    numeric_hash = sum(ord(c) for c in username)
    filenumber = numeric_hash % count

    filename = '%s_%d.txt' % (filetype, filenumber)
    return os.path.join(STORAGE_ROOT_PATH, filename)

def item_match(file_path, item_size, compare_func, compare_kwargs={}):
    """Iterates through the file backwards and returns an offset (in bytes)
    of the first matching entry.

    Iterates through the file denoted by `file_path` backwards and compares
    chunks of size `item_size` using the provided `compare_func` and
    its arguments, `compare_kwargs`. If there's a successful match, the
    iteration stops and the number of bytes from the end of the file at
    which it occurred is returned.

    Args:
        file_path (str): path to the file with wanted data.
        item_size (int): number of bytes denoting the chunks that are evaluated.
            If the relevant file contains credentials, for example, then this
            would denote the size of a single credential chunk. This only works
            as long all chunks are of a uniform size.
        compare_func (func) and compare_kwargs (dict): in order to determine
            if a chunk that is read from the file is wanted, this function can
            be called. Its first argument must be the string of size
            `item_size` that is read from the file; the remaining arguments
            (presumably used to deterine whether the read chunk is wanted)
            can be whatever. These are specified as key-value pairs in
            compare_kwargs.

    Returns:
        The number of bytes from the end of the file wherein the first
        match took place. If there was no match, then None.
    """
    file_size = os.path.getsize(file_path)

    with open(file_path, 'rb+') as f:
        read_ptr = item_size

        while abs(read_ptr) <= abs(file_size):
            f.seek(-read_ptr, os.SEEK_END)
            item = f.read(item_size).decode('utf-8')

            if compare_func(item, **compare_kwargs):
                return read_ptr

            read_ptr += item_size

    return None

def item_match_sweep(file_path, item_size, compare_func, compare_kwargs={}, limit=None):
    """Iterates through the file backwards and returns relevant entries.

    Iterates through the file denoted by `file_path` backwards and compares
    chunks of size `item_size` using the provided `compare_func` and
    its arguments, `compare_kwargs`. Every match is appended to a list of
    strings, which is returned.

    Args:
        file_path (str): path to the file with wanted data.
        item_size (int): number of bytes denoting the chunks that are evaluated.
            If the relevant file contains credentials, for example, then this
            would denote the size of a single credential chunk. This only works
            as long all chunks are of a uniform size.
        compare_func (func) and compare_kwargs (dict): in order to determine
            if a chunk that is read from the file is wanted, this function can
            be called. Its first argument must be the string of size
            `item_size` that is read from the file; the remaining arguments
            (presumably used to deterine whether the read chunk is wanted)
            can be whatever. These are specified as key-value pairs in
            compare_kwargs.
        limit (int): maximum number of matches that is acceptable. If this is
            provided, then after the list that is to be returned reaches
            this size, the iteration will stop.

    Returns:
        List of matching strings in the file.
    """
    file_size = os.path.getsize(file_path)
    items = []

    with open(file_path, 'rb+') as f:
        read_ptr = item_size

        while abs(read_ptr) <= abs(file_size):
            f.seek(-read_ptr, os.SEEK_END)
            item = f.read(item_size).decode('utf-8')

            if compare_func(item, **compare_kwargs):
                items.append(item)

                if limit and len(items) > limit:
                    return items

            read_ptr += item_size

    return items

def set_active_flag(active_flag, file_path, item_size, compare_func, compare_kwargs):
    """Iterates through the file backwards and modifies each entry's "active"
    status.

    Iterates through the file denoted by `file_path` backwards and compares
    chunks of size `item_size` using the provided `compare_func` and
    its arguments, `compare_kwargs`. Every successful match's active flag
    (a '0' or a '1' in its first byte, as .serializers.py shows) is modified
    according to the value of `active_flag`.

    Args:
        active_flag (bool): the flag to which every successful match will be set.
        file_path (str): path to the file with wanted data.
        item_size (int): number of bytes denoting the chunks that are evaluated.
            If the relevant file contains credentials, for example, then this
            would denote the size of a single credential chunk. This only works
            as long all chunks are of a uniform size.
        compare_func (func) and compare_kwargs (dict): in order to determine
            if a chunk that is read from the file is wanted, this function can
            be called. Its first argument must be the string of size
            `item_size` that is read from the file; the remaining arguments
            (presumably used to deterine whether the read chunk is wanted)
            can be whatever. These are specified as key-value pairs in
            compare_kwargs.

    Returns:
        The count of entries that were modified (i.e. the number of matches)
        in the file.
    """
    file_size = os.path.getsize(file_path)
    active_byte = ('1' if active_flag == True else '0').encode('utf-8')

    num_modified = 0

    with open(file_path, 'rb+') as f:
        read_ptr = item_size

        while abs(read_ptr) <= abs(file_size):
            f.seek(-read_ptr, os.SEEK_END)
            item = f.read(item_size).decode('utf-8')

            if compare_func(item, **compare_kwargs):
                f.seek(-read_ptr, os.SEEK_END)
                f.write(active_byte)
                f.seek(-read_ptr, os.SEEK_END)
                num_modified += 1

            read_ptr += item_size

    return num_modified
