# define the C/C++ compiler to use,default here is clang
CC = gcc-7

all:
	cd src; gcc -o ../sequential vptree_sequential.c main.c -lm; cd ..
	cd src; gcc -o ../pthreads vptree_pthreads.c main.c -pthread -lm; cd ..
	cd src; gcc -o ../cilk vptree_cilk.c main.c -fcilkplus -lm; cd ..
	cd src; gcc -o ../openmp vptree_openmp.c main.c -fopenmp -lm; cd ..
