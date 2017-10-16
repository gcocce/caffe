#!/usr/bin/env sh
set -e

../../build/tools/caffe train --solver=solver.prototxt $@
