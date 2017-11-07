#!/usr/bin/env sh
set -e

cd ../../images/trazos
../../build/tools/convert_imageset -gray ./ train_file.txt ../../project/trazos/mylistdb
../../build/tools/convert_imageset -gray ./ test_file.txt ../../project/trazos/mytestlistdb
cd ../../project/trazos
../../build/tools/compute_image_mean mylistdb mean.binaryproto
