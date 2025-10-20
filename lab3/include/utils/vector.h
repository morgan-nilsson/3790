#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

enum VectorError {
    VECTOR_SUCCESS = 0,
    VECTOR_ALLOCATION_FAILURE = 1,
    VECTOR_NOT_EMPTY = 2
};

struct vec {
    void* data;
    size_t size;
    size_t element_size;
    size_t capacity;
    enum VectorError error;
};
typedef struct vec Vec_t;

/**
 * Initializes a vector with a specified initial capacity and element size.
 */
void vector_init(Vec_t* vec, size_t initial_capacity, size_t element_size);

/**
 * Resizes the vector to a new capacity.
 */
void vector_resize(Vec_t* vec, size_t additional_elements);

/**
 * Adds an element to the end of the vector, resizing if necessary.
 */
void vector_push_back(Vec_t* vec, const void* element);

/**
 * Removes the last element from the vector.
 */
void* vector_pop_back(Vec_t* vec, void* data_new_location);

/**
 * Retrieves a pointer to the element at the specified index.
 */
void* vector_get(Vec_t* vec, size_t index);

/**
 * Returns the current number of elements in the vector.
 */
size_t vector_size(const Vec_t* vec);

/**
 * Returns the size of each element in the vector.
 */
size_t vector_element_size(const Vec_t* vec);

/**
 * Frees the memory allocated for the vector and its elements.
 */
void vector_free(Vec_t* vec);

/**
 * Frees the memory allocated for the vector without freeing individual elements.
 */
void vector_free_empty(Vec_t* vec);


/**
 * Finds an element in the vector using a comparison function.
 * Returns a pointer to the found element or NULL if not found.
 */
void* vector_find(const Vec_t* vec, const void* target, int (*cmp)(const void*, const void*));

/**
 * Prints the contents of the vector using a provided print function.
 */
void vector_print(const Vec_t* vec, void (*print_func)(const void*));

#endif