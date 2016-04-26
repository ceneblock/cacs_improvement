/**
  @author ceneblock
  @brief a simple program to test the cacs_improvements
*/
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_help()
{
  printf("A simple program to test cacs_improvements\n");
  printf("\n");
  printf("It's going to open three files whose name are file_1, file_2, and\n");
  printf(" file_3\n");
  printf("\n");
  printf("The files will be opened and the ranks will be 1, 2, and 3 \n");
  printf(" respectively\n");
  printf("\n");
  printf("cacs_test file_1 file_2 file_3\n");

}

int main(int argc, char **argv)
{
  if(argc == 4)
  {
    printf("Going to open a file called %s with rank 1\n", argv[1]);
    printf("Going to open a file called %s with rank 1\n", argv[2]);
    printf("Going to open a file called %s with rank 1\n", argv[3]);

    int file1 = open(argv[1], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    int file2 = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    int file3 = open(argv[3], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    char *test_string = "Hello World";

    struct iovec *iov = malloc(sizeof(struct iovec));
    iov -> iov_base = test_string;
    iov -> iov_len  = strlen(test_string);

    writev(file1, iov, 1);
    writev(file2, iov, 1);
    writev(file3, iov, 1);

    close(file1);
    close(file2);
    close(file3);
  }
  else
  {
    print_help();
  }
  exit(EXIT_SUCCESS);
}
