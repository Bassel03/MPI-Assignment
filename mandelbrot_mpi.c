#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define HEIGHT 480
#define WIDTH 640
#define MAX_ITERATIONS 255

struct Complex {
    double real;
    double imag;
};

int calc(struct Complex c) {
    double x_real = 0;
    double x_imag = 0;
    double x_real2;
    double x_imag2;
    double magnitude_squared;
    int iteration = 0;
    do {
        x_real2 = x_real * x_real;
        x_imag2 = x_imag * x_imag;
        x_imag = 2 * x_real * x_imag + c.imag;
        x_real = x_real2 - x_imag2 + c.real;
        magnitude_squared = x_real2 + x_imag2;
        iteration++;
    } while ((iteration < MAX_ITERATIONS) && (magnitude_squared < 4.0));

    return iteration;
}

void save_image(const char *filename, int image[HEIGHT][WIDTH]) {
    FILE *pgmimg;
    pgmimg = fopen(filename, "wb");
    fprintf(pgmimg, "P2\n");
    fprintf(pgmimg, "%d %d\n", WIDTH, HEIGHT);
    fprintf(pgmimg, "255\n");

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fprintf(pgmimg, "%d ", image[i][j]);
        }
        fprintf(pgmimg, "\n");
    }
    fclose(pgmimg);
}

int main(int argc, char *argv[]) {
    int image[HEIGHT][WIDTH];
    double avg_time = 0;
    int num_trials = 10;
    double total_times[num_trials];

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_per_process = HEIGHT / size;
    int start_row = rank * rows_per_process;
    int end_row = start_row + rows_per_process;

    if (rank == size - 1) {
        end_row = HEIGHT;
    }

    for (int trial = 0; trial < num_trials; trial++) {
        MPI_Barrier(MPI_COMM_WORLD);
        clock_t start_time = clock();

#ifdef STATIC_ASSIGNMENT
        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < WIDTH; j++) {
                struct Complex c;
                c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
                c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
                image[i][j] = calc(c);
            }
        }
#endif

#ifdef DYNAMIC_ASSIGNMENT
        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < WIDTH; j++) {
                struct Complex c;
                c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
                c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
                image[i][j] = calc(c);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
#endif

        clock_t end_time = clock();
        total_times[trial] = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("Execution time of trial [%d] on process %d: %f seconds\n", trial, rank, total_times[trial]);
        avg_time += total_times[trial];
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double global_avg_time = 0;
    MPI_Reduce(&avg_time, &global_avg_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("The average execution time of %d trials is: %f ms\n", num_trials * size, global_avg_time / (num_trials * size) * 1000);
    }

    save_image("mandelbrot.pgm", image);

    MPI_Finalize();
    return 0;
}
