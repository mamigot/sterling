import unittest
import time
import functools
from main import config, serialization as ser


class TestPadding(unittest.TestCase):
    def test_pad(self):
        value = ser.pad(value='lol', field_size=6, filler_char='$')
        self.assertEqual(value, '$$$lol')

    def test_unpad(self):
        value = ser.unpad(value='$$$lol', filler_char='$')
        self.assertEqual(value, 'lol')

class TestCredential(unittest.TestCase):
    active = True
    username = 'yaBoy211'
    password = 'yaBoysP@ssword'
    serialized = '1~~~~~~~~~~~~yaBoy211~~~~~~yaBoysP@ssword'

    def test_serialize(self):
        value = ser.serialize_credential(self.active, self.username, self.password)

        # Right length?
        self.assertTrue(len(value) == config.SERIAL_SIZE_CREDENTIAL)

        # All fields fully contained?
        active = '1' if True else '0'
        self.assertTrue(all(
            field in value for field in [active, self.username, self.password]
        ))

    def test_deserialize(self):
        values = ser.deserialize_credential(self.serialized)

        self.assertTrue(values.get('active') == True)
        self.assertTrue(values.get('username') == 'yaBoy211')
        self.assertTrue(values.get('password') == 'yaBoysP@ssword')

    def test_match(self):
        # All asserts will have to take the serialized string as the first arg
        matcher = functools.partial(ser.matches_credential, self.serialized)

        # Some random permutations of the testing params
        self.assertTrue(matcher(active=True))
        self.assertFalse(matcher(username='yaaaaaBoy211'))
        self.assertFalse(matcher(password='NOTMYp@ssword'))
        self.assertTrue(matcher(active=True, username='yaBoy211'))

class TestRelation(unittest.TestCase):
    active = True
    first_username = 'yaBoy211'
    direction = '>'
    second_username = 'brotasaurus'
    serialized = '1~~~~~~~~~~~~yaBoy211>~~~~~~~~~brotasaurus'

    def test_serialize(self):
        value = ser.serialize_relation(
            self.active,
            self.first_username,
            self.direction,
            self.second_username
        )

        # Right length?
        self.assertTrue(len(value) == config.SERIAL_SIZE_RELATION)

        # All fields fully contained?
        active = '1' if True else '0'
        for field in [active, self.first_username, self.direction, self.second_username]:
            self.assertTrue(field in value)

    def test_deserialize(self):
        values = ser.deserialize_relation(self.serialized)

        self.assertTrue(values.get('active') == True)
        self.assertTrue(values.get('first_username') == 'yaBoy211')
        self.assertTrue(values.get('direction') == '>')
        self.assertTrue(values.get('second_username') == 'brotasaurus')

    def test_match(self):
        # All asserts will have to take the serialized string as the first arg
        matcher = functools.partial(ser.matches_relation, self.serialized)

        # Some random permutations of the testing params
        self.assertTrue(matcher(active=True))
        self.assertFalse(matcher(first_username='brotato'))
        self.assertFalse(matcher(second_username='totes'))
        self.assertTrue(matcher(active=True, direction='>'))

class TestProfilePost(unittest.TestCase):
    active = False
    username = 'yooohoo'
    timestamp = int(time.time())
    text = 'ahahahhahaha hilarIoU$'
    serialized = '0~~~~~~~~~~~~~yooohoo1457969970~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ahahahhahaha hilarIoU$'

    def test_serialize(self):
        value = ser.serialize_profile_post(
            self.active,
            self.username,
            self.timestamp,
            self.text
        )

        # Right length?
        self.assertTrue(len(value) == config.SERIAL_SIZE_PROFILE_POST)

        # All fields fully contained?
        active = '1' if True else '0'
        timestamp = str(self.timestamp)
        for field in [active, self.username, timestamp, self.text]:
            self.assertTrue(field in value)

    def test_deserialize(self):
        values = ser.deserialize_profile_post(self.serialized)

        self.assertTrue(values.get('active') == False)
        self.assertTrue(values.get('username') == 'yooohoo')
        self.assertTrue(len(values.get('timestamp')) == config.FIELD_SIZE_TIMESTAMP)
        self.assertTrue(values.get('text') == 'ahahahhahaha hilarIoU$')

    def test_match(self):
        # All asserts will have to take the serialized string as the first arg
        matcher = functools.partial(ser.matches_profile_post, self.serialized)

        # Some random permutations of the testing params
        self.assertTrue(matcher(active=False))
        self.assertFalse(matcher(username='brotato'))
        self.assertFalse(matcher(timestamp='122'))
        self.assertTrue(matcher(active=False, text='ahahahhahaha hilarIoU$'))

class TestTimelinePost(unittest.TestCase):
    active = True
    username = 'yooohoo'
    author = 'TheMonkeyCarl'
    timestamp = int(time.time())
    text = 'sup its carl'
    serialized = '1~~~~~~~~~~~~~yooohoo~~~~~~~TheMonkeyCarl1457972121~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~sup its carl'

    def test_serialize(self):
        value = ser.serialize_timeline_post(
            self.active,
            self.username,
            self.author,
            self.timestamp,
            self.text
        )

        # Right length?
        self.assertTrue(len(value) == config.SERIAL_SIZE_TIMELINE_POST)

        # All fields fully contained?
        active = '1' if True else '0'
        timestamp = str(self.timestamp)
        for field in [active, self.username, self.author, timestamp, self.text]:
            self.assertTrue(field in value)

    def test_deserialize(self):
        values = ser.deserialize_timeline_post(self.serialized)

        self.assertTrue(values.get('active') == True)
        self.assertTrue(values.get('username') == 'yooohoo')
        self.assertTrue(values.get('author') == 'TheMonkeyCarl')
        self.assertTrue(len(values.get('timestamp')) == config.FIELD_SIZE_TIMESTAMP)
        self.assertTrue(values.get('text') == 'sup its carl')

    def test_match(self):
        # All asserts will have to take the serialized string as the first arg
        matcher = functools.partial(ser.matches_timeline_post, self.serialized)

        # Some random permutations of the testing params
        self.assertTrue(matcher(author='TheMonkeyCarl'))
        self.assertTrue(matcher(active=True, text='sup its carl'))
        self.assertFalse(matcher(active=False))
        self.assertFalse(matcher(timestamp='122'))
        self.assertFalse(matcher(username='brotato'))


if __name__ == '__main__':
    unittest.main()
