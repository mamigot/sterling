import unittest
import os
import subprocess


config_constants = {}

def serialized_item_size(item_type):
    if not config_constants:
        with open(os.environ['CONFIG_PATH'], 'r') as f:
            for line in f.readlines():
                # Ignore empty lines as well as those that start with '#' (comments)
                if line.strip() and not line.startswith('#'):
                    constant, value = line.strip().split('=')
                    config_constants[constant] = int(value)

    return config_constants['SERIAL_SIZE_' + item_type]

def filetype_from_filename(filename):
    return filename[:filename.rfind('_', 0, -1)]

class TestDataIntegrity(unittest.TestCase):
    def test_integrity(self):
        volumes_dir = os.environ['STORAGE_FILES_PATH']

        process_output = subprocess.Popen(
            ['ls', volumes_dir],
            stdout=subprocess.PIPE
        ).communicate()

        data_files = process_output[0].decode('utf-8').split('\n')

        for filename in filter(lambda x:x, data_files):
            data_file = os.path.join(volumes_dir, filename)

            with open(data_file, 'r') as f:
                content = ''.join(f.readlines())
                if content:
                    item_size = serialized_item_size(filetype_from_filename(filename))

                    # The size of the file must be a multiple of the item
                    print('...testing %s' % data_file)
                    self.assertEqual(len(content) % item_size, 0)


if __name__ == '__main__':
    unittest.main()
