import os, subprocess


STORAGE_ROOT_PATH = os.path.join(os.path.dirname(__file__), 'volumes')

# Optimize these values for the application
# (more files --> faster reads and writes but more wasted space)
STORED_FILE_TYPES = {
    'CREDENTIALS': 2,
    'POST_TIMELINE': 2,
    'POST_PROFILE': 2,
    'RELATION': 2
}

FIELD_SIZES = {
    'USERNAME': 20,
    'PASSWORD': 20,
    'TEXT': 140,
}

def initiate_storage():
    for filetype, count in FILE_COUNTS.items():
        for idx in range(count):
            filename = '%s_%d.txt' % (filetype, idx)
            filepath = os.path.join(STORAGE_ROOT_PATH, filename)

            subprocess.call(['touch', filepath])


if __name__ == '__main__':
    initiate_storage()
