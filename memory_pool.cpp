#include "memory_pool.h"


template<size_t BlockSize>MemoryPool<BlockSize>::MemoryPool() {
    free_memory_block_ = nullptr;
    used_memory_block_ = nullptr;
    memory_ascription_ = new trl::unordered_map<void *, MemoryBlock *>();
}

template<size_t BlockSize>void *MemoryPool<BlockSize>::Malloc(size_t size) {
    if(size + sizeof(size_t) > BlockSize) {

        perror("MemoryPool malloc size > block size!!!");
    }
    if(free_memory_block_ == nullptr) {
        free_memory_block_ = new MemoryBlock();
    }
    if(free_memory_block_->free_size < size) {
        
    }
}