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
#include <sys/types.h>
#include <sys/wait.h>
#include "main.h"

/**
  @brief does all of the child stuff
  @param program_name the name (and location) of the program
  @param program_opts the options of the program 
*/
void do_child(char *program_name, char **program_opts)
{
  ptrace(PTRACE_TRACEME, NULL, NULL);
  int rv = execvp(program_name, program_opts);
  if(rv != 0)
  {
    exit(exit_status);
  }
  else
  { 
    fprintf(stderr, "Error: Line %i execl()\n", __LINE__);
    exit_status = EXIT_FAILURE;
  }
  exit(exit_status);
}

/**
  @brief comparison operator for qsort
  @param left the left hand operator
  @param right the right hand operator
  @return equality -1 = left < right; 0 left = right; 1 = left > right
*/
int file_vec_struct_comparison(const void *left, const void *right)
{
  int return_value = 0;
  if(((struct file_vec_struct *)left)->priority < 
      ((struct file_vec_struct *)right)->priority)
  {
    return_value = -1;
  }
  else if(((struct file_vec_struct *)left)->priority == 
      ((struct file_vec_struct *)right)->priority)
  {
    return_value = 0;
  }
  else
  {
    return_value = 1;
  }
  return return_value;
}

/**
  @brief reads in a configuration file and opens up files
  @param conf_location the location of our config file
  @param file_array an unitialized array to contain all of the data
*/
void read_conf(char *conf_location, struct file_vec_struct *file_array)
{
  unsigned int count = 1;
  FILE *conf = fopen(conf_location, "r");
  if(conf != NULL)
  {
    size_t length = 0;
    char *buf = NULL;
    ssize_t read = 0;
                             ///should be one byte
    while((read = fread(&buf, sizeof(char), length, conf)) != -1)
    {
      ///Comments start with #
      if(buf[0] != '#')
      {
        file_array = realloc(file_array, sizeof(struct file_vec_struct) *
            (count+1));
        int rank = 0;
        char *pch = strtok(buf, " \t="); ///allow for spaces or tabs and I don't care
                                   //about the equal sign
        while(pch != NULL)
        {
          if(strncmp(pch, "rank_", 5) == 0)
          {
            //so stupid..
            size_t opts_length = strlen(pch) - 5;
            char *program_opts = malloc(opts_length * sizeof(char));
            program_opts = strncpy(program_opts, optarg+5, opts_length);
            rank = atoi(program_opts);
            file_array[count].priority = rank;
          }
          else
          {
            file_array[count].location = malloc(sizeof(char) * strlen(pch));
            strncpy(file_array[count].location, pch, strlen(pch));
            /*
            FILE *tmp_file = fopen(pch, "w+");
            if(tmp_file == NULL)
            {
              fprintf(stderr, "Error: Line %i fopen(\"w+\")\n", __LINE__);
              fprintf(stderr, "Error: Error is: %s\n", strerror(errno));
            }
            else
            {
              file_array[count].file = tmp_file;
            }
            */

          }
          pch = strtok(NULL, " \t="); ///allow for spaces or tabs and I don't care
        }
        count++;  
      }
    }
    fclose(conf);
    qsort(file_array, count, sizeof(struct file_vec_struct), 
        file_vec_struct_comparison);
  }
  else
  {
    fprintf(stderr, "Error: Unable to read config file at line: %i\n", __LINE__);
    fprintf(stderr, "Error: Error is: %s\n", strerror(errno));
    fprintf(stderr, "Going to abort!\n");
    exit_status = EXIT_FAILURE;
    //exit(exit_status);
  }
}


/**
  @brief kills all the children!
  @param status what our exit status is
  @param pid the child's pid
*/
void handle_death(int status, void *pid)
{
  pid_t child = *((pid_t *)pid);
    
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
    fprintf(stderr, "Error on line %i ptrace(PTRACE_GETREGS)\n", __LINE__);
    fprintf(stderr, "Error is: %s\n", strerror(errno));
    fprintf(stderr, "PANIC! Don't know how to recover! Aborting!\n");
    exit_status = EXIT_FAILURE;

    //exit(exit_status);
  }
  return syscall_num;
}

/**
  @brief handles all of the parent functions (stopping on writev and
  manipulating it)
  @param child the child's process id
*/
void do_parent(char *conf_location, pid_t child)
{

  long rv = ptrace(PTRACE_ATTACH, child, NULL, NULL);

  int status = 0;

  if(rv != -1)
  {
    waitpid(-1, &status, __WALL);
    ///make sure that we can inspect children\threads..
    /*
    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL | PTRACE_O_TRACECLONE
        | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEFORK | PTRACE_O_TRACESYSGOOD |
        PTRACE_O_TRACEVFORK);
    */
    //waitpid(-1, &status, __WALL);
    //wait(&status);
    while(1)
    {
      ptrace(PTRACE_SYSCALL, child, NULL, 0);
      wait(&status);
      if(WIFSTOPPED(status))
      {
        long syscall = get_syscall(child);
        if(syscall == SYS_writev)
        {
          ///hijack the write
          /**
            From the man page:
            ssize_t writev(int fd, const struct iovec *iov, int iocnt);

            struct iovec
            {
              void *iov_base;   //Starting Address
              ssize_t iov_len;  //Number of bytes to transfer
            };

            iov is an array of iocnt size.

            So, I'd have to read each one these units, peek data, write it, and
            return what was sent. This applies to read as well. s/write/read/
          */

        }
        else if(syscall == SYS_readv)
        {
          ///hijack the read
        }

      }
    }

    ptrace(PTRACE_DETACH, child, NULL, 0);
  }
  else
  {
    fprintf(stderr, "ERROR on ptrace(PTRACE_ATTACH) line: %i\n", __LINE__);
    fprintf(stderr, "Aborting and killing children\n");
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
  int x = 0;
  for(x = 0; x < argc; x++)
  {
    printf("%i: %s\n", x,argv[x]);
  }
  char *conf_location = NULL, *program_name = NULL, **program_opts = NULL;
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

          //Okay so optarg will only contain the program name. We'll have to
          //copy everything else from argv
          program_name = malloc(program_length * sizeof(char));
          strncpy(program_name, optarg, program_length);
          int x =0;
          for(x = 0; x < argc; x++)
          {
            if(strcmp(argv[x], program_name) == 0)
            {
              x++;
              ///Anything past here belongs to the child.
              program_opts = malloc(sizeof(char *) * (argc - x + 1));
              int y = 0;
              while(x < argc)
              {
                program_opts[y] = argv[x];
                x++;
                y++;
              }
              program_opts[x] = NULL;
            }
          }
          opt_id = -1; //break out of the while loop.
          break;
        }
        else
        {
          //This should never execute.
          fprintf(stderr, "Error on line %i! Found two or more instances of \
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
          fprintf(stderr, "Error on line %i! Found two or more instances of \
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
      //default:
        //print_help();
        //exit(exit_status);
    }
  }
  if(have_c && have_p)
  {
    pid_t child = 0;
    child = fork();
    if(child > 0)
    {
       on_exit(handle_death,&child); ///so this isn't pased to the child
       do_parent(conf_location, child);
    }
    if(child == 0)
    {
      do_child(program_name, program_opts);
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
