#include "memory_pool.h"


template<size_t BlockSize>MemoryPool<BlockSize>::MemoryPool() {
    free_memory_block_head_ = new MemoryBlock();
    free_memory_block_tail_ = new MemoryBlock();
    free_memory_block_head_->next = free_memory_block_tail_;
    free_memory_addr_ = &free_memory_block_head_->buffer;
    total_memory_block_count_ = 2;
    free_memory_block_count_ = 1;
    memory_ascription_ = new trl::unordered_map<void *, MemoryBlock *>();
}

template<size_t BlockSize>MemoryPool<BlockSize>::~MemoryPool() {
   for(auto memory_ascription_iterator :memory_ascription_) {
       if(memory_ascription_iterator.second != free_memory_block_head_) {
           delete memory_ascription_iterator.second;
           //test only
           total_free_count++;
           //test only
       }
   }
   delete memory_ascription_;
   MemoryBlock * next = nullptr;
   while(free_memory_block_head_ != nullptr) {
       next = free_memory_block_head_->next;
       delete free_memory_block_head_;
       //test only
       total_free_count++;
       //test only
       free_memory_block_head_ = next;
   }
   printf("total_new_count = %d, total_free_count = %d\n",total_new_count,total_free_count);
}

template<size_t BlockSize>void MemoryPool<BlockSize>::DeleteRedundantMemoryBlock(){
    if(total_memory_block_count_ - free_memory_block_count_ + 1 < free_memory_block_count_ * 2 && 
       free_memory_block_count_ > 2) {
        for(size_t i = 0; i < 2; i++) {
            MemoryBlock * next = free_memory_block_head_->next->next;
            delete free_memory_block_head_->next;
            free_memory_block_count_--;
            total_memory_block_count_--;
            free_memory_block_head_->next = next;
        }
    }
}

template<size_t BlockSize>void MemoryPool<BlockSize>::AddFreeMemoryBlock(MemoryBlock *memory_block) {
    free_memory_block_tail_->next = memory_block;
    free_memory_block_tail_ = free_memory_block_tail_->next;
    free_memory_block_count_++;
}

template<size_t BlockSize>void MemoryPool<BlockSize>::NewMemoryBlock() {
    AddFreeMemoryBlock(new MemoryBlock());
    total_memory_block_count_++;
    //test only
    total_new_count++;
    //test only
}

template<size_t BlockSize>void *MemoryPool<BlockSize>::Malloc(size_t size) {
    size_t need_size = size + sizeof(size_t);
    if(need_size > BlockSize) {

        perror("MemoryPool malloc size > block size!!!");
    }
    if(free_memory_block_head_->free_size < need_size) {
        free_memory_block_head_ = free_memory_block_->next;
        free_memory_addr_ = &free_memory_block_head_->buffer;
        free_memory_block_count_--;
        if(free_memory_block_head == free_memory_block_tail_) {
            NewMemoryBlock();
        }
    }
    memcpy(free_memory_addr_,&need_size, sizeof(size_t));
    void * malloc_memory_addr_ = (void *)(free_memory_addr_ + sizeof(size_t));
    free_memory_addr_ += need_size;
    free_memory_block_head_->free_size -= need_size;
    (*memory_ascription_)[malloc_memory_addr_] = free_memory_block_head_;
    DeleteRedundantMemoryBlock();
    return malloc_memory_addr_;
}

template<size_t BlockSize>void MemoryPool<BlockSize>::Free(void *addr) {
    auto memory_ascription_iterator = memory_ascription_->find(addr);
    if(memory_ascription_iterator == memory_ascription_->end()) {
        return;
    }
    size_t malloc_size;
    memcpy(&malloc_size, addr - sizeof(size_t), sizeof(size_t));
    memory_ascription_iterator->second->free_size += malloc_size;
    if(memory_ascription_iterator->second->free_size == BlockSize && 
       memory_ascription_iterator->second != free_memory_block_head_) {
        memory_ascription_iterator->second->MemoryBlock();
        AddFreeMemoryBlock(memory_ascription_iterator->second);
        memory_ascription_->erase(memory_ascription_iterator);
    }
    
}