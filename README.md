# Surface Reconstruction plugin for V-REP

### Compiling

1. Install required packages for [v_repStubsGen](https://github.com/fferri/v_repStubsGen): see v_repStubsGen's [README](external/v_repStubsGen/README.md)
2. Install CGAL 4.9
3. Checkout and compile
```
$ git clone --recursive https://github.com/fferri/v_repExtSurfaceReconstruction.git
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```
