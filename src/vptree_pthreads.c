/*
*************************************
*    Vantage Point Tree Sequential  *
*************************************
*/
#include "../inc/vptree.h"

typedef struct vptree vptree;
typedef struct dist_thread dist_thread;
typedef struct tree_thread tree_thread;

int activeThreads = 0, maxThreads = 62;

pthread_mutex_t mux;

static void* calcDistances(void* inp)
{
    pthread_mutex_lock (&mux);
    activeThreads ++;
    pthread_mutex_unlock (&mux);

    dist_thread *tun = (dist_thread *) inp;

    int n = tun->n;
    int d = tun->d;
    double *X = tun->X;
    double *dist = tun->dist;
    int start = tun->start;
    int end = tun->end;

    for(int i = start; i < end; i++)
    {
        double sum = 0;
        for(int j=0; j<d; j++)
            sum += (*(X + n*j + i) - *(X + n*j + (n-1))) * (*(X + n*j + i) - *(X + n*j + (n-1)));

        *(dist + i) = sqrt(sum);
    }

    pthread_mutex_lock (&mux);
    activeThreads --;
    pthread_mutex_unlock (&mux);

    pthread_exit(NULL);
}

static void* calcOuter(void* inp)
{
    pthread_mutex_lock (&mux);
    activeThreads ++;
    pthread_mutex_unlock (&mux);

    tree_thread *T_ = (tree_thread *) inp;

    T_->T = buildvp_recursive(T_->X, T_->n, T_->d, T_->idx);

    pthread_mutex_lock (&mux);
    activeThreads --;
    pthread_mutex_unlock (&mux);

    pthread_exit(NULL);
}

vptree * buildvp(double *X, int n, int d)
{
    int *idx_list;

    idx_list = malloc(n*sizeof(int));

    //! Initialize data
    for(int i=0; i<n; i++)
        for(int j=0; j<d; j++)
            *(idx_list + i) = i;

    vptree *T;

    T = buildvp_recursive(X, n, d, idx_list);

    free(idx_list);

    return T;
}

