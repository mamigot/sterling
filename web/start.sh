#!/bin/bash
#
# Run on the client machine
#

#######################################################
# Uncomment the following if your environment differs #
#######################################################

#pip install -r requirements.txt

#####################################################################
# Parameters to communicate with the data server (change as needed) #
#####################################################################

export DATASERVER_HOST="localhost"

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

###########################################
# Launch the app (http://127.0.0.1:5000/) #
###########################################

PYTHON_VERSION=python3
APP=$PROJECT_ROOT/main/app.py

$PYTHON_VERSION $APP
