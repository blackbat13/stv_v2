#!/bin/bash
for f in *.dot; do
    dot -Tsvg "$f" > "${f%.dot}.svg"
done