vptree * buildvp_recursive(double *X, int n, int d, int *idx_list)
{
    vptree *T = NULL;

    if(!n)
        return T;

    T = calloc(1, sizeof(vptree));

    T->vp = malloc(d * sizeof(double));

    //! Assume vantage point is the last one
    //! Vantage-point's coordinates
    for(int j=0; j<d; j++)
        *(T->vp + j) = *(X + n*j + n-1);

    //! Vantage-point's index
    T->idx = *(idx_list + n-1);

    if(n > 1)
    {
        double *dist = malloc((n-1)*sizeof(double));

        //! If n > 100000 calculate distances with Threads
        //! Ohterwise, do it sequentially
        if(n>100000)
        {
            //! Calculate number of threads to use
            int step = 100*n;
            int dist_threads_num = 10*( n + step - 1)/step;

            //! Initialize struct variables
            dist_thread *tun = malloc(dist_threads_num * sizeof(dist_thread));

            for(int i=0; i<dist_threads_num; i++)
            {
                tun[i].X = X;
                tun[i].d = d;
                tun[i].n = n;
                tun[i].start = ( i*(n-1) )/dist_threads_num;
                tun[i].end = ( (i+1)*(n-1) )/dist_threads_num;
                tun[i].dist = dist;
            }

            //! Calculate distances from the vantage point
            pthread_t threads[dist_threads_num];

            for(int i = 0; i<dist_threads_num; i++)
                pthread_create(&threads[i], NULL, calcDistances, &tun[i]);

            for(int i = 0; i<dist_threads_num; i++)
                pthread_join(threads[i], NULL);

            free(tun);
        }else
        {
            //! Calculate distances from the vantage point
            for(int i=0; i<n-1; i++)
            {
                double sum = 0;
                for(int j=0; j<d; j++)
                    sum += (*(X + n*j + i) - *(X + n*j + (n-1))) * (*(X + n*j + i) - *(X + n*j + (n-1)));

                *(dist + i) = sqrt(sum);
            }
        }

        double md;
        double *dist_cp = malloc((n-1)*sizeof(double));
        //! Copy dist array in order to implement quickselect to it and find medan value
        //! Then, we will use the dist array again so that the right indexes are kept
        for(int i=0; i<n-1; i++)
            *(dist_cp + i) = *(dist + i);

        //! Find median distance (md)
        if((n-1)%2 != 0)
            md = quickselect(dist_cp, 0, n-2, n/2);
        else
            md = ( quickselect(dist_cp, 0, n-2, (n-1)/2) + quickselect(dist_cp, 0, n-2, (n-1)/2 + 1))/2;

        T->md = md;

        //! Calculate number of inner and outer subtree elements
        int *inner_idx, inner_n = 0, *outer_idx, outer_n;
        double *inner, *outer;

        for(int i=0; i<n-1; i++)
            if( *(dist + i) <= md )
                inner_n++;

        outer_n = (n-1) - inner_n;

        //! Declare inner and outer data
        inner = malloc(inner_n*d*sizeof(double));
        inner_idx = malloc(inner_n*sizeof(int));
        outer = malloc(outer_n*d*sizeof(double));
        outer_idx = malloc(outer_n*sizeof(int));

        //! Create inner Tree
        int inner_counter = 0;
        for(int i=0; i<n-1; i++)
            if( *(dist + i) <= md )
            {
                for(int j=0; j<d; j++)
                    *(inner + inner_n*j + inner_counter) = *(X + n*j + i);

                *(inner_idx + inner_counter) = *(idx_list + i);

                inner_counter++;
            }

        if(n>2)
        {
            //! Create outer Tree
            int outer_counter = 0;
            for(int i=0; i<n-1; i++)
                if( *(dist + i) > md )
                {
                    for(int j=0; j<d; j++)
                        *(outer + outer_n*j + outer_counter) = *(X + n*j + i);

                    *(outer_idx + outer_counter) = *(idx_list + i);

                    outer_counter++;
                }

            if(n>1000 && activeThreads<maxThreads-2)
            {
                //! T inner
                pthread_t thread1;
                tree_thread *T_inner = calloc(1, sizeof(tree_thread));

                T_inner->d = d;
                T_inner->n = inner_n;
                T_inner->idx = inner_idx;
                T_inner->X = inner;

                //! T Outer
                pthread_t thread2;
                tree_thread *T_outer = calloc(1, sizeof(tree_thread));

                T_outer->d = d;
                T_outer->n = outer_n;
                T_outer->idx = outer_idx;
                T_outer->X = outer;

                T_inner->T = T->inner;
                T_outer->T = T->outer;

                pthread_create(&thread1, NULL, calcOuter, T_inner);
                pthread_create(&thread2, NULL, calcOuter, T_outer);

                pthread_join(thread1, NULL);
                pthread_join(thread2, NULL);

                free(T_inner);
                free(T_outer);
            }else
            {
                T->inner = buildvp_recursive(inner, inner_n, d, inner_idx);
                T->outer = buildvp_recursive(outer, outer_n, d, outer_idx);
            }
        }else
        {
            T->inner = buildvp_recursive(inner, inner_n, d, inner_idx);
            T->outer = buildvp_recursive(outer, 0, d, outer_idx);
        }

        //! Free allocated memory
        free(dist);
        free(dist_cp);
        free(inner);
        free(inner_idx);
        free(outer);
        free(outer_idx);
    }

    return T;
}

vptree * getInner(vptree * T)
{
    return T->inner;
}

vptree * getOuter(vptree * T)
{
    return T->outer;
}

double getMD(vptree * T)
{
    return T->md;
}

double * getVP(vptree * T)
{
    return T->vp;
}

int getIDX(vptree * T)
{
    return T->idx;
}

void swap(double* a, double* b)
{
    double t = *a;
    *a = *b;
    *b = t;
}

int partition(double* arr, int l, int r)
{
    double x = *(arr + r);
    int i = l;
    for (int j = l; j <= r - 1; j++)
    {
        if (*(arr + j) <= x)
        {
            swap(&*(arr + i), &*(arr + j));
            i++;
        }
    }
    swap(&*(arr + i), &*(arr + r));
    return i;
}

double quickselect(double* arr, int l, int r, int k)
{
    if (k > 0 && k <= r - l + 1)
    {
        int index = partition(arr, l, r);

        if (index - l == k - 1)
            return *(arr + index);

        if (index - l > k - 1)
            return quickselect(arr, l, index - 1, k);

        return quickselect(arr, index + 1, r, k - index + l - 1);
    }

    return DBL_MAX;
}

void destroy(vptree *T)
{
    if(T==NULL)
    {
        return;
    }

    destroy(T->inner);
    destroy(T->outer);

    free(T->vp);
    free(T);
}
