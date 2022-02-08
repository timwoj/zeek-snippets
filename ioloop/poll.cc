#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char **argv)
	{
	char buff[500];
    int buff_sz;                             //size of data recieved

    printf("App Started\n");

    //allocate space for 10
	struct pollfd pd;
	memset(&pd, 0, sizeof(struct pollfd));

	int fd = open(argv[1], O_RDONLY | O_NONBLOCK);

    //I call listen_socket() which creates a socket to listen to
    //this is anchored into my_fds array at element 0.
	pd.fd = fd;
    pd.events = POLLIN;
    pd.revents = 0;

    //This is the main loop.
    //While (true)
    //  set all struct pollfd items revents to 0
    //  call poll
    //  loop through, see if there is data to read
    //      read the data
    //          loop through all sockets (except the listen_socket()) and send the data.
    while (1)
		{
		pd.events = POLLIN | POLLPRI;
		pd.revents = 0;

        //put all this into poll and wait for something magical to happen
        printf("calling poll\n");
        if (poll(&pd, 1, 0) == -1)
			{
            perror("poll");
            exit(0);
			}

        printf("poll returned!\n");

		buff_sz = read(fd, buff, 500);
		if ( buff_sz == 1 )
			{
			perror("read error");
			break;
			}
		else if ( buff_sz == 0 )
			{
			printf("eof\n");
			break;
			}
		else
			{
			printf("read %d bytes\n", buff_sz);
			}
        }
	
	printf("App Ended\n");
	}
