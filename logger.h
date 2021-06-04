#ifndef _LOGGER_H_
#define _LOGGER_H_

int init(int signo_1, int signo_2, char* log, void (*fun)(FILE*));
int set_log(unsigned int sv, const char* s);

#endif //_LOGGER_H_
