#!/bin/bash
#
# Run in order to launch the app
#

# Uncomment the following if your current environment's requirements.txt differ
#pip install -r requirements.txt

PYTHON_VERSION=python3

############################################
# Create storage files if they don't exist #
############################################
STORAGE_FILES="storage/volumes/"

if [ -z "$(ls $STORAGE_FILES 2>/dev/null)" ]; then
  # Create the directory holding the files
  mkdir $STORAGE_FILES

  # Create the text files that are required to store users' data
  $PYTHON_VERSION storage/config.py
fi

##############################################
# Append the project's modules to PYTHONPATH #
##############################################
export PROJECT_ROOT=$PWD
export PYTHONPATH=$PYTHONPATH:$PROJECT_ROOT

for dir in $PROJECT_ROOT/*; do
	export PYTHONPATH=$PYTHONPATH:$dir
done

###########################################
# Launch the app (http://127.0.0.1:5000/) #
###########################################
$PYTHON_VERSION web/app.py
