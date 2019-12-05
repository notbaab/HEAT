#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# We want to include this directory

ln -s "$DIR" "$1/HEAT"
