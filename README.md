# Surface Reconstruction plugin for CoppeliaSim

### Compiling

1. Install required packages for [libPlugin](https://github.com/CoppeliaRobotics/libPlugin): see libPlugin's README
2. Install CGAL 4.11
3. Checkout and compile
```text
$ git clone --recursive https://github.com/CoppeliaRobotics/simExtSurfRec.git
$ cmake .
$ cmake --build .
```
You may need to set `CGAL_DIR` variable to the correct value, e.g.:
```text
$ cmake -G "MinGW Makefiles" -DCGAL_DIR=c:\local\CGAL-4.11-32bit\build .
$ cmake --build .
```
