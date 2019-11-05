# vp_tree

## Description
Sequential and parallel implementation of vptree.

There are 4 different implementations. One sequential, and three in parallel (using pthreads, cilk, and openmp).

## How to run it

1. If your gcc version is previous than gcc-7, then change it in the Makefile
2. Type "make lib" so that the Makefile produces four libraries, one for each implementation
3. Use those files along with inc/vptree.h at your own Makefile, with your main function, to produce executable files.
