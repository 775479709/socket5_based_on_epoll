#include "memory_pool.h"
#include<stdlib.h>
#include<sys/time.h>
#include<vector>
#include<time.h>
#include<queue>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
//#pragma GCC optimize(2)


int myrand(int l,int r){
    return rand()%(r-l+1) +l;
}

class Buffer{
    char data[4096];
public:
    Buffer(){}
    ~Buffer(){}
};

bool IsMalloc() {
    int op = myrand(1,5);
    return op > 2;
}

const int max_time = 100000;

int main(int argc, char **argv)
{
    srand(time(0));
    timeval start, end;
    const size_t mx= 128;
    std::queue<Buffer *>q;
    if(argc > 1) {

        MemoryPool<Buffer,1024>* memory_pool = new MemoryPool<Buffer,1024>();
        size_t malloc_count = 0;
        size_t free_count = 0;
        // for(int i = 0; i < max_time; i++) {
        //     q.push(memory_pool->construct());
        // }
        // while(!q.empty()) {
        //     memory_pool->Free(q.front());
        //     q.pop();
        // }

        
        gettimeofday(&start, 0);
        for(int i = 0; i < max_time;i++) {
            if(IsMalloc()) {
                Buffer * new_f = memory_pool->New();
                q.push(new_f);
                malloc_count++;
            }else if(q.size()){
                memory_pool->Delete(q.front());
                q.pop();
                free_count++;
            }
        }
        gettimeofday(&end, 0);
        long long t1 = 1000000ll*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        memory_pool->DeleteRedundantMemoryBlock();
        printf("malloc_count= %lu,free_count = %lu,time =%lld\n",malloc_count,free_count,t1);
        
        while(!q.empty()) {
            memory_pool->Delete(q.front());
            q.pop();
        }
        delete memory_pool;
    }else {

        size_t malloc_count = 0;
        size_t free_count = 0;

        
        // while(!q.empty()) {
        //     free(q.front());
        //     q.pop();
        // }

        gettimeofday(&start, 0);
        for(int i = 0; i < max_time;i++) {
             if(IsMalloc()) {
                q.push(new Buffer);
                malloc_count++;
            }else if(q.size()){
                delete q.front();
                free_count++;
                q.pop();
            }
        }
        gettimeofday(&end, 0);
        long long t1 = 1000000ll*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        printf("malloc_count= %lu,free_count = %lu,time =%lld\n",malloc_count,free_count,t1);
        // while(!q.empty()) {
        //     delete q.front();
        //     q.pop();
        // }


    }





    return 0;
}

