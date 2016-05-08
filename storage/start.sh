#!/bin/bash
#
# Run on the server machine
#

#############################
# System Requirements (!!!) #
#############################

# Version 4.9 or greater of g++
echo "CHECK THAT YOU ARE USING VERSION 4.9 OF g++ OR GREATER USING 'g++ -version'."
echo -e "IF YOU ARE USING AN OLDER VERSION, THE APPLICATION WILL NOT COMPILE.\n"

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

# Directory wherein the application data is stored for this RM
export STORAGE_FILES_PATH=$PROJECT_ROOT/volumes/$UREQUEST_PORT

# Create it if it doesn't exist
if [ -z "$(ls $STORAGE_FILES_PATH 2>/dev/null)" ]; then
  echo "Creating directory to hold the files... $STORAGE_FILES_PATH"
  mkdir $STORAGE_FILES_PATH
fi

#########################
# Networking parameters #
#########################

# Ports of all replica managers in the network
export TOPOLOGY_PATH=$PROJECT_ROOT/topology.txt

# Max. size of the buffer used for communications across the network
export DATASERVER_BUFFSIZE=8192

if [ -n "$UREQUEST_PORT" ]; then echo "Using UREQUEST_PORT=$UREQUEST_PORT."
else echo "Set env. var UREQUEST_PORT." return
fi

if [ -n "$IREQUEST_PORT" ]; then echo "Using IREQUEST_PORT=$IREQUEST_PORT."
else echo "Set env. var IREQUEST_PORT." return
fi

echo

# Kill processes that may be using the ports (potentially dangerous; maybe not)
#kill -9 $(lsof -ti tcp:$UREQUEST_PORT) 2>/dev/null
#kill -9 $(lsof -ti tcp:$IREQUEST_PORT) 2>/dev/null

#########################################################
# Launch the server and start listening for connections #
#########################################################

make clean

make

./bin/runner
