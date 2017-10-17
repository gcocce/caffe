#!/usr/bin/env sh
set -e

cd ../../images/aros_y_rectangulos
../../build/tools/convert_imageset -gray ./ train_file.txt ../../project/aros_y_rectangulos/mylistdb
../../build/tools/convert_imageset -gray ./ test_file.txt ../../project/aros_y_rectangulos/mytestlistdb
cd ../../project/aros_y_rectangulos
../../build/tools/compute_image_mean mylistdb mean.binaryproto
