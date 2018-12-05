#ifndef MEMORY_POLL_H_
#define MEMORY_POLL_H_


#include<stddef.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<algorithm>


template<class T>
class Queue {
public:
    Queue(){
        capacity_ = 128 * 128;
        array_ = (T *)malloc(capacity_ * sizeof(T));
        head_ = 0;
        tail_ = 0;
        mod_ = capacity_ - 1;
    }

    ~Queue() {
        free((void *)array_);
    }

    void Resize(bool bigger) {
        T *new_array = (T *)malloc((bigger ? capacity_ << 1 : capacity_ >> 1) * sizeof(T));
        if(head_ < tail_) {
            memcpy(new_array, &array_[head_], (tail_ - head_) * sizeof(T));
            tail_ = tail_ - head_;
        }else {
            memcpy(new_array, &array_[head_], (capacity_ - head_) * sizeof(T));
            memcpy(new_array + (capacity_ - head_), array_, tail_ * sizeof(T));
            tail_ += capacity_ - head_;
        }
        head_ = 0;
        capacity_ = bigger ? capacity_ << 1 : capacity_ >> 1;
        mod_ = capacity_ - 1;
        free((void *)array_);
        array_ = new_array;
        
    }

    void offer(T &data) {
        array_[tail_++] = data;
        tail_ &= mod_;
        if(tail_ == head_) {
            Resize(true);
        }
    }

    T poll() {
        if((head_ <= tail_ ? tail_ - head_ : capacity_ - head_ + tail_) < (capacity_ >> 2) && capacity_ > 128) {
            Resize(false);
        }
        T &data = array_[head_];
        head_ != tail_ ? (head_ = head_ + 1 & mod_) : throw("Queue has no element!");
        return data;
    }
    
    size_t size() {
        return tail_ >= head_ ? tail_ - head_ : capacity_ - head_ + tail_;
    }
    size_t capacity() {
        return capacity_;
    }

    void AdjustQueue() {
        while(head_ > tail_) {
            array_[tail_] = array_[head_];
            head_ = head_ + 1 & mod_;
            tail_ = tail_ + 1 & mod_;
        }
    }
    T Back() {
        AdjustQueue();
        return array_[tail_ - 1];
    }

    T *Begin() {
        AdjustQueue();
        return &array_[head_];
    }

    T *End() {
        AdjustQueue();
        return &array_[tail_];
    }

private:
    T *array_;
    size_t mod_;
    size_t capacity_;
    size_t head_;
    size_t tail_;
};



template<class obj, size_t pool_size>
class MemoryPool {
public:
    obj *New();
    obj *New2();
    bool Delete2(obj *buffer);
    bool Delete(obj *buffer);
    void DeleteRedundantMemoryBlock();
    MemoryPool();
    ~MemoryPool();
private:
    obj *NewBuffer();

private:
    Queue<obj *> *buffer_queue_;
    Queue<char *> *memory_block_queue_;
    size_t memory_block_unit_count_;

    size_t free_buffer_count_;
    void *free_buffer_head_;
};


template<class obj, size_t pool_size>
MemoryPool<obj, pool_size>::MemoryPool() {
    buffer_queue_ = new Queue<obj *>();
    memory_block_queue_ = new Queue<char *>();
    memory_block_unit_count_ = -1;

    free_buffer_count_ = 0;
    free_buffer_head_ = nullptr;
}

template<class obj,size_t pool_size>
MemoryPool<obj, pool_size>::~MemoryPool() {
    // if(buffer_queue_->size() + memory_block_unit_count_ + 1 != memory_block_queue_->size() * pool_size) {
    //     throw ("Memory leak exists");
    // }
    while(memory_block_queue_->size()) {
        free((void *)memory_block_queue_->poll());
    }
    delete buffer_queue_;
    delete memory_block_queue_;
}


template<class obj,size_t pool_size>
obj *MemoryPool<obj, pool_size>::NewBuffer() {
    if(memory_block_unit_count_ == -1) {
        //alloc a new memory block
        char *buffer_ptr = (char *)malloc(sizeof(obj) * pool_size);
        memory_block_queue_->offer(buffer_ptr);
        memory_block_unit_count_ = pool_size - 1;
    }
    return (obj *)((memory_block_queue_->Back()) + (memory_block_unit_count_--) * sizeof(obj));
}

template<class obj,size_t pool_size>
obj *MemoryPool<obj, pool_size>::New() {
    if(buffer_queue_->size() == 0) {
        return new(NewBuffer())obj();
    }
    return new(buffer_queue_->poll())obj();
}

template<class obj,size_t pool_size>
obj *MemoryPool<obj, pool_size>::New2() {
    if(free_buffer_count_ == 0) {
        return new(NewBuffer())obj();
    }
    obj *buffer = (obj*)free_buffer_head_;
    memcpy(&free_buffer_head_, free_buffer_head_, sizeof(free_buffer_head_));
    free_buffer_count_--;
    return new(buffer)obj();
}


template<class obj,size_t pool_size>
bool MemoryPool<obj, pool_size>::Delete(obj *buffer) {
    buffer->~obj();
    buffer_queue_->offer(buffer);
    return buffer_queue_->size() > (memory_block_queue_->size() * pool_size >> 2);
}

template<class obj,size_t pool_size>
bool MemoryPool<obj, pool_size>::Delete2(obj *buffer) {
    puts("asd");
    buffer->~obj();
    *((long long *)buffer) = (long long)free_buffer_head_;
   // memcpy(buffer, &free_buffer_head_, sizeof(free_buffer_head_));
    puts("adasda");
    free_buffer_head_ = buffer;
    return ++free_buffer_count_ > (memory_block_queue_->size() * pool_size >> 2);
}


template<class obj,size_t pool_size>
void MemoryPool<obj, pool_size>::DeleteRedundantMemoryBlock() {
    while(memory_block_unit_count_ != -1) {
        obj *buffer =(obj *)((*memory_block_queue_->End()) + (memory_block_unit_count_--) * sizeof(obj));
        buffer_queue_->offer(buffer);
    }
    std::sort(buffer_queue_->Begin(), buffer_queue_->End());
    size_t size = memory_block_queue_->size();
    size_t need_erase_size = size >> 1;
    while(size && need_erase_size != 0) {
        size--;
        
    }
    //TODO::binary search
}











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
//     long long count = 0;
//     //test only
//     std::set<MemoryBlock *>memory_block_ptr_set;
//     for(auto memory_ascription_iterator : *memory_ascription_) {
//         memory_block_ptr_set.insert(memory_ascription_iterator.second);
//         //test only
//         count++;
//         //test only
//     }
//     //test only
//     prlong longf("count =%d\n",count);
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
//     prlong longf("stay size = %lu,total_new_count = %lu, total_free_count = %lu\n",total_memory_block_count_,total_new_count,total_free_count);
    
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
//     //prlong longf("total count = %lu\n",total_memory_block_count_);
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
//     unsigned short x = 2;
//    // memcpy(free_memory_addr_,&x, 2);
//     *((unsigned short *)free_memory_addr_) = x;
//     void * malloc_memory_addr_ = (void *)(free_memory_addr_ + sizeof(size_t));
//     free_memory_addr_ += need_size;
//     free_memory_block_head_->free_size -= need_size;
//     head_memory_block_free_size_ -= need_size;
//     //(*memory_ascription_)[malloc_memory_addr_] = free_memory_block_head_;
//     //DeleteRedundantMemoryBlock();
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