#!/usr/bin/env sh
set -e

if [ "$#" -eq 1 ];then
	../../build/project/herramientas/entrenador.bin time -model deploy.prototxt -weights $1 -iterations 10
else
	echo "Uso del script:"
	echo "./script archivo.caffemodel"
fi
