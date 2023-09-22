#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include "netutil.h"

using namespace std;


/*
Open the /etc/sysctl.conf file using vi:

sudo vi /etc/sysctl.conf

Add the following lines to the end of the file:

net.core.rmem_max=128000000
net.core.rmem_default=128000000
*/


//==========================================================================================================
// createServer() - Create a UDP server socket
//==========================================================================================================
int createServer(int port, string bind_to )
{
    // Fetch information about the local machine
    addrinfo_t info = NetUtil::get_local_addrinfo(SOCK_DGRAM, port, bind_to, AF_INET);

    // Create the socket
    int sd = socket(info.family, info.socktype, info.protocol);

    // If that failed complain
    if (sd < 0)
    {
        printf("socket() failed\n");
        exit(1);        
    }

    // Bind the socket to the specified port
    if (bind(sd, info, info.addrlen) < 0)
    {
        printf("bind() failed\n");
        exit(1);
    }

    // Hand the socket descriptor to the caller
    return sd;
}
//==========================================================================================================


char buffer[0x10000];
uint64_t& counter = *(uint64_t*)buffer;
uint64_t expected;

int main()
{
    fd_set original;
    uint64_t expected = 0;

    // Create the server
    int sd = createServer(10000, "192.168.50.197");
    
    // Create an fd_set that correspends to our socket descriptor
    FD_ZERO(&original);
    FD_SET(sd, &original);

    // Receive the first UDP datagram
    recvfrom(sd, buffer, sizeof(buffer), 0, NULL, 0);
    
    // The first datagram should contain a 0
    if (counter != expected)
    {
        printf("The first datagram didn't contain a 0!\n");
        exit(1);
    }

    // If no data arrives for 2 seconds, it's the end of the test
    struct timeval timeout = {2, 0};

    while (true)
    {
        // Get the fdset that contains our socket descriptor
        fd_set fds = original;

        // Fetch the timeout that determines how long to wait for a datagram
        auto this_timeout = timeout;
        
        // Wait for a UDP datagram to arrive
        if (select(sd+1, &fds, nullptr, nullptr, &this_timeout) < 1)
        {
            printf("Success\n");
            exit(0);
        }

        // Read the UDP datagram that just arrived
        recvfrom(sd, buffer, sizeof(buffer), 0, NULL, 0);

        // If the UDP datagram doesn't contain the expected value, complain
        if (counter != ++expected)
        {
            printf("Expected %lu, received %lu !\n", expected, counter);
            exit(1);
        }

    }
}




