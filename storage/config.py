import os, subprocess


FILE_COUNTS = {
    'login': 5,
    'posts': 5,
    'followers': 5
}

FIELD_SIZES = {
    'post_text': 140,
    'username': 20,
    'password': 20
}

def initiate_storage():
    storage_abspath = os.path.join(os.path.dirname(__file__), 'volumes')

    for filetype, count in FILE_COUNTS.items():
        for idx in range(count):
            filename = '%s_%d.txt' % (filetype, idx)
            filepath = os.path.join(storage_abspath, filename)

            subprocess.call(['touch', filepath])


if __name__ == '__main__':
    initiate_storage()
