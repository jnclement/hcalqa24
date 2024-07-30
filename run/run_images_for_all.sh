#!/bin/bash

for file in `ls lists/*.imagelist`; do
    bash run_images.sh $file
done
