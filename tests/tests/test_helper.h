#include "user.h"

#define DEFAULT_TICKETS 8
#define TEST_PREFIX "P4_TESTER"

#define ASSERT(exp, msg, ...) if (!(exp)) { \
                        printf(1, "%s:%s:%d ", TEST_PREFIX, __FILE__, __LINE__); \
                        printf(1, msg, ##__VA_ARGS__); \
                        printf(1, "\n"); \
                        exit(); }

#define PRINTF(...)  printf(1, "%s: ", TEST_PREFIX); \
                        printf(1, __VA_ARGS__); \
                        printf(1, "\n");

/* 
 * Issue a getpinfo call to get latest info
 * Then, get the pstat entry index corresponding to the calling process
 * @param s a ptr to a struct that is filled by this function
 * @return matching index on success, -1 otherwise
 */
static int find_my_stats_index(struct pstat *s) {
    if (!s)
        return -1;

    int mypid = getpid();

    if (getpinfo(s) == -1)
        return -1;
    
    // Find an entry matching my pid
    for (int i = 0; i < NPROC; i++)
        if (s->pid[i] == mypid)
           return i; 

    // Could not find an entry matching my pid
    return -1;
}

/*
 * Find the index of the matching pid
 * Doesn't issue a new call - just uses the provided ptr to get info
 * @param pid The pid that will be used for match
 * @return -1 if a match is not found, the match index otherwise
 */
static __attribute__((unused)) int find_stats_index_for_pid(const struct pstat *s, int pid) {
    if (!s)
        return -1;

    // Find an entry matching my pid
    for (int i = 0; i < NPROC; i++)
        if (s->pid[i] == pid)
           return i; 

    // Could not find an entry matching my pid
    return -1;
}

#define SUCCESS_MSG "TEST PASSED"
static void test_passed() {
    PRINTF("%s", SUCCESS_MSG);
}

/*
 * Run at least until the specified target rtime
 * Might immediately return if the rtime is already reached
 */
static __attribute__((unused)) void run_until(int target_rtime) {
    struct pstat ps;
    while (1) {
        int my_idx = find_my_stats_index(&ps);
        ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

        if (ps.rtime[my_idx] >= target_rtime)
            return;
    }
}
