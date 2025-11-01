// Copyright 2025 Morgan Nilsson

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define write_string(str) \
    printf("%s", str)

#define mem_alloc(size) \
    malloc(size)

typedef void (*task_entry_t)(void);

#define SAMPLE_TASK_PATH "./build/prog.out"

#define TIME_SEGMENT_MS 100
#define TASK_STACK_SIZE 4096

typedef struct task {
    uint32_t pid;
    uint32_t state;
    uint32_t priority;
    uint32_t time;
    uint32_t stack_start;
    char *c;
    struct task *next;
} task_t;

typedef struct process_queue {
    task_t *current_task;
    task_t *head;
    task_t *tail;
    bool active;
    uint32_t time_share_remaining;
} process_queue_t;

typedef enum {
    TASK_NEW,
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_TERMINATED
} task_state_t;

void activate_scheduler();
void scheduler_timer_callback();
void add_task(char c);

#endif