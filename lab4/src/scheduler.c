// Copyright 2025 Morgan Nilsson

#include "scheduler.h"
#include "logger.h"

process_queue_t process_queue;

// start or stop thread
void switch_task(task_t *next_task) {

    if (next_task->state == TASK_NEW) {

        pid_t pid = fork();
        if (pid < 0) {

            write_string("Error creating new process\n");
            return;

        } else if (pid == 0) {
            // dispatch child
            next_task->state = TASK_RUNNING;

            execl(SAMPLE_TASK_PATH, "./task_process", next_task->c, NULL);
            write_string("Error starting task process\n");
            exit(1);
        } else {
            // parent process
            next_task->pid = pid;
            LOG_MESSAGE_FORMATTED(LOG_LEVEL_INFO, "Started new task with PID %u\n", next_task->pid);
        }

    } else if (next_task->state == TASK_READY) {

        // send SIGCONT to process
        kill(next_task->pid, SIGCONT);
        next_task->state = TASK_RUNNING;

    } else if (next_task->state == TASK_RUNNING) {

        LOG_MESSAGE_FORMATTED(LOG_LEVEL_WARNING, "Task with PID %u is already running\n", next_task->pid);

    } else {
        LOG_MESSAGE_FORMATTED(LOG_LEVEL_ERROR, "Cannot switch to task with PID %u in state %u\n", next_task->pid, next_task->state);
    }

}

void scheduler_timer_callback() {

    if (process_queue.active == false) {

        return;

    }

    --(process_queue.time_share_remaining);

    // time has run out thus switch to next task
    if (process_queue.time_share_remaining < 1) {
        LOG_MESSAGE(LOG_LEVEL_INFO, "Time slice expired, switching task\n");

        kill(process_queue.current_task->pid, SIGSTOP);
        process_queue.current_task->state = TASK_READY;
        process_queue.time_share_remaining = TIME_SEGMENT_MS;

        process_queue.current_task = process_queue.current_task->next;

        switch_task(process_queue.current_task);

    }

}

void add_task(char c) {

    task_t *task = mem_alloc(sizeof(task_t));
    if (task == NULL) {

        write_string("Error allocating memory from heap\n");
        return;

    }

    task->state = TASK_NEW;
    task->c = malloc(2 * sizeof(char));
    assert(task->c != NULL);
    task->c[0] = c;
    task->c[1] = '\0';

    LOG_MESSAGE_FORMATTED(LOG_LEVEL_INFO, "Adding: task %c\n", c);

    if (process_queue.current_task == NULL) {

        task->next = task;
        
        process_queue.head = task;
        process_queue.tail = task;
        process_queue.current_task = task;

    } else {
        // put the new task at the end
        task->next = process_queue.head;

        process_queue.tail->next = task;
        process_queue.tail = task;

    }
}

void activate_scheduler() {

    if (process_queue.current_task == NULL) {

        write_string("No tasks to schedule this call will crash\n");
        while (1);

    }

    process_queue.time_share_remaining = TIME_SEGMENT_MS;

    process_queue.active = true;

    switch_task(process_queue.current_task);

}
