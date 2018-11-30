#include "memory_pool.h"
#include<stdlib.h>
#include<sys/time.h>
#include<vector>
int myrand(int l,int r){
    return rand()%(r-l+1) +l;
}

int main(int argc, char **argv)
{
    srand(time(0));
    timeval start, end;
    const size_t mx= 128;
    char ** p = new char *[5000000];
    if(argc > 1) {

        MemoryPool<mx * 1024*10>* memory_pool = new MemoryPool<mx * 1024*10>();
        long long all_1 = 0;
        
        gettimeofday(&start, 0);
        for(int i = 0; i < 5000000;i++) {
            size_t size = myrand(1,mx - 10);
            char *f = (char *)memory_pool->Malloc(size);
            all_1 += size;
            p[i] =f;

            // size_t size2 = myrand(1,mx - 100);
            // char *f2 = (char *)memory_pool->Malloc(size2);
            // all_1 += size2;

            // size_t size3 = myrand(1,mx - 100);
            // char *f3 = (char *)memory_pool->Malloc(size3);
            // all_1 += size3;

            //memory_pool->Free((void *)f);
            // memory_pool->Free((void *)f2);
            // memory_pool->Free((void *)f3);
        }
        gettimeofday(&end, 0);
        long long t1 = 1000000ll*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        printf("allsize = %lld,time =%lld\n",all_1,t1);


        // gettimeofday(&start, 0);
        // for(int i = 0; i < 5000000;i++) {
        //     memory_pool->Free((void *)p[i]);
        // }
        // gettimeofday(&end, 0);
        // t1 = 1000000ll*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);

        printf("free ::allsize = %lld,time =%lld\n",all_1,t1);
        delete memory_pool;
    }else {
        long long all_1 = 0;
        gettimeofday(&start, 0);
        for(int i = 0; i < 5000000;i++) {
            size_t size = myrand(1,mx - 10);
            char *f = (char *)malloc(size);
            all_1 += size;
            p[i] = f;

            // size_t size2 = myrand(1,mx - 100);
            // char *f2 = new char[size];
            // all_1 += size2;
            // size_t size3 = myrand(1,mx - 100);
            // char *f3 = new char[size];
            // all_1 += size3;
            //delete [] f;
            // delete [] f2;
            // delete [] f3;
        }
        gettimeofday(&end, 0);
        int t1 = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        printf("allsize = %lld,time =%d\n",all_1,t1);


        gettimeofday(&start, 0);
        for(int i = 0; i < 1000000;i++) {
            delete p[i];
        }
        gettimeofday(&end, 0);
        t1 = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        printf("free::allsize = %lld,time =%d\n",all_1,t1);
    }





    return 0;
}