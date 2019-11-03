#include "../inc/vptree.h"

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

    //! Works for any n or d
    n = 10000;
    d = 20;

    X = malloc(n*d*sizeof(double));

    //! Initialize data
    for (int i=0;i<n*d;i++)
      *(X + i) = (double)rand()/(double)RAND_MAX;

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

    printf("%f sec\n",p_time);

    printf("Real Time: %jd, User Time %jd, System Time %jd\n",
           (long)(en_time - st_time),
           (long)(en_cpu.tms_utime - st_cpu.tms_utime),
           (long)(en_cpu.tms_stime - st_cpu.tms_stime));
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    free(X);
    destroy(T);

    return 0;
}
