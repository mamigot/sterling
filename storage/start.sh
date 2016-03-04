#!/bin/bash

# TODO: get absolute path
PROJECT_ROOT=$PWD/storage/

# Application configuration variables
CONFIG_PATH=$PROJECT_ROOT/config

############################################
# Create storage files if they don't exist #
############################################
STORAGE_FILES=$PROJECT_ROOT/main/volumes/

if [ -z "$(ls $STORAGE_FILES 2>/dev/null)" ]; then
  # Create the directory holding the files
  mkdir $STORAGE_FILES

  # Create the text files that are required to store users' data
  $PYTHON_VERSION storage/config.py
fi

###################################################
# Start listening for connections from the client #
###################################################
