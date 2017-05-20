Compiling:

1) Get required python packages for v_repStubsGen

   On Linux:

	> sudo apt-get install python-tempita

   On OS X:

	> pip install tempita

2) Download and install CGAL 4.9 or greater

3) Generate makefiles

   On Linux:

	> ~/Qt5.5.0/5.5/gcc_64/bin/qmake plugin.pro

   On OS X:

	> ~/Qt5.5.0/5.5/clang_64/bin/qmake plugin.pro

4) Compile

	> make

