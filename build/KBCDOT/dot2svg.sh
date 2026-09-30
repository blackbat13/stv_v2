#!/bin/bash
#TARGET_DIR=${1:-'./dot'}

#cd $TARGET_DIR
for f in $(ls | grep -P '.dot$'); do
    dot -v -Tsvg "$f" > "${f%.dot}.svg"
done
cd $OLDPWD
