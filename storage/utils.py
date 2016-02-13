import os


def pad(value, field_size, filler_char='\0'):
    """Pad value with as many filler_chars as field_size requires"""
    extra_count = field_size - len(value)

    if extra_count < 0:
        raise ValueError('Given value exceeds specified field_size')

    return (filler_char * extra_count) + value

def unpad(value, filler_char='\0'):
    """Remove all starting filler_chars from value"""
    bad_count = 0

    for char in value:
        if char == filler_char:
            bad_count += 1
        else:
            break

    return value[bad_count:]

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
