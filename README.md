# Toy-Neural-Network-C [![CodeFactor](https://www.codefactor.io/repository/github/vnepogodin/toy-neural-network-c/badge)](https://www.codefactor.io/repository/github/vnepogodin/toy-neural-network-c) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/79f9ce19cfd84b268e03ef29e1bb37c0)](https://www.codacy.com/manual/nepogodin.vlad/Neural-Network-C?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=ctrlcvnigerguard/Neural-Network-C&amp;utm_campaign=Badge_Grade) [![Build Status](https://travis-ci.com/vnepogodin/Toy-Neural-Network-C.svg?branch=master)](https://travis-ci.com/vnepogodin/Toy-Neural-Network-C)

Port [Toy-Neural-Network-JS](https://github.com/CodingTrain/Toy-Neural-Network-JS) to C++ and C

## Examples
Here are some demos running directly on your computer:
* [ColorPredictor](https://github.com/vnepogodin/ColorPredictor)
* [GradientDescent](https://github.com/vnepogodin/GradientDescent)

### Documentation
 For C developers
   * `NeuralNetwork` - The neural network structure
     * `neural_network_predict(nn, input_array)` - Returns the output of a neural network
     * `neural_network_train(nn, input_array, target_array)` - Trains a neural network

 For C++ developers
   * `NeuralNetwork` - The neural network class
     * `predict(input_array)` - Returns the output of a neural network
     * `train(input_array, target_array)` - Trains a neural network

## Libraries used in this project

* [JSON for Modern C++](https://github.com/nlohmann/json) used for Json serialize / deserialize in C++.
* [JSON-C](https://github.com/json-c/json-c) used for Json serialize / deserialize in C.

## License

This project is licensed under the terms of the MIT license, see LICENSE.
