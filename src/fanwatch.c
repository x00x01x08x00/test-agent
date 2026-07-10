#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <unistd.h>

static volatile sig_atomic_t keep_running = 1;

static void handle_signal(int signo)
{
    (void)signo;
    keep_running = 0;
}

static const char *event_name(uint64_t mask)
{
    if (mask & FAN_OPEN)
        return "OPEN";
    if (mask & FAN_MODIFY)
        return "MODIFY";
    if (mask & FAN_CLOSE_WRITE)
        return "CLOSE_WRITE";

    return "OTHER";
}

static int install_signal_handlers(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) == -1)
        return -1;

    if (sigaction(SIGTERM, &action, NULL) == -1)
        return -1;

    return 0;
}

int main(int argc, char **argv)
{
    struct stat st;
    int fan_fd;
    uint64_t mask;
    char buffer[8192] __attribute__((aligned(8)));

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (stat(argv[1], &st) == -1) {
        perror("stat");
        return EXIT_FAILURE;
    }

    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "%s is not a directory\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (install_signal_handlers() == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    fan_fd = fanotify_init(FAN_CLASS_NOTIF | FAN_CLOEXEC,
                           O_RDONLY | O_LARGEFILE);
    if (fan_fd == -1) {
        perror("fanotify_init");
        fprintf(stderr,
                "Tip: run as root or with the required fanotify capability.\n");
        return EXIT_FAILURE;
    }

    mask = FAN_OPEN | FAN_MODIFY | FAN_CLOSE_WRITE;

    if (fanotify_mark(fan_fd,
                      FAN_MARK_ADD | FAN_MARK_ONLYDIR,
                      mask,
                      AT_FDCWD,
                      argv[1]) == -1) {
        perror("fanotify_mark");
        close(fan_fd);
        return EXIT_FAILURE;
    }

    printf("Watching directory: %s\n", argv[1]);
    printf("Press Ctrl+C to stop.\n");
    fflush(stdout);

    while (keep_running) {
        ssize_t length = read(fan_fd, buffer, sizeof(buffer));

        if (length == -1) {
            if (errno == EINTR) {
                if (!keep_running)
                    break;
                continue;
            }

            perror("read");
            break;
        }

        struct fanotify_event_metadata *metadata =
            (struct fanotify_event_metadata *)buffer;

        while (FAN_EVENT_OK(metadata, length)) {
            if (metadata->vers != FANOTIFY_METADATA_VERSION) {
                fprintf(stderr, "fanotify metadata version mismatch\n");
                close(fan_fd);
                return EXIT_FAILURE;
            }

            if (metadata->mask & FAN_Q_OVERFLOW) {
                fprintf(stderr, "Warning: fanotify queue overflow\n");
                metadata = FAN_EVENT_NEXT(metadata, length);
                continue;
            }

            if (metadata->fd >= 0) {
                char proc_path[64];
                char path[PATH_MAX];
                ssize_t path_length;

                snprintf(proc_path,
                         sizeof(proc_path),
                         "/proc/self/fd/%d",
                         metadata->fd);

                path_length =
                    readlink(proc_path, path, sizeof(path) - 1);

                if (path_length == -1) {
                    snprintf(path,
                             sizeof(path),
                             "<unresolved: %s>",
                             strerror(errno));
                }

                printf("%-12s pid=%d path=%s\n",
                       event_name(metadata->mask),
                       metadata->pid,
                       path);
                fflush(stdout);
            }

            metadata = FAN_EVENT_NEXT(metadata, length);
        }
    }

    close(fan_fd);
    puts("Stopped.");
    return EXIT_SUCCESS;
}
