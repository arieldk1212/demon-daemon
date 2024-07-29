#include <iostream>
#include <string.h>
#include <signal.h>
#include <sys/inotify.h>
#include <sys/syscall.h> // linux api basically
#include <libnotify/notify.h>

// just some error codes..
#define EXT_SUCCESS 0
#define EXT_ERR_TOO_FEW_ARGS 1
#define EXT_ERR_INIT_INOTIFY 2
#define EXT_ERR_ADD_WATCH 3
#define EXT_ERR_BASE_PATH_NULL 4
#define EXT_ERR_READ_INOTIFY 5
#define EXT_ERR_INIT_LIBNOTIFY 6

int IeventQueue = -1;
int IeventStatus = -1;
int closeStatus = -1;

char *ProgramTitle = "Demon";

void signal_handler(int signal) {
  std::cout << "Signal Received, Cleaning Up..\n";
  closeStatus = inotify_rm_watch(IeventQueue, IeventStatus);
  if (closeStatus == -1) {
    std::cerr << "Error Removing From Watch Queue.\n";
  }
  close(IeventQueue);
  exit(EXT_SUCCESS);
}

// to accept arguments thru terminal wer define these two variables.
int main(int argc, char* argv[]) {

  bool libnotifyInitStatus = false;

  char *basePath = NULL;
  char *token = NULL;
  char *notificationMessage = NULL;

  NotifyNotification *notifyHandle;

  char buffer[4096];
  int readLength;

  // successful read returns a struct..
  const struct inotify_event *watchEvent;

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
  // std::cout << basePath << "\n";

  if (basePath == NULL) {
    std::cerr << "Error Getting Base Path.\n";
    exit(EXT_ERR_BASE_PATH_NULL);
  }

  libnotifyInitStatus = notify_init(ProgramTitle);
  if (!libnotifyInitStatus) {
    std::cerr << "Error Initialising libnotify.\n";
    exit(EXT_ERR_INIT_LIBNOTIFY);
  }

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

  signal(SIGABRT, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  while(true) {
    std::cout << "Waiting For Ievent..\n";
    readLength = read(IeventQueue, buffer, sizeof(buffer));
    if (readLength == -1) {
      std::cerr << "Error Reading From Inotify Instance.";
      exit(EXT_ERR_READ_INOTIFY);
    }

    for (char *bufferPointer = buffer; bufferPointer < buffer + readLength;
        bufferPointer += sizeof(struct inotify_event) + watchEvent->len) {
          notificationMessage = NULL;
          watchEvent = (const struct inotify_event *) bufferPointer;

          if (watchEvent->mask & IN_CREATE) {
            notificationMessage = "File Created.\n";
          }

          if (watchEvent->mask & IN_DELETE) {
            notificationMessage = "File Deleted.\n";
          }

          if (watchEvent->mask & IN_ACCESS) {
            notificationMessage = "File Accessed.\n";
          }

          if (watchEvent->mask & IN_CLOSE_WRITE) {
            notificationMessage = "File Written and Closed.\n";
          }

          if (watchEvent->mask & IN_MODIFY) {
            notificationMessage = "File Modified.\n";
          }

          if (watchEvent->mask & IN_MOVE_SELF) {
            notificationMessage = "File Moved.\n";
          }

          if (notificationMessage == NULL) { 
            continue;
          }

          notifyHandle = notify_notification_new(basePath, notificationMessage, "dialog-information");
          if (notifyHandle == NULL) {
            std::cerr << "Notification Handle was Null.\n";
            continue;
          }

          notify_notification_show(notifyHandle, NULL);
    }
  }
}