#!/bin/bash
#
# Run on the server machine
#

########################################################################
# Parameters to communicate with the client servers (change as needed) #
########################################################################

export DATASERVER_PORT=13002

export DATASERVER_BUFFSIZE=8192

########################################
# Application configuration parameters #
########################################

# The root of the project is the root of this script
# http://stackoverflow.com/a/246128/2708484
PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Make all root-level modules of the app accessible
export PATH=$PATH:$PROJECT_ROOT/main

# Application configuration variables
export CONFIG_PATH=$PROJECT_ROOT/config.txt

# Directory wherein the application data is stored
export STORAGE_FILES_PATH=$PROJECT_ROOT/volumes/

# Create it if it doesn't exist
if [ -z "$(ls $STORAGE_FILES_PATH 2>/dev/null)" ]; then
  echo "Creating directory to hold the files... $STORAGE_FILES_PATH"
  mkdir $STORAGE_FILES_PATH
fi

#########################################################
# Launch the server and start listening for connections #
#########################################################

make clean

make

./bin/runner
