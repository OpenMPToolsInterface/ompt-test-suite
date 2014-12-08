#ifndef __OMPT__Timer__
#define __OMPT__Timer__

#include <stdio.h>
typedef struct {
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    long long freq_nanosecs;
    sigset_t mask;
    struct sigaction sa;
} Timer;

int init_timer(Timer* timer);
int register_timer_callback(Timer* timer, void (*callback)(int, siginfo_t *, void *));
int start_timer(Timer *timer, long long interval_nanosecs);
int delete_timer(Timer *timer);
#endif /* defined(__OMPT__Timer__) */
