#include<tr1/unordered_map>
#include<string.h>
#include<stddef.h>
template<size_t BlockSize>
class MemoryPool{
private:
    struct MemoryBlock{
        MemoryBlock * next;
        size_t free_size;
        char buffer[BlockSize];
        MemoryBlock():next(nullptr),free_size(BlockSize){}
    };

public:
    void *Malloc(size_t size);
    void Free(void * addr);
    MemoryPool();
    ~MemoryPool();

private:
    void NewMemoryBlock();
    void AddFreeMemoryBlock(MemoryBlock *memory_block);


private:
    MemoryBlock *free_memory_block_head_;
    MemoryBlock *free_memory_block_tail_;
    char *free_memory_addr_;
    sizt_t total_memory_block_count_;
    size_t free_memory_block_count_;
    trl::unordered_map<void *, MemoryBlock *> *memory_ascription_;
};
