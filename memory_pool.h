#include<tr1/unordered_map>
#include<stddef.h>
template<size_t BlockSize>
class MemoryPool{
private:
    struct MemoryBlock{
        MemoryBlock * next = nullptr;
        size_t free_size = BlockSize;
        char buffer[BlockSize];
    };

public:
    void *Malloc(size_t size);
    void Free(void *);
    MemoryPool();
    ~MemoryPool();


private:
    MemoryBlock *free_memory_block_;
    MemoryBlock *used_memory_block_;
    char *
    trl::unordered_map<void *, MemoryBlock *> *memory_ascription_;
};