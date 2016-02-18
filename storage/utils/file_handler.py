import os
from ..config import STORAGE_ROOT_PATH, STORED_FILE_TYPES


def stored_filename(username, filetype):
    if filetype not in STORED_FILE_TYPES:
        raise ValueError('Type of the file is unknown.')

    numeric_hash = sum(ord(c) for c in username)
    filenumber = numeric_hash % STORED_FILE_TYPES[filetype]

    return '%s_%d.txt' % (filetype, filenumber)

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

def read_lines_backwards(filepath, line_size):
    desired_chunk = 4096
    chunk_size = line_size

    while (chunk_size + line_size) <= desired_chunk:
        chunk_size += line_size

    for block in read_chunks_backwards(filepath, chunk_size):
        yield reverse(block.split('\n'))

"""
timeline = []
reader = utils.read_lines_backwards('test.txt', line_size=Post.BYTES_PER_POST)

for serialized_posts in reader:
    for serialized_post in serialized_posts:
        if len(timeline) < limit:
            post = Post.deserialize(serialized_post)

            if post.active and post.post_type == 't' and post.username == self.username:
                timeline.append(post)
        else:
            return timeline
"""
