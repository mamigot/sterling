"""Read the relevant config. constants and save them to the global namespace"""
import os


config_path = os.environ['CONFIG_PATH']

with open(config_path, 'r') as f:
    for line in filter(lambda line: line.strip(), f.readlines()):
        constant, value = line.strip().split('=')

        globals()[constant] = value
