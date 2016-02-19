import os
from ..config import STORAGE_ROOT_PATH, StoredFileType


def get_path(username, stored_type):
    filetype, count = stored_type

    if not hasattr(StoredFileType, filetype):
        raise ValueError('Type of the file is unknown.')

    numeric_hash = sum(ord(c) for c in username)
    filenumber = numeric_hash % count

    filename = '%s_%d.txt' % (filetype, filenumber)
    return os.path.join(STORAGE_ROOT_PATH, filename)

def read_chunks_backwards(filepath, chunk_size):
    """Read one chunk at a time, backwards"""
    bytes_left = os.path.getsize(filepath)

    with open(filepath, 'r') as f:
        while True:
            if bytes_left > 0:
                f.seek(-1 * bytes_left, os.SEEK_END)
                yield f.read(chunk_size)

                bytes_left -= min(bytes_left, chunk_size)
            else:
                break

def read_entries_backwards(filepath, entry_size):
    desired_chunk = 4096
    chunk_size = entry_size

    while (chunk_size + entry_size) <= desired_chunk:
        chunk_size += entry_size

    entries_for_chunk = []

    for chunk in read_chunks_backwards(filepath, chunk_size):
        for i in range(0, chunk_size, entry_size):
            entry = chunk[i:(i + entry_size)]

            if entry.strip():
                entries_for_chunk.append(entry)

    return reversed(entries_for_chunk)
