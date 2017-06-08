# Surface Reconstruction plugin for V-REP

### Compiling

1. Install required packages for [v_repStubsGen](https://github.com/fferri/v_repStubsGen): see v_repStubsGen's [README](external/v_repStubsGen/README.md)
2. Install CGAL 4.9
3. Checkout and compile
```text
$ git clone --recursive https://github.com/fferri/v_repExtSurfaceReconstruction.git
$ cmake .
$ cmake --build .
```
You may need to set `CGAL_DIR` cmake variable to the correct value, e.g.:
```text
$ cmake -G "MinGW Makefiles" -DCGAL_DIR=c:\local\CGAL-4.10-32bit\build .
$ cmake --build .
```
