# xraymcExample3

## About
This is an example on how to use the xraymc C++ library for simulation of radiation transport in an interventional lab. This example illustrates how to build a model of an X-ray guidet interventional lab with patient, staff and shielding equitment and how to perform a Monte Carlo photon transport simulation.

## How to run
Running the example requires a C++ build environment, the build generator used by this example and the xraymc library is CMake. Using CMake is the preferred/easiest way to use xraymc and this example. To build this example please install:
*C++ compiler that supports C++20 (newest Microsoft Visual Studio, Apple XCode or Linux GCC/Clang)
*Git
*Git LFS (Large file support)
*CMake (install the CMake-GUI for a GUI interface)


### Command line instructions
Start by cloning the xraymcExample3 repository, navigate to a folder you want the source code and clone the repository:
git clone https://github.com/medicalphysics/xraymcExample3.git .
The phantom models are included with the source code, so it will take some minutes to clone the repository. 

Create a build directory
mkdir build

Generate build files 
cmake -S . -B build
During this configure step CMake will download xraymc from GitHub and photon attenuation data from IAEA.

Compile the the example
cmake --build build --config Release --target xraymcexample3

Run the example by running the xraymcexample3 executable in the build folder.

