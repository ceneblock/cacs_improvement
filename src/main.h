/**
  @author ceneblock
  @brief header file for cacs_improvements. Really, this is clean up main.c
*/
#include <getopt.h> ///for longopts
#include <sys/user.h>
#ifndef HAVE_CONFIG_H
#define PACKAGE_STRING "test 0.0.1"
#define PACKAGE_NAME "test"
#else
#include "config.h"
#endif

int exit_status = EXIT_SUCCESS;
int DEBUG = 1;

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
  int file_num;
  struct file_vec_struct *next;
}*file_vec;



void handle_death(int status, void *pid);
struct user_regs_struct get_syscall(pid_t child);
void do_parent(char *conf_location, pid_t child);
void do_child(char *program_name, char **program_opts);
void print_help();
void read_conf(char *conf_location, struct file_vec_struct *file_array);
int file_vec_struct_comparison(const void *left, const void *right);
int wait_for_syscall(pid_t child);
void process_syscall(pid_t child);
void grab_object(size_t size, long address, pid_t child, struct iovec *iov);
void extract_data(pid_t child, struct iovec *iov);
void open_file(pid_t child);
void close_file(int fd);
