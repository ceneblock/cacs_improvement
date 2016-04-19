/**
  @author ceneblock
  @brief header file for cacs_improvements. Really, this is clean up main.c
*/
#include <getopt.h> ///for longopts
#ifndef PACKAGE_STRING
#define PACKAGE_STRING "test program"
#define PACKAGE_NAME "test"
#endif

int exit_status = EXIT_SUCCESS;
int DEBUG = 0;

static const struct option longopts[] = {
  {"help", no_argument, NULL, 'h'},
  {"program", required_argument, NULL, 'p'},
  {"config", required_argument, NULL, 'c'},
  {"debug", no_argument, NULL, 'd'},
  {"version", no_argument, NULL, 'v'},
  {NULL, 0, NULL, 0}
};

struct file_vec_struct
{
  FILE *file;
  unsigned int priority;
};

void handle_death(int status, void *pid);
long get_syscall(pid_t child);
void do_parent(char *conf_location, pid_t child);
void do_child(char *program_name, char *program_opts);
void print_help();
void read_conf(char *conf_location, struct file_vec_struct *file_array);
int file_vec_struct_comparison(const void *left, const void *right);
