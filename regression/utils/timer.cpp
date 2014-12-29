#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/syscall.h>

#include "timer.h"

static void
state_query_callback(int sig, siginfo_t *si, void *uc)
{
    printf("state_query_callback %d\n", sig);
}

static void
default_callback(int sig, siginfo_t *si, void *uc)
{
    printf("Caught signal %d\n", sig);
}

int 
init_timer(Timer* timer)
{
    timer->sev.sigev_notify = SIGEV_THREAD_ID;
    timer->sev._sigev_un._tid = syscall(SYS_gettid);
    timer->sev.sigev_signo = SIGRTMIN;
    timer->sev.sigev_value.sival_ptr = &(timer->timerid);
    if (timer_create(CLOCK_REALTIME, &(timer->sev), &(timer->timerid)) == -1)
        return -1;
    return 0;
}


int
register_timer_callback(Timer* timer,
                  void (*callback)(int, siginfo_t *, void *))
{

    timer->sa.sa_flags = SA_SIGINFO;
    timer->sa.sa_sigaction = callback;
    sigemptyset(&(timer->sa.sa_mask));
    if (sigaction(SIGRTMIN, &(timer->sa), NULL) == -1)
        return -1;
    return 0;
}


int 
start_timer(Timer *timer, long long interval_nanosecs)
{

    timer->its.it_value.tv_sec = interval_nanosecs / 1000000000;
    timer->its.it_value.tv_nsec = interval_nanosecs % 1000000000;
    timer->its.it_interval.tv_sec = timer->its.it_value.tv_sec;
    timer->its.it_interval.tv_nsec = timer->its.it_value.tv_nsec;

    if (timer_settime(timer->timerid, 0, &(timer->its), NULL) == -1)
        return -1;
    return 0;

}

int 
delete_timer(Timer *timer)
{
    register_timer_callback(timer, default_callback);
    if (timer_delete(timer->timerid) == -1) 
        return -1;
    return 0;
}
