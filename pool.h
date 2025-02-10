#ifndef POOL_H
#define POOL_H

#include <stdio.h>
#include <stdint.h>
#include <new>


//
// Raw_Pool
//
struct Raw_Pool;

template< typename T >
struct Raw_Pool_Handle {
    T *value;
    int index;
    Raw_Pool *pool;
};

// A Raw_Pool is an untyped pool allocator without generational indices
struct Raw_Pool {
    unsigned char *slots {};
    int slot_count {};
    int slot_size {};
    int *free_stack {};
    int free_stack_top {};
    bool *is_occupied {};

    Raw_Pool(int p_slot_count, int p_slot_size) {
        slot_count = p_slot_count;
        slot_size = p_slot_size;
        slots = new unsigned char[slot_count * slot_size];
        free_stack = new int[slot_count]{};
        free_stack_top = 0;
        is_occupied = new bool[slot_count]{};

        // Populate free stack
        for(int i = 0; i < slot_count; ++i) {
            free_stack[i] = i;
        }
    }

    int size() const { return free_stack_top; }
    int capacity() const { return slot_count; }

    template< typename T >
    Raw_Pool_Handle<T> add(const T &value) {
        if (sizeof(T) > slot_size) {
            fprintf(stderr, "Pool::add: value is greater than slot size");
            exit(1);
        }

        bool pop_ok {};
        int index = pop_free_index(pop_ok);
        if (!pop_ok) {
            fprintf(stderr, "Pool::add: pool is full\n");
            exit(1); // TODO: do something else than crashing the program
        }

        auto slot = get_slot_raw_ptr(index);
        auto result = new (slot) T { value };
        is_occupied[index] = true;

        return {result, index, this};
    }

    void *get(int index) {
        if (!is_occupied[index]) {
            return nullptr;
        }
        return (void*)get_slot_raw_ptr(index);
    }

    template< typename T >
    void free(Raw_Pool_Handle<T> handle) {
        // TODO: add handle validity checks
        free<T>(handle.index);
    }

    template< typename T >
    void free(int index) {
        if (!is_occupied[index]) {
            fprintf(stderr, "Pool::free(Raw_Pool_Handle<T>): index points to freed slot\n");
            exit(1);
        }
        T *value = (T*)get(index);
        value->~T();
        is_occupied[index] = false;
        free_stack_top -= 1;
        free_stack[free_stack_top] = index;
    }


    //
    // Helpers
    //
    unsigned char *get_slot_raw_ptr(int index) const {
        return slots + index * slot_size;
    }

    int pop_free_index(bool &success) {
        if (free_stack_top >= slot_count) {
            success = false;
            return -1;
        }
        int index = free_stack[free_stack_top];
        ++free_stack_top;
        success = true;
        return index;
    }
};

// END Raw_Pool
// ---------------------------------------------


//
// Pool
//
template< typename T >
struct Pool;

template< typename T >
struct Pool_Handle {
    int index;
    uint64_t generation;
    Pool<T> *pool;
};

// A typed pool with generational indices
template< typename T >
struct Pool {
    Raw_Pool pool;
    uint64_t *generations {};

    Pool(int p_slot_count) : pool{p_slot_count, sizeof(T)} {
        generations = new uint64_t[p_slot_count]{};
    }

    int size()      const { return pool.free_stack_top; }
    int capacity()  const { return pool.slot_count; }

    Pool_Handle<T> add(const T &value) {
        Raw_Pool_Handle<T> raw_handle = pool.add(value);
        auto index = raw_handle.index;
        return { index, generations[index], this };
    }

    T *get(int index) {
        return (T*)pool.get(index);
    }

    Pool_Handle<T> get_handle_from_index(int index) const {
        if (!pool.is_occupied[index]) {
            fprintf(stderr, "Pool::get_handle_from_index: index points to a freed slot\n");
            exit(1); // TODO: do something else than crashing the program
        }
        return { index, generations[index], this };
    }

    void free(int index) {
        pool.free<T>(index);
    }

    //
    // Helpers
    //
    bool is_handle_valid(Pool_Handle<T> handle) const {
        if (handle.index < 0 || handle.index >= capacity()) return false;
        if (handle.generation != generations[handle.index]) return false;
        return true;
    }
};

template< typename T >
static T *get(Pool_Handle<T> handle) {
    bool handle_ok = handle.pool->is_handle_valid(handle);
    if (!handle_ok) {
        fprintf(stderr, "Pool::get: invalid handle\n");
        exit(1); // TODO: do something else than crashing the program
    }
    return (T*)handle.pool->get(handle.index);
}

template< typename T >
void free(Pool_Handle<T> handle) {
    bool handle_ok = handle.pool->is_handle_valid(handle);
    if (!handle_ok) {
        fprintf(stderr, "Pool::free(Pool_Handle<T>): invalid handle\n");
        exit(1); // TODO: do something else than crashing the program
    }
    handle.pool->free(handle.index);
}

// END Pool
//------------------------------------------------------

#endif