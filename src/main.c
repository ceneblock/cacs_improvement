/**
  @author ceneblock
  @brief implements a monitor program to change where data is written to in a
  printv
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/syscall.h> ///cleaner than <asm/unistd.h>
#include <sys/user.h> ///for examining registers
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
int exit_status = EXIT_SUCCESS;
int DEBUG = 0;

#define PACKAGE_STRING "test program"
#define PACKAGE_NAME "test"

static const struct option longopts[] = {
  {"help", no_argument, NULL, 'h'},
  {"program", required_argument, NULL, 'p'},
  {"config", required_argument, NULL, 'c'},
  {"debug", no_argument, NULL, 'd'},
  {"version", no_argument, NULL, 'v'},
  {NULL, 0, NULL, 0}
};

/**
  @brief kills all the children!
  @param status what our exit status is
  @param pid the child's pid
*/
void handle_death(int status, void *pid)
{
  pid_t child = (pid_t)&pid;
    
  kill(-child, SIGKILL); ///for all subchildren to die

}

/**
  @brief gets the syscall that a child encountered
  @param child the child's process
  @return the syscall number
*/
long get_syscall(pid_t child)
{
  long syscall_num = 0;
  struct user_regs_struct registers;
  int rv = ptrace(PTRACE_GETREGS, child, NULL, registers);

  if(rv != -1)
  {
#ifdef __x86_64__
  syscall_num = registers.rax;
#else
  syscall_num = registers.eax
#endif
  }
  else
  {
    fprintf(stderr, "Error on line %s ptrace(PTRACE_GETREGS)\n", __LINE__);
    fprintf(stderr, "Error is: %s\n", strerror(errno));
    fprintf(stderr, "PANIC! Don't know how to recover! Aborting!\n");
    exit_status = EXIT_FAILURE;

    exit(exit_status);
  }
  return syscall_num;
}

/**
  @brief handles all of the parent functions (stopping on writev and
  manipulating it)
  @param child the child's process id
*/
void do_parent(pid_t child)
{
  long rv = ptrace(PTRACE_ATTACH, child, NULL, NULL);

  int status = 0;

  if(rv != -1)
  {
    ///make sure that we can inspect children\threads..
    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL | PTRACE_O_TRACECLONE
        | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEFORK | PTRACE_O_TRACESYSGOOD |
        PTRACE_O_TRACEVFORK);
    wait(&status);
    while(1)
    {
      ptrace(PTRACE_SYSCALL, child, NULL, 0);
      wait(&status);
      if(WIFSTOPPED(status))
      {
        long syscall = get_syscall(child);
      }
    }

    ptrace(PTRACE_DETACH, child, NULL, 0);
  }
  else
  {
    fprintf(stderr, "ERROR on ptrace(PTRACE_ATTACH) line: %s\n", __LINE__);
    fprintf(stderr, "Aborting and killing children\n");
    kill(-child, SIGKILL); ///for all subchildren to die
    exit_status = EXIT_FAILURE;
  }
  
}

/**
  @brief prints the help message in a lazy way
*/
void print_help()
{
  printf("%s\n", PACKAGE_STRING);
  printf("\n");
  printf("Valid Options:\n");
  printf("\n");
  int x = 0;
  for(x = 0; longopts[x].name != NULL; x++)
  {
    printf("%s\n",longopts[x].name);
  }
  printf("\n");
  printf("Example usage %s -c /etc/monitor.conf -p my_prog -param\n",
      PACKAGE_NAME);
}

int main(int argc, char **argv)
{
  char *conf_location = NULL, *program_name = NULL, *program_opts = NULL;
  int have_c = 0, have_p = 0;
  
  int opt_id = 0;
  while(opt_id != -1)
  {
    opt_id = getopt_long(argc, argv, "hp:c:vd", longopts, NULL);
    switch(opt_id)
    {
      case 'p':
        if(have_p == 0)
        {
          have_p = 1;
          size_t program_length = strlen(optarg);

          char *found = strchr(optarg, ' ');
          if(found) //for when there are arguments
          {
            size_t opts_length = found - optarg;
            size_t opts_location = strlen(optarg) - opts_length;
            program_opts = malloc(opts_length * sizeof(char));
            program_opts = strncpy(program_opts, optarg+opts_location, opts_length);

            program_name = malloc((opts_location - 1) * sizeof(char));
            strncpy(program_name, optarg, opts_location - 1);
          }
          else
          {
            program_name = malloc(program_length * sizeof(char));
            strncpy(program_name, optarg, program_length);
          }

          opt_id = -1; //break out of the while loop.
          break;
        }
        else
        {
          fprintf(stderr, "Error on line %d! Found two or more instances of \
              -p!", __LINE__);
          fprintf(stderr, "Only going to use the first one!\n");
        }
        break;
      case 'c':
        if(have_c == 0)
        {
          have_c = 1;
          size_t conf_length = strlen(optarg);
          conf_location = malloc(conf_length * sizeof(char));
          strncpy(conf_location, optarg, conf_length);
        }
        else
        {
          fprintf(stderr, "Error on line %d! Found two or more instances of \
              -c!", __LINE__);
          fprintf(stderr, "Only going to read the first one!\n");
        }
        break;
      case 'v':
        printf("%s\n", PACKAGE_STRING);
        exit(exit_status);
      case 'd':
        DEBUG = 1;
        break;
      default:
        print_help();
        exit(exit_status);
    }
  }
  if(have_c && have_p)
  {
    pid_t child = 0;
    child = fork();
    if(child > 0)
    {
       do_parent(child);
    }
    if(child == 0)
    {
      do_child(conf_location, program_name, program_opts);
    }
    else
    {
      fprintf(stderr, "Error at fork()! Errno: %s\n", strerror(errno));
      fprintf(stderr, "Going to quit!\n");
      exit_status = EXIT_FAILURE;
    }
  }
  else
  {
    print_help();
  }
  return exit_status;
}
