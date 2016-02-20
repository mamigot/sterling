import os, subprocess


# Absolute path of the root directory of the storage of the application
STORAGE_ROOT_PATH = os.path.join(os.path.dirname(__file__), 'volumes')

# Optimize these values for the application
# (more files --> faster reads and writes but more wasted space)
class StoredFileType:
    """All users' data is divided into the following types of files (this
    class' parameters). Each attribute contains the number of files that exist
    for each type.
    """
    credential = ('credential', 2)
    post_timeline = ('post_timeline', 10)
    post_profile = ('post_profile', 10)
    relation = ('relation', 5)

    @classmethod
    def get_types(cls):
        """Return the tuple attributes of this class that denote the types
        of files as well as their counts.
        """
        return filter(lambda prop: type(prop) is tuple, cls.__dict__.values())

class UserFieldSizes:
    """The maximum size for each field that the user is responsible for."""
    username = 20
    password = 20
    text = 140


def initiate_storage():
    """Create the storage files that StoredFileType specifies."""
    for filetype, count in StoredFileType.get_types():

        for idx in range(count):
            filename = '%s_%d.txt' % (filetype, idx)
            filepath = os.path.join(STORAGE_ROOT_PATH, filename)

            subprocess.call(['touch', filepath])


if __name__ == '__main__':
    initiate_storage()
