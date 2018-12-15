#ifndef MEMORY_POLL_H_
#define MEMORY_POLL_H_

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <set>
template <class obj, size_t pool_size>
class MemoryPool
{
  public:
    obj *New();
    void Delete(obj *buffer);
    void DeleteRedundantMemoryBlock();
    MemoryPool();
    ~MemoryPool();

  private:
    void AllocMemoryBlock();
    bool NeedCleanMemoryBlock();
    obj *NewBuffer();

    //test
    void test();
    std::set<obj *> st;
    //test

  private:
    char *memory_block_head_;
    size_t memory_block_count_;
    size_t memory_block_unit_count_;

    size_t free_buffer_count_;
    obj *free_buffer_head_;

    size_t clean_count_;
};

template <class obj, size_t pool_size>
MemoryPool<obj, pool_size>::MemoryPool()
{
    memory_block_head_ = nullptr;
    memory_block_count_ = 0;
    memory_block_unit_count_ = 0;

    free_buffer_count_ = 0;
    free_buffer_head_ = nullptr;
    clean_count_ = 0;
}

template <class obj, size_t pool_size>
MemoryPool<obj, pool_size>::~MemoryPool()
{
    if (free_buffer_count_ + memory_block_unit_count_ != memory_block_count_ * pool_size)
    {
        perror("MemoryPool:: buffer no delete exists");
    }
    while (memory_block_head_ != nullptr)
    {
        char *next;
        memcpy(&next, memory_block_head_, sizeof(char *));
        free(memory_block_head_);
        memory_block_head_ = next;
    }
}

template <class obj, size_t pool_size>
void MemoryPool<obj, pool_size>::AllocMemoryBlock()
{
    char *buffer_ptr = (char *)malloc(sizeof(obj) * pool_size + sizeof(char *));
    if (buffer_ptr == nullptr)
    {
        throw("malloc error!");
    }
    memcpy(buffer_ptr, &memory_block_head_, sizeof(char *));
    memory_block_head_ = buffer_ptr;
    ++memory_block_count_;
    memory_block_unit_count_ = pool_size;
}

template <class obj, size_t pool_size>
obj *MemoryPool<obj, pool_size>::NewBuffer()
{
    if (memory_block_unit_count_ == 0)
    {
        AllocMemoryBlock();
    }
    --memory_block_unit_count_;
    return (obj *)(memory_block_head_ + sizeof(char *) + memory_block_unit_count_ * sizeof(obj));
}

template <class obj, size_t pool_size>
obj *MemoryPool<obj, pool_size>::New()
{
    //puts("memory_pool::do new");
    if (free_buffer_count_ == 0)
    {
        return new (NewBuffer()) obj();
    }
    obj *buffer = free_buffer_head_;
    memcpy(&free_buffer_head_, free_buffer_head_, sizeof(free_buffer_head_));
    --free_buffer_count_;
    --clean_count_;

    //test
    st.erase(buffer);
    //test

    return new (buffer) obj();
}

template <class obj, size_t pool_size>
void MemoryPool<obj, pool_size>::Delete(obj *buffer)
{
    puts("memory_pool::do delete");
    
    //test
    if(st.find(buffer) != st.end()) {
        perror("buffer have been delete!!");
        throw "buffer have been delete!!";
    }
    st.insert(buffer);
    //test

    buffer->~obj();
    memcpy(buffer, &free_buffer_head_, sizeof(free_buffer_head_));
    free_buffer_head_ = buffer;
    ++free_buffer_count_;
    ++clean_count_;
    DeleteRedundantMemoryBlock();
}

template <class obj, size_t pool_size>
bool MemoryPool<obj, pool_size>::NeedCleanMemoryBlock()
{
    return free_buffer_count_ > (memory_block_count_ * pool_size >> 2) * 3 && memory_block_count_ > 1 && clean_count_ > pool_size;
}

