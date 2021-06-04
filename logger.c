#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdatomic.h>
#include <pthread.h>
#include "logger.h"

static int use_count;

static int file_count;
static char* filename;
static char* general_log;

static void (*fun_log)(FILE* f);

volatile sig_atomic_t flag_signo_1 = 0;
volatile sig_atomic_t flag_signo_2 = 0;

volatile sig_atomic_t flag_stop = 1;
volatile sig_atomic_t sv_lvl = 1;

atomic_int sig_ch;

int num_of_digits(int num)
{
  return (int) floor(log10(abs(num))) + 1;
}

struct tm* datetime_struct()
{
  time_t ltime;
  ltime = time(NULL);
  return localtime(&ltime);
}

void handler_signo_1(int signo, siginfo_t* info, void* other)
{
  flag_signo_1 = 1;
  atomic_store(&sig_ch, info->si_value.sival_int);
}

void handler_signo_2(int signo, siginfo_t* info, void* other)
{
  flag_signo_2 = 1;
  atomic_store(&sig_ch, info->si_value.sival_int);
}

int set_log(unsigned int sv, const char* s)
{
  if (sv_lvl < sv) return 1;
  else if (!flag_stop) return 2;
  FILE* f = fopen(general_log, "a");
  if (!f)
  {
    perror("Error writing to general log file");
    return 1;
  }
  fprintf(f, "%s", s);
  switch (sv)
  {
    case 1:
    {
      int hour = datetime_struct()->tm_hour;
      int min = datetime_struct()->tm_min;
      int sec = datetime_struct()->tm_sec;
      fprintf(f, "\nCurrent time: %02d:%02d:%02d %s\n\n",
              hour < 12 ? hour : hour - 12, min, sec,
              hour < 12 ? "am" : "pm");
      break;
    }
    case 2:
    {
        int day = datetime_struct()->tm_mday;
        int month = datetime_struct()->tm_mon + 1;
        int year = datetime_struct()->tm_year + 1900;
        fprintf(f, "\nCurrent date: %02d/%02d/%d\n\n", day, month, year);
        break;
    }
    case 3:
    {
      fprintf(f, "\nFull timestamp: %s\n", asctime(datetime_struct()));
      break;
    }
    default:
      fprintf(stderr, "Invalid severity level\n");
      break;
  }
  fclose(f);
  return 0;
}

void sv_state_switch()
{
  int choice = atomic_load(&sig_ch);
  if (choice < 0 || choice > 3)
  {
    fprintf(stderr, "Invalid option\n");
    return;
  }
  else if (choice == 0) sv_lvl = sv_lvl == 0;
  else sv_lvl = choice;
}

void save_one_log()
{
  if (atomic_load(&sig_ch) == -1)
  {
    flag_stop = 0;
    return;
  }
  char* file = malloc(strlen(filename) + num_of_digits(++file_count) + 4 + 1);
  if (!file)
  {
    fprintf(stderr, "Error creating one log file: %s\n", file);
    return;
  }
  sprintf(file, "%s%d.log", filename, file_count);
  FILE* f = fopen(file, "w");
  if (!f)
  {
    fprintf(stderr, "Error creating one log file: %s\n", file);
    free(file);
    return;
  }
  fun_log(f);
  fclose(f);
  free(file);
}

void* thread_handle(void* arg)
{
  for(;;)
  {
    if (flag_stop == 0)
    {
      use_count--;
      free(general_log);
      return NULL;
    }
    if (flag_signo_1)
    {
      if (flag_stop) sv_state_switch();
      flag_signo_1 = 0;
    }
    if (flag_signo_2)
    {
      if (flag_stop) save_one_log();
      flag_signo_2 = 0;
    }
    sleep(1);
  }
}

int init(int signo_1, int signo_2, char* log, void (*fun)(FILE*))
{
  if (use_count)
  {
    fprintf(stderr, "Library already in use\n");
    return 1;
  }
  if (!log || signo_1 == signo_2)
  {
    fprintf(stderr, "Invalid parameters\n");
    return 1;
  }
  use_count++;
  atomic_store(&sig_ch, 0);
  filename = log;
  general_log = malloc(strlen(filename) + 4 + 1);
  if (!general_log)
  {
    perror( "Error initializing library");
    return 1;
  }
  snprintf(general_log, sizeof(general_log), "%s.log", filename);
  fun_log = fun;
  struct sigaction act;
  sigset_t set;
  sigfillset(&set);
  act.sa_sigaction = handler_signo_1;
  act.sa_mask = set;
  act.sa_flags = SA_SIGINFO;
  sigaction(signo_1, &act, NULL);
  act.sa_sigaction = handler_signo_2;
  act.sa_mask = set;
  act.sa_flags = SA_SIGINFO;
  sigaction(signo_2, &act, NULL);
  pthread_t tid;
  pthread_create(&tid, NULL, thread_handle, NULL);
  printf("PID: %d\n", getpid());
}