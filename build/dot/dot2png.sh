#!/bin/bash

TARGET_DIR=${1:-'.'}

cd $TARGET_DIR
for f in $(ls | grep -P '.dot$'); do
    echo $f
    dot -Tpng "$f" > "${f%.dot}.png"
done
cd $OLDPWD
