#!/usr/bin/env sh
set -e

../../build/project/tool/classification.bin deploy.prototxt train_iter_10000.caffemodel mean.binaryproto labels.txt validate_file.txt
