#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "logger.h"

static unsigned int i = 0;

void fun(FILE* f)
{
  fprintf(f, "Current number: %d", i++);
}

int main()
{
  init(34, 35, "log", fun);
  for(;;)
  {
    if (set_log(1, "Example message for log severity MIN") == 2) return EXIT_SUCCESS;
    if (set_log(2, "Example message for log severity STANDARD") == 2) return EXIT_SUCCESS;
    if (set_log(3, "Example message for log severity MAX") == 2) return EXIT_SUCCESS;
    sleep(3);
  }
}
