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

def item_match(file_path, item_size, compare_func, compare_kwargs={}):
    file_size = os.path.getsize(file_path)

    with open(file_path, 'rb+') as f:
        read_ptr = item_size

        while abs(read_ptr) <= abs(file_size):
            f.seek(-read_ptr, os.SEEK_END)
            item = f.read(item_size).decode('utf-8')

            if compare_func(item, **compare_kwargs):
                return -read_ptr

            read_ptr += item_size

def item_match_sweep(file_path, item_size, compare_func, compare_kwargs={}, limit=None):
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
