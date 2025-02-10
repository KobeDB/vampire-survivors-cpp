#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <new>

template< typename T >
struct Array {
    unsigned char *elements = nullptr;
    int m_size = 0;
    int m_capacity = 0;

    int size() const { return m_size; }
    int capacity() const { return m_capacity; }

    T *data() { return &(this->operator[](0)); }

    void push(const T &value) {
        ensure_capacity();
        new (get_element_ptr(m_size)) T{ value };
        ++m_size;
    };

    void ensure_capacity() {
        if (!elements) {
            m_capacity = 8;
            elements = (unsigned char*)malloc(m_capacity * sizeof(T));
            assert(elements);
            return;
        }

        if (m_size >= m_capacity) {
            int new_capacity = m_capacity * 2;
            unsigned char *new_elements = (unsigned char*)malloc(new_capacity * sizeof(T));
            assert(new_elements);
            // copy over elements to new buffer
            for (int i = 0; i < m_size; ++i) {
                unsigned char *new_element_ptr = new_elements + i * sizeof(T);
                new (new_element_ptr) T{ *get_element_ptr(i) };
            }
            // destruct elements in old buffer
            destruct_elements();
            // free old buffer
            free(elements);

            m_capacity = new_capacity;
            elements = new_elements;
        }
    }

    void destroy() {
        destruct_elements();
        free(elements);
        elements = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    T &operator[](int index) {
        verify_index(index);
        return *get_element_ptr(index);
    }

    const T &operator[](int index) const {
        verify_index(index);
        return *get_element_ptr(index);
    }

    //
    // Helpers
    //
    void destruct_elements() {
        for (int i = 0; i < m_size; ++i) {
            T *element = get_element_ptr(i);
            element->~T();
        }
    }

    T *get_element_ptr(int index) {
        return (T*)(elements + index * sizeof(T));
    }

    void verify_index(int index) {
        if (index < 0 || index >= m_size) {
            fprintf(stderr, "Array: index out of bounds");
            exit(1);
        }
    }


};

#endif