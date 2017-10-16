#!/usr/bin/env sh
set -e

../../build/tools/convert_imageset -gray ./ ../../images/arboles/train_file.txt mylistdb

#../../build/tools/convert_imageset -gray ./ test_file.txt mytestlistdb
#../../build/tools/compute_image_mean mylistdb mean.binaryproto

#../../build/tools/caffe train --solver=solver.prototxt $@
