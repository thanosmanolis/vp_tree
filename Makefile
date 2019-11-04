# define the C/C++ compiler to use,default here is clang
CC = gcc-7

all:
	cd src; $(CC) -o ../sequential vptree_sequential.c main.c -lm; cd ..
	./sequential
	cd src; $(CC) -o ../pthreads vptree_pthreads.c main.c -lm -pthread; cd ..
	./pthreads
	cd src; $(CC) -o ../cilk vptree_cilk.c main.c -lm -fcilkplus; cd ..
	./cilk
	cd src; $(CC) -o ../openmp vptree_openmp.c main.c -lm -fopenmp; cd ..
	./openmp
