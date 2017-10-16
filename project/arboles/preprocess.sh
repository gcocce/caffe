#!/usr/bin/env sh
set -e

cd ../../images/arboles
../../build/tools/convert_imageset -gray ./ train_file.txt ../../project/arboles/mylistdb
../../build/tools/convert_imageset -gray ./ test_file.txt ../../project/arboles/mytestlistdb
cd ../../project/curvas_y_lineas
../../build/tools/compute_image_mean mylistdb mean.binaryproto
