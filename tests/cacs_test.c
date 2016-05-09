/**
  @author ceneblock
  @brief a simple program to test the cacs_improvements
*/
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm-generic/fcntl.h>
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

//int main(int _argc, char **_argv)
int main(int argc, char **argv)
{
  if(argc == 4)
  {
    unsigned int class1 = 1, class2 = 2, class3 = 3;
    printf("Going to open a file called %s with rank 1\n", argv[1]);
    printf("Going to open a file called %s with rank 2\n", argv[2]);
    printf("Going to open a file called %s with rank 3\n", argv[3]);

    int file1 = open(argv[1], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | O_CLASSIFY);
    int file2 = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | O_CLASSIFY);
    int file3 = open(argv[3], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | O_CLASSIFY);

    char *test_string1 = "Hello World 1\n";
    char *test_string2 = "Hello World 2\n";
    char *test_string3 = "Hello World 3\n";

    struct iovec *iov = malloc(sizeof(struct iovec) * 3 * 2);
    iov[0].iov_base = &class1;
    iov[0].iov_len  = sizeof(unsigned int);
    iov[1].iov_base = test_string1;
    iov[1].iov_len  = strlen(test_string1);
    
    iov[2].iov_base = &class2;
    iov[2].iov_len  = sizeof(unsigned int);
    iov[3].iov_base = test_string2;
    iov[3].iov_len  = strlen(test_string2);
    
    iov[4].iov_base = &class3;
    iov[4].iov_len  = sizeof(unsigned int);
    iov[5].iov_base = test_string3;
    iov[5].iov_len  = strlen(test_string3);

    printf("test1: IOV: &p, IOV: p, IOV_BASE: &%p, IOV_BASE: %p IOV_LEN: &%p IOV_LEN: %#zx\n", 
        (void *) &iov[0].iov_base,(void *) iov[0].iov_base, (void *) &iov[0].iov_len, iov[0].iov_len);
    printf("test2: IOV: &p, IOV: p, IOV_BASE: &%p, IOV_BASE: %p IOV_LEN: &%p IOV_LEN: %#zx\n", 
        (void *) &iov[1].iov_base,(void *) iov[1].iov_base, (void *) &iov[1].iov_len, iov[1].iov_len);
    printf("test3: IOV: &p, IOV: p, IOV_BASE: &%p, IOV_BASE: %p IOV_LEN: &%p IOV_LEN: %#zx\n", 
        (void *) &iov[2].iov_base,(void *) iov[2].iov_base, (void *) &iov[2].iov_len, iov[2].iov_len);
    
    writev(file1, &iov[0], 2);
    writev(file2, &iov[2], 2);
    writev(file3, &iov[4], 2);

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
