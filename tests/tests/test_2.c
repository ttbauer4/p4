#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "test_helper.h"

int
main(int argc, char* argv[])
{
    struct pstat ps;
    int my_idx = find_my_stats_index(&ps);
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    int old_rtime = ps.rtime[my_idx];
    int old_pass = ps.pass[my_idx];
    int old_stride = ps.stride[my_idx];
    
    int extra = 4;

    run_until(old_rtime + extra);

    my_idx = find_my_stats_index(&ps);
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    int now_rtime = ps.rtime[my_idx];
    int now_pass = ps.pass[my_idx];
    int now_stride = ps.stride[my_idx];

    ASSERT(now_pass > old_pass, "Pass didn't increase: old_pass was %d, \
new_pass is %d", old_pass, now_pass);
    
    ASSERT(old_stride == now_stride, "Stride changed from %d to %d without \
calling settickets", old_stride, now_stride);

    int diff_rtime = now_rtime - old_rtime;
    int diff_pass = now_pass - old_pass;
    int exp_pass = diff_rtime * now_stride;

    ASSERT(diff_pass == exp_pass, "Pass is not incremented correctly by stride. \
            Process got scheduled %d times, with a stride of %d, should have \
            increased the pass value by %d, while only got increased by %d",
            diff_rtime, now_stride, exp_pass, diff_pass);

    test_passed();

    exit();
}
