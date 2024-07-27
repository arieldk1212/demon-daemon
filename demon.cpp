#include <iostream>
#include <sys/syscall.h> // linux api basically
#include <sys/inotify.h>
#include <string.h>
#include <signal.h>

// just some error codes..
#define EXT_SUCCESS 0
#define EXT_ERR_TOO_FEW_ARGS 1
#define EXT_ERR_INIT_INOTIFY 2
#define EXT_ERR_ADD_WATCH 3

int IeventQueue = -1;
int IeventStatus = -1;

// to accept arguments thru terminal wer define these two variables.
int main(int argc, char* argv[]) {

  char *basePath = NULL;
  char *token = NULL;

  const uint32_t watchMask = IN_CREATE | IN_DELETE | IN_ACCESS | IN_CLOSE_WRITE | IN_MODIFY | IN_MOVE_SELF;
  
  if (argc < 2) {
    std::cerr << "USAGE: demon daemon PATH\n";
    exit(EXT_ERR_TOO_FEW_ARGS);
  }

  // Accepting the full file path, getting the file name from it.
  basePath = (char *)malloc(sizeof(char)*(strlen(argv[1]) + 1));
  strcpy(basePath, argv[1]);

  token = strtok(basePath, "/");
  while(token != NULL) {
    basePath = token;
    token = strtok(NULL, "/");
  }
  // Test
  std::cout << basePath << "\n";

  IeventQueue = inotify_init();
  if (IeventQueue == -1) {
    std::cerr << "Error Initialising inotify Instance.\n";
    exit(EXT_ERR_INIT_INOTIFY);
  }

  IeventStatus = inotify_add_watch(IeventQueue, argv[1], watchMask);
  if (IeventStatus == -1) {
    std::cerr << "Error Adding File To Watch Instance.\n";
    exit(EXT_ERR_ADD_WATCH);
  }

  while(true) {

  }

}