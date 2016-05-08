#!/bin/bash
#
# Run on the client machine
#

PYTHON_VERSION=python3
PIP_VERSION=pip3

#######################################################
# Uncomment the following if your environment differs #
#######################################################

#$PIP_VERSION install -r requirements.txt

########################################
# Application configuration parameters #
########################################

# The root of the project is the root of this script
# http://stackoverflow.com/a/246128/2708484
PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Make all root-level modules of the app accessible
export PATH=$PATH:$PROJECT_ROOT:$PROJECT_ROOT/main

# Application configuration variables
export CONFIG_PATH=$PROJECT_ROOT/config.txt

#########################
# Networking parameters #
#########################

# Ports of the replica managers in the network
export TOPOLOGY_PATH=$PROJECT_ROOT/topology.txt

# Max. size of the buffer used for communications across the network
export DATASERVER_BUFFSIZE=8192

# Host of the RMs
export DATASERVER_HOST="localhost"

# Port that this process will be using
export CLIENT_PORT=13000

###########################################
# Launch the app (http://127.0.0.1:5000/) #
###########################################

APP=$PROJECT_ROOT/main/app/app.py

$PYTHON_VERSION $APP
