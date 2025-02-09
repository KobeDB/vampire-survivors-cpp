#ifndef ARENA_H
#define ARENA_H

#include <stdio.h>
#include <new>

struct Arena {

    unsigned char *slots {};
    int slot_count {};
    int slot_size {};
    int cur_slot {};

    void init(int p_slot_count, int p_slot_size) {
        slot_count = p_slot_count;
        slot_size = p_slot_size;
        slots = new unsigned char[slot_size * slot_count];
        cur_slot = 0;
    }

    template< typename T >
    T *add(const T &value) {
        if cur_slot >= slot_count {
            fprintf(stderr, "Arena::add: arena full");
            return nullptr;
        }
        unsigned char *addr = slots + slot_size * cur_slot;
        ++cur_slot;
        T *result = new (addr) T{value};
        return result;
    }
};

#endif