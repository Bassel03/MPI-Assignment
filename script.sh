mpicc -o mandelbrot_static mandelbrot_mpi.c -DSTATIC_ASSIGNMENT -Wall -lm
mpirun -np 4 ./mandelbrot_static
mpicc -o mandelbrot_dynamic mandelbrot_mpi.c -DDYNAMIC_ASSIGNMENT -Wall -lm
mpirun -np 4 ./mandelbrot_dynamic

