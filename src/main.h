/**
  @author ceneblock
  @brief header file for cacs_improvements. Really, this is clean up main.c
*/
#include <getopt.h> ///for longopts
#include <sys/user.h>
#ifndef HAVE_CONFIG_H
#define PACKAGE_STRING "test 0.0.1"
#else
#include "config.h"
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
  char *location;
  unsigned int priority;
};

void handle_death(int status, void *pid);
struct user_regs_struct get_syscall(pid_t child);
void do_parent(char *conf_location, pid_t child);
void do_child(char *program_name, char **program_opts);
void print_help();
void read_conf(char *conf_location, struct file_vec_struct *file_array);
int file_vec_struct_comparison(const void *left, const void *right);
int wait_for_syscall(pid_t child);
void process_syscall(pid_t child);
void *grab_object(size_t size, void *address);
