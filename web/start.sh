#!/bin/bash

# TODO: get absolute path
PROJECT_ROOT=$PWD/web

# Application configuration variables
CONFIG_PATH=$PROJECT_ROOT/config

###########################################
# Launch the app (http://127.0.0.1:5000/) #
###########################################
PYTHON_VERSION=python3
APP=$PROJECT_ROOT/main/app.py

$PYTHON_VERSION $APP
