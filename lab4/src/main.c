#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "unistd.h"
#include "signal.h"
#include "sys/types.h"

#include "scheduler.h"
#include "logger.h"

struct sigaction newhandler, oldhandler;
sigset_t sig;

int main(void) {

    CREATE_LOGGER("scheduler.log", LOG_LEVEL_DEBUG);

    LOG_MESSAGE(LOG_LEVEL_INFO, "=================\n STARTING SCHEDULER\n=================\n");

    for (char c = 'A'; c <= 'E'; ++c) {
        add_task(c);
    }

    activate_scheduler();

    sigemptyset(&sig);
    // call scheduler_timer_callback on alarm
    newhandler.sa_handler = scheduler_timer_callback;
    newhandler.sa_mask = sig;
    newhandler.sa_flags = 0;
    if (sigaction(SIGALRM, &newhandler, &oldhandler) < 0) {
        write_string("Error setting signal handler\n");
        return 1;
    }
    LOG_MESSAGE(LOG_LEVEL_INFO, "Installed alarm handler\n");

    while (1) {
        ualarm(10000, 10000);
        pause();
    }
}