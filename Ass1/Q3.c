#include <stdio.h> #include <unistd.h> #include <fcntl.h>

int main() {
  close(1); //Close stdout
  open("redirect.txt", O_WRONLY); //Open redirect.txt in write only mode 
  printf(“A simple program output.”);
  return 0; 
}

