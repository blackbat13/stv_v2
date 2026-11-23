#!/bin/bash
# Deletes every .dot and .png file in the folder this script lives in.

DOT_DIR=$(cd "$(dirname "$0")" && pwd)

find "$DOT_DIR" -maxdepth 1 -type f \( -name '*.dot' -o -name '*.png' \) -delete
