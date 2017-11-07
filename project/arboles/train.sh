#!/usr/bin/env sh
set -e

../../build/project/herramientas/entrenador.bin train --solver=solver.prototxt $@
