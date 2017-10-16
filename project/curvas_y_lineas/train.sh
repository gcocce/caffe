#!/usr/bin/env sh
set -e

cd ../../images/curvas_y_lineas

../../build/tools/convert_imageset -gray ./ train_file.txt ../../project/curvas_y_lineas/mylistdb
../../build/tools/convert_imageset -gray ./ test_file.txt ../../project/curvas_y_lineas/mytestlistdb

cd ../../project/curvas_y_lineas

../../build/tools/compute_image_mean mylistdb mean.binaryproto

../../build/tools/caffe train --solver=solver.prototxt $@