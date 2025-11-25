#include <sys/types.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <thread>

void pipe_thread(int fd) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        const char* data = "test";
        printf("thread: Writing data to pipe\n");
        write(fd, data, 4);
    }
}

int main(int argc, char **argv) {
    // make a fifo queue and open it
    printf("Making fifo file\n");
    if ( mkfifo("zeek-kq.fifo", 0666) ) {
        perror("Failed to create fifo file");
        exit(-1);
    }

    printf("Opening fifo file\n");
    int fifo_fd = open("zeek-kq.fifo", O_RDONLY | O_NONBLOCK);
    if ( fifo_fd < 0 ) {
        perror("Failed to open fifo file");
        exit(-1);
    }
    printf("Fifo FD: %d\n", fifo_fd);

    // make a pipe
    printf("Opening pipe\n");
    int pipefds[2];
    if ( pipe(pipefds) ) {
        perror("Failed to open pipe");
        exit(-1);
    }

    // add both to kqueue
    printf("Making kqueue\n");
    int kq = kqueue();

    struct kevent ke[2];
    struct timespec timeout = {10,0};

    printf("Setting up kevents\n");
    EV_SET(&ke[0], fifo_fd, EVFILT_READ, EV_ADD, 0, 0, 0);
    EV_SET(&ke[1], pipefds[0], EVFILT_READ, EV_ADD, 0, 0, 0);

    printf("Adding kevents to kqueue\n");
    kevent(kq, ke, 2, NULL, 0, NULL);

    // start a thread that sleeps then writes the pipe
    printf("Starting thread\n");
    std::thread pt{pipe_thread, pipefds[1]};

    int count = 0;
    while (true) {
        off_t left;
        memset(&ke[0], 0, sizeof(struct kevent));
        memset(&ke[1], 0, sizeof(struct kevent));
        printf("main: Waiting for kevent\n");
        int ret = kevent(kq, NULL, 0, ke, 2, &timeout);
        if ( ret == -1 ) {
            perror("kevent failed");
            break;
        } else if ( ret == 0 ) {
            printf("timeout\n");
            count++;

            if ( count == 50 )
                break;
        } else {
            for ( int i = 0; i < ret; i++ ) {
                if( ke[i].flags & EV_ERROR ) {
                    printf("flags had error\n");
                    break;
                }

                if (ke[i].flags & EV_EOF) {
                    printf("premature end of file!\n");
                    break;
                }

                if ( ke[i].ident == pipefds[0] ) {
                    printf("Got data from pipe\n");
                    char buf[10];
                    read(pipefds[0], buf, 10);
                }
                else if ( ke[i].ident == fifo_fd ) {
                    printf("Got data from fifo\n");
                    char buf[10];
                    read(fifo_fd, buf, 10);
                }
            }
        }
    }

    return 0;

    // wait for the kqeuue
    // see if the pipe wakes up the queue
}
