#!/bin/bash

PYTHON_VERSION=python3
PIP_VERSION=pip3

#######################################################
# Uncomment the following if your environment differs #
#######################################################

#$PIP_VERSION install -r requirements.txt

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
export PYTHONPATH=$PYTHONPATH:$PROJECT_ROOT:$PROJECT_ROOT/main

# Application configuration variables
export CONFIG_PATH=$PROJECT_ROOT/config.txt

##############################################
# Append the project's modules to PYTHONPATH #
##############################################
#export PYTHONPATH=$PYTHONPATH:$PROJECT_ROOT/main

####################
# Launch the tests #
####################

MAIN=$PROJECT_ROOT/tests/runner.py

$PYTHON_VERSION $MAIN
