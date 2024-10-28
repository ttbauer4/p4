#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "test_helper.h"

int
main(int argc, char* argv[])
{
    struct pstat ps;

    int pa_tickets = 4;
    ASSERT(settickets(pa_tickets) != -1, "settickets syscall failed in parent");

    int pid = fork();

    // Child has double the tickets of the parent (default = 8)
    int ch_tickets = 8;

    if (pid == 0) {
        // This just has to be large enough that we know it doesn't terminate
        // before the parent gets some information - note that they should 
        // run proportional to their tickets, so it's not completely random
        // One can pick the number and add some safe margin to that
        int rt = 1000;
        run_until(rt);
        exit();
    }

    int my_idx = find_my_stats_index(&ps);
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");
    int ch_idx = find_stats_index_for_pid(&ps, pid);
    ASSERT(ch_idx != -1, "Could not get child process stats from pgetinfo");

    ASSERT(ps.tickets[my_idx] == pa_tickets, "Parent tickets should be set to %d, \
but got %d from pgetinfo", pa_tickets, ps.tickets[my_idx]);
    ASSERT(ps.tickets[ch_idx] == ch_tickets, "Child tickets should be set to %d, \
but got %d from pgetinfo", ch_tickets, ps.tickets[ch_idx]);

    int old_rtime = ps.rtime[my_idx];
    int old_pass = ps.pass[my_idx];

    int old_ch_rtime = ps.rtime[ch_idx];
    int old_ch_pass = ps.pass[ch_idx];
    
    int extra = 40;
    run_until(old_rtime + extra);

    my_idx = find_my_stats_index(&ps);
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");
    ch_idx = find_stats_index_for_pid(&ps, pid);
    ASSERT(ch_idx != -1, "Could not get child process stats from pgetinfo");

    int now_rtime = ps.rtime[my_idx];
    int now_pass = ps.pass[my_idx];

    int now_ch_rtime = ps.rtime[ch_idx];
    int now_ch_pass = ps.pass[ch_idx];

    ASSERT(now_pass > old_pass, "Pass didn't increase: old_pass was %d, \
new_pass is %d", old_pass, now_pass);

    ASSERT(now_ch_pass > old_ch_pass, "Child pass didn't increase: old_pass was %d, \
new_pass is %d", old_ch_pass, now_ch_pass);
    

    int diff_rtime = now_rtime - old_rtime;
    int __attribute__((unused)) diff_pass = now_pass - old_pass;
    int diff_ch_rtime = now_ch_rtime - old_ch_rtime;
    int __attribute__((unused)) diff_ch_pass = now_ch_pass - old_ch_pass;

    int exp_rtime = (diff_ch_rtime * pa_tickets) / ch_tickets;

    int margin = 2;
    ASSERT(diff_rtime <= exp_rtime + margin && diff_rtime >= exp_rtime - margin,
            "Parent got %d ticks, child got %d ticks, parent should be within a \
%d margin of half the child ticks", diff_rtime, diff_ch_rtime, margin);


    test_passed();

    wait();

    exit();
}
