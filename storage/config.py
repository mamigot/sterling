import os, subprocess


STORAGE_ROOT_PATH = os.path.join(os.path.dirname(__file__), 'volumes')

# Optimize these values for the application
# (more files --> faster reads and writes but more wasted space)
STORED_FILE_TYPES = {
    'user_accounts': 2,
    'posts_timeline': 2,
    'posts_profile':2,
    'relations': 2
}

FIELD_SIZES = {
    'post_text': 140,
    'username': 20,
    'password': 20
}

def initiate_storage():
    for filetype, count in FILE_COUNTS.items():
        for idx in range(count):
            filename = '%s_%d.txt' % (filetype, idx)
            filepath = os.path.join(STORAGE_ROOT_PATH, filename)

            subprocess.call(['touch', filepath])


if __name__ == '__main__':
    initiate_storage()