template <class obj, size_t pool_size>
void MemoryPool<obj, pool_size>::DeleteRedundantMemoryBlock()
{
    if (!NeedCleanMemoryBlock())
    {
        return;
    }
    size_t size = (memory_block_count_ > 1024 ? memory_block_count_ : 1024) * sizeof(char *) << 1;
    char **memory_block_addr = (char **)malloc(size);
    int *hash_count = (int *)(memory_block_addr + memory_block_count_);
    memset(hash_count, 0, memory_block_count_ * sizeof(int));

    char *head = memory_block_head_;
    for (size_t i = 0; i < memory_block_count_; i++)
    {
        memory_block_addr[i] = head;
        memcpy(&head, head, 8);
    }
    std::sort(memory_block_addr, memory_block_addr + memory_block_count_);

    obj *free_buffer = free_buffer_head_;
    while (free_buffer != nullptr)
    {
        int index = std::upper_bound(memory_block_addr, memory_block_addr + memory_block_count_, (char *)free_buffer) - memory_block_addr - 1;
        if (index < 0 || memory_block_addr[index] + sizeof(char *) + (pool_size - 1) * sizeof(obj) < (char *)free_buffer)
        {
            throw("DeleteRedundantMemoryBlock have bug");
        }
        hash_count[index]++;
        memcpy(&free_buffer, free_buffer, sizeof(char *));
    }

    size_t new_memory_block_count = 0;
    size_t delete_memory_block_count = 0;
    for (size_t i = 0; i < memory_block_count_; i++)
    {
        if (delete_memory_block_count >= (memory_block_count_ >> 1))
        {
            break;
        }
        if (hash_count[i] == pool_size)
        {
            delete_memory_block_count++;
            hash_count[i] = -1;
        }
    }

    obj *pre_buffer = nullptr;
    free_buffer = free_buffer_head_;
    while (free_buffer != nullptr)
    {
        int index = std::upper_bound(memory_block_addr, memory_block_addr + memory_block_count_, (char *)free_buffer) - memory_block_addr - 1;
        if (hash_count[index] == -1)
        {
            if (pre_buffer == nullptr)
            {
                memcpy(&free_buffer_head_, free_buffer, sizeof(char *));
            }
            else
            {
                memcpy(pre_buffer, free_buffer, sizeof(char *));
            }
        }
        else
        {
            pre_buffer = free_buffer;
        }
        memcpy(&free_buffer, free_buffer, sizeof(char *));
    }

    char *pre_memory_block = nullptr;
    char *memory_block = memory_block_head_;
    while (memory_block != nullptr)
    {
        int index = std::upper_bound(memory_block_addr, memory_block_addr + memory_block_count_, memory_block) - memory_block_addr - 1;
        if (hash_count[index] == -1)
        {
            if (pre_memory_block == nullptr)
            {
                memcpy(&memory_block_head_, memory_block, sizeof(char *));
            }
            else
            {
                memcpy(pre_memory_block, memory_block, sizeof(char *));
            }
        }
        else
        {
            pre_memory_block = memory_block;
        }
        memcpy(&memory_block, memory_block, sizeof(char *));
    }

    free_buffer_count_ -= delete_memory_block_count * pool_size;
    memory_block_count_ -= delete_memory_block_count;
    clean_count_ = 0;
    free(memory_block_addr);
}

template <class obj, size_t pool_size>
void MemoryPool<obj, pool_size>::test()
{
    int free_buffer_count = 0;
    int mem_block = 0;
    obj *ptr = free_buffer_head_;
    while (ptr != nullptr)
    {
        free_buffer_count++;
        memcpy(&ptr, ptr, 8);
    }

    char *m_ptr = memory_block_head_;
    while (m_ptr != nullptr)
    {
        mem_block++;
        memcpy(&m_ptr, m_ptr, 8);
    }
    printf("test::free_buffer_count = %d, m_count=%d,free_buffer = %lu,block_count =%lu\n", free_buffer_count, mem_block, free_buffer_count_, memory_block_count_);
}

/*
template<class T>
class Queue 
{
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
*/

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

#endif