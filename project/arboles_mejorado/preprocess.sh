#!/usr/bin/env sh
set -e

cd ../../images/arboles
../../build/tools/convert_imageset -gray ./ train_file.txt ../../project/arboles_mejorado/mylistdb
../../build/tools/convert_imageset -gray ./ test_file.txt ../../project/arboles_mejorado/mytestlistdb
cd ../../project/arboles_mejorado
../../build/tools/compute_image_mean mylistdb mean.binaryproto
