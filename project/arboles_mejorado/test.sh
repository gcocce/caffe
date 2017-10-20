#!/usr/bin/env sh
set -e

../../build/tools/caffe time -model deploy.prototxt -weights train_iter_2500.caffemodel -iterations 10
