# Ray Tracer
A simple CPU based ray tracer that renders .NFF files and outputs them as simple .PPM files

Utilizes SIMD operations as well as multithreading to achieve near real time render speeds.

### Dependencies
- Requires Eigen (3.4.0 or later) to function

### Compiling
#### Visual Studio
1. Download and install Eigen from here https://eigen.tuxfamily.org/index.php?title=Main_Page
2. Include Eigen in the project by adding the path in `Project Properties > C/C++ > General > Additional Include Directories` 
3. Build using Release mode on Visual Studio
#### Makefile / g++
1. Download and install Eigen from here https://eigen.tuxfamily.org/index.php?title=Main_Page
2. Include Eigen in the project by adding the path to the `CFLAGS` field in the Makefile like so `-I /<path>/include/eigen3.4.0/`
3. Build using the command `make` in the project directory

### Usage
The program takes two command line inputs: \<input file\> the path to an NFF file and \<output file\> the path to the rendered image

#### Windows: 
`raytracer.exe <input.nff> <output.ppm>`

#### Linux: 
`./raytracer <input.nff> <output.ppm>`

### Sample Output:
![Rendered image of a burnt orange teapot on a reflective black and white checkerboard with a sky blue background](https://github.com/RyderReed15/RayTracer/blob/master/examples/teapot.png)
