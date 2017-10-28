#!/usr/bin/env sh
set -e

../../build/tools/caffe time -model deploy.prototxt -weights train_iter_3500.caffemodel -iterations 10
