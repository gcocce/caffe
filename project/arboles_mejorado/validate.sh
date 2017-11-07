#!/usr/bin/env sh
set -e

if [ "$#" -eq 1 ];then
../../build/project/herramientas/clasificador.bin deploy.prototxt $1 mean.binaryproto labels.txt validate_file.txt
else
	echo "Uso del script:"
	echo "./script archivo.caffemodel"
fi


#../../build/project/tool/clasificador.bin deploy.prototxt train_iter_500.caffemodel mean.binaryproto labels.txt validate_file.txt

