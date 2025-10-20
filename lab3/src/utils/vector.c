#include "utils/vector.h"

#include <stdlib.h>
#include <string.h>

void vector_init(Vec_t* vec, size_t initial_capacity, size_t element_size) {
    void* data = malloc(initial_capacity * element_size);
    if (data == NULL) {
        vec->error = VECTOR_ALLOCATION_FAILURE;
        return;
    }
    vec->data = data;
    vec->size = 0;
    vec->element_size = element_size;
    vec->capacity = initial_capacity;
    vec->error = VECTOR_SUCCESS;
}

void vector_resize(Vec_t* vec, size_t additional_elements) {
    size_t new_capacity = vec->capacity + additional_elements;
    void* new_data = realloc(vec->data, new_capacity * vec->element_size);
    if (new_data == NULL) {
        vec->error = VECTOR_ALLOCATION_FAILURE;
        return;
    }
    vec->data = new_data;
    vec->capacity = new_capacity;
    vec->error = VECTOR_SUCCESS;
}

void vector_push_back(Vec_t* vec, const void* element) {
    if (vec->size >= vec->capacity) {
        vector_resize(vec, vec->capacity > 0 ? vec->capacity : 1);
        if (vec->error != VECTOR_SUCCESS) {
            return;
        }
    }
    memcpy((char*)vec->data + vec->size * vec->element_size, element, vec->element_size);
    vec->size++;
}

void* vector_pop_back(Vec_t* vec, void* data_new_location) {
    if (vec->size == 0) {
        return NULL;
    }
    memcpy(data_new_location, (char*)vec->data + (vec->size - 1) * vec->element_size, vec->element_size);
    vec->size--;
    return data_new_location;
}

void* vector_get(Vec_t* vec, size_t index) {
    if (index >= vec->size) {
        return NULL;
    }
    return (char*)vec->data + index * vec->element_size;
}

size_t vector_size(const Vec_t* vec) {
    return vec->size;
}

size_t vector_element_size(const Vec_t* vec) {
    return vec->element_size;
}

void vector_free(Vec_t* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
    vec->error = VECTOR_SUCCESS;
}

void vector_free_empty(Vec_t* vec) {
    if (vec->data != NULL) {
        vec->error = VECTOR_NOT_EMPTY;
        return;
    }
    free(vec);
}

void* vector_find(const Vec_t* vec, const void* target, int (*cmp_func)(const void*, const void*)) {
    for (size_t i = 0; i < vec->size; i++) {
        void* current_element = (char*)vec->data + i * vec->element_size;
        if (cmp_func(current_element, target) == 0) {
            return current_element;
        }
    }
    return NULL;
}

void vector_print(const Vec_t* vec, void (*print_func)(const void*)) {
    for (size_t i = 0; i < vec->size; i++) {
        void* element = (char*)vec->data + i * vec->element_size;
        print_func(element);
    }
}