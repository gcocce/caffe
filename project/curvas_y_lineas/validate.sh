#!/usr/bin/env sh
set -e

../../build/project/tool/classification.bin deploy.prototxt mytrain_iter_2500.caffemodel mean.binaryproto labels.txt validate_file.txt
