# Surface Reconstruction plugin for V-REP

### Compiling

1. Install required python packages for [v_repStubsGen](https://github.com/fferri/v_repStubsGen): see v_repStubsGen's [README](https://github.com/fferri/v_repStubsGen/blob/master/README.md)
2. Install `xsltproc` (an XSLT processor)
3. Install CGAL 4.9
4. Download and install Qt (same version as V-REP, i.e. 5.5.0)
5. Checkout and compile
```
$ git clone --recursive https://github.com/fferri/v_repExtCustomUI.git
$ /path/to/Qt5.5.0/5.5/.../bin/qmake v_repExtCustomUI.pro
$ make
```