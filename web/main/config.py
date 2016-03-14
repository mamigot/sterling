"""Read the relevant config. constants and save them to the global namespace"""
import os


config_path = os.environ['CONFIG_PATH']

with open(config_path, 'r') as f:
    for line in f.readlines():
        # Ignore empty lines as well as those that start with '#' (comments)
        if line.strip() and not line.startswith('#'):
            constant, value = line.strip().split('=')
            globals()[constant] = int(value)
