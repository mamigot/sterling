import os, subprocess


STORAGE_ROOT_PATH = os.path.join(os.path.dirname(__file__), 'volumes')

# Optimize these values for the application
# (more files --> faster reads and writes but more wasted space)
class StoredFileType:
    credential = ('credential', 2)
    post_timeline = ('post_timeline', 2)
    post_profile = ('post_profile', 2)
    relation = ('relation', 2)

    @classmethod
    def get_types(cls):
        return filter(lambda prop: type(prop) is tuple, cls.__dict__.values())

class UserFieldSizes:
    username = 20
    password = 20
    text = 140


def initiate_storage():
    for filetype, count in StoredFileType.get_types():

        for idx in range(count):
            filename = '%s_%d.txt' % (filetype, idx)
            filepath = os.path.join(STORAGE_ROOT_PATH, filename)

            subprocess.call(['touch', filepath])


if __name__ == '__main__':
    initiate_storage()
