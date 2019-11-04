#include "../inc/vptree.h"

//! Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define GREEN_BOLD "\033[1;32m"
#define YELLOW "\033[0;33m"
#define RESET_COLOR "\033[0m"

typedef struct vptree vptree;

struct timeval startwtime, endwtime;
static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;

double seq_time;
double p_time;

int main()
{
    int n, d;
    double *X, cpu_time_used;
    clock_t start, end;

    n = 10000;
    d = 20;

    X = malloc(n*d*sizeof(double));

    //! Initialize data
    for (int i=0;i<n*d;i++)
      *(X + i) = (double)rand()/(double)RAND_MAX;

    // ! Print X array
    // for (int i=0; i<n; i++)
    // {
    //     for(int j=0; j<d; j++)
    //         printf("%f ", *(X + n*j + i));
    //
    //     printf("\n\n");
    // }

    //! Set this timestamp as start
    gettimeofday (&startwtime, NULL);

    st_time = times(&st_cpu);

    vptree *T;
    T = buildvp(X, n, d);

    //! Set this timestamp as end
    en_time = times(&en_cpu);
    gettimeofday (&endwtime, NULL);
    p_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
  		      + endwtime.tv_sec - startwtime.tv_sec);

    printf(RED "%f sec\n" RESET_COLOR,p_time);

    printf(YELLOW "Real Time: %jd, User Time %jd, System Time %jd\n" RESET_COLOR,
           (long)(en_time - st_time),
           (long)(en_cpu.tms_utime - st_cpu.tms_utime),
           (long)(en_cpu.tms_stime - st_cpu.tms_stime));
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    free(X);
    destroy(T);

    return 0;
}
