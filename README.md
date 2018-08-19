# Intro

This is an example of how to use caffe framework which has been tested under Raspberry an Organpe Pi.

## Caffe

[![Build Status](https://travis-ci.org/BVLC/caffe.svg?branch=master)](https://travis-ci.org/BVLC/caffe)
[![License](https://img.shields.io/badge/license-BSD-blue.svg)](LICENSE)

Caffe is a deep learning framework made with expression, speed, and modularity in mind.
It is developed by Berkeley AI Research ([BAIR](http://bair.berkeley.edu))/The Berkeley Vision and Learning Center (BVLC) and community contributors.

Check out the [project site](http://caffe.berkeleyvision.org) for all the details like

- [DIY Deep Learning for Vision with Caffe](https://docs.google.com/presentation/d/1UeKXVgRvvxg9OUdh_UiC5G71UMscNPlvArsWER41PsU/edit#slide=id.p)
- [Tutorial Documentation](http://caffe.berkeleyvision.org/tutorial/)
- [BAIR reference models](http://caffe.berkeleyvision.org/model_zoo.html) and the [community model zoo](https://github.com/BVLC/caffe/wiki/Model-Zoo)
- [Installation instructions](http://caffe.berkeleyvision.org/installation.html)

and step-by-step examples.

## Modifications in this repo

There are two directories added here:

* Images
* Project

Images contains source code to generate random images and Project contains source code to process those images with caffe framework.

## Project building

The makefile of the caffe project has been modified to allow you to compile the tools modified for this project with the next command:

```sh
$make project
CXX project/herramientas/entrenador.cpp
CXX/LD -o .build_release/project/herramientas/entrenador.bin
CXX project/herramientas/clasificador.cpp
CXX/LD -o .build_release/project/herramientas/clasificador.bin
```

First you have to build caffe framework with the same makefile as explained in the official caffe guide.

## Usage

### Generate images

Inside images/ directory there are three projects:

* trazos
* figuras
* arboles

each one of them allows to create a set of images.

Each project has a makefile to build the project.

```sh
$cd images/trazos
$ make
$ ./build/create_images
```
The process will take some time and it will create three directories with an image set for training, testing and validating:

* /img
* /test
* /validate

And it will create three files with the list of the files:

* traing_file.txt
* test_file.txt
* validate_file.txt


## Training and validating

Inside project/ directory there are also three directories which names are:

* trazos
* figuras
* arboles

Inside those directories there are files that allow you to preprocess images, neural network models for training and deploing, labels for classification categories and scripts to simplify training and validation.

* ./train_val.prototxt and ./deploy.prototxt
are neural network configuration for training and deploying

* ./preprocess.sh
It generates a database with the images and generates a file with the medium pixel values.

* ./train.sh
It initiate the training of the neural network which will generate a file with the network parameters (train.caffemodel)

* ./benchmark.sh train.caffemodel
It provides benchmark information about classifications time.

* ./validate.sh train.caffemodel [image or text file with list of images]
It will classify the image and output a result, in case of a list it will provide a percentage result.

## Results

* For results both in Raspberry and Orange pi you can check the [Resume](http://laboratorios.fi.uba.ar/lse/tesis/LSE-FIUBA-Trabajo-Final-CESE-Julio-Gaston-Cocce-2017.pdf)

Here a comparison between AlexNet model with the model used for images created inside arboles/ directory:
| Model | Images | Categories | Parameters | Orange Pi | Raspberry |
| ------------- |:-------------:| -----:| -----: | -----: | -----:|
| AlexNet     | 256x256 Color | 1000 | 238 Mbytes | 732 ms | 549 ms |
| Arboles     | 300x300 BW    | 2 | 2 Mbytes | 221 ms | 166 ms|


## License and Citation

Caffe is released under the [BSD 2-Clause license](https://github.com/BVLC/caffe/blob/master/LICENSE).
The BAIR/BVLC reference models are released for unrestricted use.