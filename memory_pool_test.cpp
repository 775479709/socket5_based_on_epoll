#include "memory_pool.h"
#include<stdlib.h>
#include<sys/time.h>

int myrand(int l,int r){
    return rand()%(r-l+1) +l;
}

int main(int argc, char **argv)
{
    srand(time(0));
    timeval start, end;
    if(argc > 1) {
        MemoryPool<1024>* memory_pool = new MemoryPool<1024>();
        long long all_1 = 0;
        gettimeofday(&start, 0);
        for(int i = 0; i < 1000000;i++) {
            size_t size = myrand(1,800);
            char *f = (char *)memory_pool->Malloc(size);
            all_1 += size;
            memory_pool->Free((void *)f);
        }
        gettimeofday(&end, 0);
        int t1 = 1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        printf("allsize = %lld,time =%d\n",all_1,t1);
        delete memory_pool;
    }else {
        long long all_1 = 0;
        gettimeofday(&start, 0);
        for(int i = 0; i < 1000000;i++) {
            size_t size = myrand(1,800);
            char *f = new char[size];
            all_1 += size;
            delete [] f;
        }
        gettimeofday(&end, 0);
        int t1 = 1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        printf("allsize = %lld,time =%d\n",all_1,t1);
    }





    return 0;
}