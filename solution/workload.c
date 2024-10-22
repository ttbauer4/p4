#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "fcntl.h"

#define INITIAL_PROCESSES 10
#define ADDITIONAL_PROCESSES 6
#define TOTAL_PROCESSES (INITIAL_PROCESSES+ADDITIONAL_PROCESSES)
#define WORKLOAD_TIME 100000000

#define CSVHEADER "Time,PID,Tickets,Pass,Stride,Runtime\n"
#define MAX_INT_STR_LENGTH 12   // Max length for integer string representation

#ifdef STRIDE
#define CSVFILE "stride_process_stats.csv"
#else
#define CSVFILE "rr_process_stats.csv"
#endif

void long_running_task(long duration);
void measure(int counter, int start_time, int fd);
void itoa(int n, char* s);
void write_csv_line(int fd, int current_time, int pid, int tickets, int pass, int stride, int runtime);

int main() {
  int i;
  int pid;

  int fd = open(CSVFILE, O_CREATE | O_WRONLY);
  if (fd < 0) {
    printf(1, "Failed to open file for writing\n");
    exit();
  }
  write(fd, CSVHEADER, strlen(CSVHEADER));

  int ticket_values[TOTAL_PROCESSES];
  // Assign tickets for the initial processes
  ticket_values[0] = 1;
  for (i = 1; i < INITIAL_PROCESSES; i++) {
    ticket_values[i] = ticket_values[i - 1] * 2; // Tickets: 2, 4, 8, 16, 32, 32, 32, 32, 32
  }

  // Assign different tickets for the additional processes
  ticket_values[INITIAL_PROCESSES] = 128;
  for (i = INITIAL_PROCESSES + 1; i < TOTAL_PROCESSES; i++) {
    ticket_values[i] = ticket_values[i - 1] / 4; // Tickets: 32, 8, 2, 0, 0, 0
  }

  printf(1, "Starting initial %d processes.\n", INITIAL_PROCESSES);
  for (i = 0; i < INITIAL_PROCESSES; i++) {
    pid = fork();
    if (pid < 0) {
      printf(1, "Fork failed\n");
      exit();
    }
    else if (pid == 0) {
      settickets(ticket_values[i]);
      long_running_task(WORKLOAD_TIME);
      exit();
    }
  }


  int start_time = uptime();
  // Allow initial processes to run for a while
  sleep(100);

  measure(20, start_time, fd);

  printf(1, "Adding additional %d processes with different tickets.\n", ADDITIONAL_PROCESSES);
  for (i = INITIAL_PROCESSES; i < TOTAL_PROCESSES; i++) {
    pid = fork();
    if (pid < 0) {
      printf(1, "Fork failed\n");
      exit();
    }
    else if (pid == 0) {
      settickets(ticket_values[i]); // Set tickets for this process
      long_running_task(WORKLOAD_TIME);
      exit();
    }
  }


  measure(20, start_time, fd);
  printf(1, "Done measuring.\n");

  if (fd >= 0) {
    close(fd);
  }

  for (i = 0; i < TOTAL_PROCESSES; i++) {
    wait();
  }
  printf(1, "All child processes have completed.\n");
  exit();
}


void measure(int counter, int start_time, int fd) {
  struct pstat ps;
  while (counter) {
    if (getpinfo(&ps) != 0) {
      printf(1, "Failed to get process info\n");
      exit();
    }

    // Display process statistics
    int curr_time = uptime() - start_time;
    printf(1, "\nProcess statistics at time %d ticks:\n", curr_time);
    printf(1, "PID\tTickets\tPass\tStride\tRuntime\n");
    for (int i = 0; i < NPROC; i++) {
      if (ps.inuse[i]) {
        printf(1, "%d\t%d\t%d\t%d\t%d\n",
          ps.pid[i],
          ps.tickets[i],
          ps.pass[i],
          ps.stride[i],
          ps.rtime[i]);

        write_csv_line(fd,
          curr_time,
          ps.pid[i],
          ps.tickets[i],
          ps.pass[i],
          ps.stride[i],
          ps.rtime[i]);
      }
    }

    sleep(50); // Collect data every 50 ticks
    counter--;
  }
}

void long_running_task(long duration) {
  volatile long i, j;
  for (i = 0; i < duration; i++) {
    for (j = 0; j < duration; j++) {
      // Busy-wait loop to consume CPU time
    }
  }
}


void write_csv_line(int fd, int current_time, int pid, int tickets, int pass, int stride, int runtime) {
  char buffer[256];
  int offset = 0;
  char temp_str[MAX_INT_STR_LENGTH];

  // Write Time
  itoa(current_time, temp_str);
  strcpy(buffer + offset, temp_str);
  offset += strlen(temp_str);
  buffer[offset++] = ',';

  // Write PID
  itoa(pid, temp_str);
  strcpy(buffer + offset, temp_str);
  offset += strlen(temp_str);
  buffer[offset++] = ',';

  // Write Tickets
  itoa(tickets, temp_str);
  strcpy(buffer + offset, temp_str);
  offset += strlen(temp_str);
  buffer[offset++] = ',';

  // Write Pass
  itoa(pass, temp_str);
  strcpy(buffer + offset, temp_str);
  offset += strlen(temp_str);
  buffer[offset++] = ',';

  // Write Stride
  itoa(stride, temp_str);
  strcpy(buffer + offset, temp_str);
  offset += strlen(temp_str);
  buffer[offset++] = ',';

  // Write Runtime
  itoa(runtime, temp_str);
  strcpy(buffer + offset, temp_str);
  offset += strlen(temp_str);

  // Add newline
  buffer[offset++] = '\n';

  // Null-terminate the string
  buffer[offset] = '\0';

  // Write to file
  write(fd, buffer, offset);
}

void itoa(int n, char* s) {
  int i = 0;
  int is_negative = 0;
  if (n == 0) {
    s[i++] = '0';
    s[i] = '\0';
    return;
  }
  if (n < 0) {
    is_negative = 1;
    n = -n;
  }
  while (n > 0) {
    s[i++] = (n % 10) + '0';
    n /= 10;
  }
  if (is_negative) {
    s[i++] = '-';
  }
  s[i] = '\0';
  // Reverse the string
  int start = 0;
  int end = i - 1;
  char temp;
  while (start < end) {
    temp = s[start];
    s[start] = s[end];
    s[end] = temp;
    start++;
    end--;
  }
}

