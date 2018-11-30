#ifndef MEMORY_POLL_H_
#define MEMORY_POLL_H_

#include<unordered_map>
#include<string.h>
#include<stddef.h>
#include<stdio.h>


template<class T>
class MemoryPool {
    public:
    T *New();
    void Free(T *buffer);
private:
    size_t total_Block_count_;
    size_t free_block_count_;
    T *free_Block_head_;
};



// template<size_t BlockSize>
// class MemoryPool{
// private:
//     struct MemoryBlock{
//         MemoryBlock * next;
//         size_t free_size;
//         char buffer[BlockSize];
//         MemoryBlock():next(nullptr),free_size(BlockSize){}
//     };

// public:
//     void *Malloc(size_t size);
//     void Free(void * addr);
//     MemoryPool();
//     ~MemoryPool();

// private:
//     void NewMemoryBlock();
//     void DeleteRedundantMemoryBlock();
    
//     void AddFreeMemoryBlock(MemoryBlock *memory_block);


// private:
//     MemoryBlock *free_memory_block_head_;
//     MemoryBlock *free_memory_block_tail_;
//     char *free_memory_addr_;
//     size_t head_memory_block_free_size_;
//     size_t total_memory_block_count_;
//     size_t free_memory_block_count_;
//     std::unordered_map<void *, MemoryBlock *> *memory_ascription_;
//     // test only
//     size_t total_new_count;
//     size_t total_free_count;
//     //test only

// };


// template<size_t BlockSize>
// MemoryPool<BlockSize>::MemoryPool() {
//     free_memory_block_head_ = new MemoryBlock();
//     free_memory_block_tail_ = new MemoryBlock();
//     free_memory_block_head_->next = free_memory_block_tail_;
//     free_memory_addr_ = free_memory_block_head_->buffer;
//     total_memory_block_count_ = 2;
//     free_memory_block_count_ = 1;
//     head_memory_block_free_size_ = BlockSize;
//     memory_ascription_ = new std::unordered_map<void *, MemoryBlock *>();
//     //test only
//     total_new_count = 2;
//     total_free_count = 0;
//     //test only
// }

// template<size_t BlockSize>
// MemoryPool<BlockSize>::~MemoryPool() {
//     //test only
//     int count = 0;
//     //test only
//     std::set<MemoryBlock *>memory_block_ptr_set;
//     for(auto memory_ascription_iterator : *memory_ascription_) {
//         memory_block_ptr_set.insert(memory_ascription_iterator.second);
//         //test only
//         count++;
//         //test only
//     }
//     //test only
//     printf("count =%d\n",count);
//     //test only
//     delete memory_ascription_;
//     MemoryBlock * next = nullptr;
//     while(free_memory_block_head_ != nullptr) {
//         next = free_memory_block_head_->next;
//         memory_block_ptr_set.insert(free_memory_block_head_);
//         free_memory_block_head_ = next;
//     }
//     total_free_count+= memory_block_ptr_set.size();
//     for(auto set_iterator : memory_block_ptr_set) {
//         delete set_iterator;
//     }
//     //test only
//     printf("stay size = %lu,total_new_count = %lu, total_free_count = %lu\n",total_memory_block_count_,total_new_count,total_free_count);
    
//     //test only
// }

// template<size_t BlockSize>
// void MemoryPool<BlockSize>::DeleteRedundantMemoryBlock(){
//     if(total_memory_block_count_ - free_memory_block_count_ + 1 < free_memory_block_count_ * 2 && 
//        free_memory_block_count_ > 2) {
//         for(size_t i = 0; i < 2; i++) {
//             MemoryBlock * next = free_memory_block_head_->next->next;
//             delete free_memory_block_head_->next;
//             free_memory_block_count_--;
//             total_memory_block_count_--;
//             //test only
//             total_free_count++;
//             //test only
//             free_memory_block_head_->next = next;
//         }
//     }
// }

// template<size_t BlockSize>
// void MemoryPool<BlockSize>::AddFreeMemoryBlock(MemoryBlock *memory_block) {
//     free_memory_block_tail_->next = memory_block;
//     free_memory_block_tail_ = free_memory_block_tail_->next;
//     free_memory_block_count_++;
// }

// template<size_t BlockSize>
// void MemoryPool<BlockSize>::NewMemoryBlock() {
//     AddFreeMemoryBlock(new MemoryBlock());

//     total_memory_block_count_++;
//     //test only
//     //printf("total count = %lu\n",total_memory_block_count_);
//     total_new_count++;
//     //test only
// }

// template<size_t BlockSize>
// void *MemoryPool<BlockSize>::Malloc(size_t size) {
//     size_t need_size = size + sizeof(size_t);
//     if(need_size > BlockSize) {

//         perror("MemoryPool malloc size > block size!!!");
//     }
//     if(head_memory_block_free_size_ < need_size) {
//         free_memory_block_head_ = free_memory_block_head_->next;
//         free_memory_addr_ = free_memory_block_head_->buffer;
//         head_memory_block_free_size_ = BlockSize;
//         free_memory_block_count_--;
//         if(free_memory_block_head_ == free_memory_block_tail_) {
//             NewMemoryBlock();
//         }
//     }
//     memcpy(free_memory_addr_,&need_size, sizeof(size_t));
//     void * malloc_memory_addr_ = (void *)(free_memory_addr_ + sizeof(size_t));
//     free_memory_addr_ += need_size;
//     free_memory_block_head_->free_size -= need_size;
//     head_memory_block_free_size_ -= need_size;
//    // (*memory_ascription_)[malloc_memory_addr_] = free_memory_block_head_;
//     DeleteRedundantMemoryBlock();
//     return malloc_memory_addr_;
// }

// template<size_t BlockSize>
// void MemoryPool<BlockSize>::Free(void *addr) {
//    // size_t malloc_size;
//    // memcpy(&malloc_size, ((char*)addr - sizeof(size_t)), sizeof(size_t));
//     // free_memory_block_head_->free_size = BlockSize;
//     // free_memory_addr_ = free_memory_block_head_->buffer;
//     // head_memory_block_free_size_ = BlockSize;

//     auto memory_ascription_iterator = memory_ascription_->find(addr);
//     if(memory_ascription_iterator == memory_ascription_->end()) {
//         return;
//     }
//     size_t malloc_size;
//     memcpy(&malloc_size, ((char*)addr - sizeof(size_t)), sizeof(size_t));
//     memory_ascription_iterator->second->free_size += malloc_size;
//     memory_ascription_->erase(memory_ascription_iterator);
//     if(memory_ascription_iterator->second->free_size == BlockSize) {
//         if(memory_ascription_iterator->second == free_memory_block_head_) {
//             head_memory_block_free_size_ = BlockSize;
//             free_memory_addr_ = free_memory_block_head_->buffer;
//         }else {
//             memory_ascription_iterator->second->next = nullptr;
//             AddFreeMemoryBlock(memory_ascription_iterator->second);
//         }
//     }
// }

#endif