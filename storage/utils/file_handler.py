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
