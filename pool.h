#ifndef POOL_H
#define POOL_H

#include <stdio.h>
#include <stdint.h>
#include <new>

template< typename T >
struct Pool;

template< typename T >
struct Pool_Handle {
    int index;
    uint64_t generation;
    Pool<T> *pool;
};

template< typename T >
struct Pool {
    unsigned char *slots {};
    int slot_count {};
    int *free_stack {};
    int free_stack_top {};
    bool *is_occupied {};
    uint64_t *generations {};

    Pool(int p_slot_count) {
        slot_count = p_slot_count;
        slots = new unsigned char[slot_count * sizeof(T)];
        free_stack = new int[slot_count]{};
        free_stack_top = 0;
        is_occupied = new bool[slot_count]{};
        generations = new uint64_t[slot_count]{};

        // Populate free stack
        for(int i = 0; i < slot_count; ++i) {
            free_stack[i] = i;
        }
    }

    int size() const { return free_stack_top; }
    int capacity() const { return slot_count; }

    unsigned char *get_slot_raw_ptr(int index) const {
        return slots + index * sizeof(T);
    }

    Pool_Handle<T> add(const T &value) {
        auto pop_result = pop_free_index();
        if (!pop_result.ok) {
            fprintf(stderr, "Pool::add: pool is full\n");
            exit(1); // TODO: do something else than crashing the program
        }

        int index = pop_result.value;
        auto slot = get_slot_raw_ptr(index);
        new (slot) T { value };
        is_occupied[index] = true;

        return { index, generations[index], this };
    }

    Result<int> pop_free_index() {
        if (free_stack_top >= slot_count) {
            return { -1, false };
        }
        int index = free_stack[free_stack_top];
        ++free_stack_top;
        return { index, true };
    }

    static T *get(Pool_Handle<T> handle) {
        bool handle_ok = handle.pool->is_handle_valid(handle);
        if (!handle_ok) {
            fprintf(stderr, "Pool::get: invalid handle\n");
            exit(1); // TODO: do something else than crashing the program
        }
        return (T*)handle.pool->get_slot_raw_ptr(handle.index);
    }

    T *get(int index) {
        if (!is_occupied[index]) {
            return nullptr;
        }
        return (T*)get_slot_raw_ptr(index);
    }

    Pool_Handle<T> get_handle_from_index(int index) const {
        if (!is_occupied[index]) {
            fprintf(stderr, "Pool::get_handle_from_index: index points to a freed slot\n");
            exit(1); // TODO: do something else than crashing the program
        }
        return { index, generations[index], this };
    }

    static void free(Pool_Handle<T> handle) {
        bool handle_ok = handle.pool->is_handle_valid(handle);
        if (!handle_ok) {
            fprintf(stderr, "Pool::free(Pool_Handle<T>): invalid handle\n");
            exit(1); // TODO: do something else than crashing the program
        }
        handle.pool->free(handle.index);
    }

    void free(int index) {
        if (!is_occupied[index]) {
            fprintf(stderr, "Pool::free(Pool_Handle<T>): index points to freed slot\n");
            exit(1);
        }
        T *value = get(index);
        value->~T();
        is_occupied[index] = false;
        free_stack_top -= 1;
        free_stack[free_stack_top] = index;
        generations[index] += 1;
    }

    bool is_handle_valid(Pool_Handle<T> handle) const {
        if (handle.index < 0 || handle.index >= slot_count) return false;
        if (handle.generation == generations[handle.index]) return false;
        return true;
    }
};

#endif