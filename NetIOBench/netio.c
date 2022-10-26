/* This program is a simple benchmark utility that tests the efficiency of three different modes of client/server process communication.
 * Namely local function calls,  pipes, TCP/IP sockets, and remote procedure calls. (RPC)
 * Efficiency is measured in terms of floating point operations per second for addition, subtraction, multiplication, and division.
 *
 * Author: Grayson Kern  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <rpc/rpc.h>
#define PORT 8080

#define MSG "* running netio with method %s operation %s for %s number of ops...\n"

#define USAGE "usage: ./netio <method> <operation> <num_ops> \n" \
"     - method: function / pipe / socket / rpc \n" \
"     - operation: add / subtract / multiply / divide \n" \
"     - num_calls: 1000 | 1000000 \n"

double multiply(double a, double b)
{
	return a * b;
}

double divide(double a, double b)
{
	if(b == 0.0)
	{
		return 0;
	}
	else
	{
		return a / b;
	}
}

double add(double a, double b)
{

	return a + b;
}

double subtract(double a, double b)
{
	return a - b;
}

struct SocketAddress
{
	unsigned short family, port;
	unsigned long address;
}socketAddress;

int main(int argc, char **argv)
{
	time_t t;
	srand((unsigned) time(&t));
	int method = -1; //used to store what method to test
	int operation = -1; //used to store what operation to test
	int num_ops = atoi(argv[3]);
	int fds[2];
	int client = socket(PF_INET, SOCK_STREAM, 0);
	int newClient;
	int server = socket(PF_INET, SOCK_STREAM, 0);
	socketAddress.family = AF_INET;
	socketAddress.port = htons(8080);
	socketAddress.address = htonl(INADDR_ANY);
	double ret_value = 0.0;
	double sent, received, newSent, newReceived;
	struct timeval start, end;

    if (argc != 4) 
    {
        printf(USAGE);
        exit(1);
    } 
    else 
    {
        printf(MSG, argv[1], argv[2], argv[3]);

        if(strcmp(argv[1],"function") == 0)
        	method = 0;

        else if(strcmp(argv[1],"pipe") == 0)
        	method = 1;

        else if(strcmp(argv[1],"socket") == 0)
        	method = 2;

        else if(strcmp(argv[1],"rpc") == 0)
        	method = 3;

        else
        	method = -1;

        if(strcmp(argv[2],"add") == 0)
        	operation = 0;

        else if(strcmp(argv[2],"subtract") == 0)
        	operation = 1;

        else if(strcmp(argv[2],"multiply") == 0)
        	operation = 2;

        else if(strcmp(argv[2],"divide") == 0)
        	operation = 3;

        else
        	operation = -1;

  	gettimeofday(&start, NULL);

   	switch (method)
     	{
        	case 0: // function
   			switch (operation)
     			{
        			case 0: // add
           				for (int i=0;i<num_ops;i++)
           					ret_value = add((double)rand()/RAND_MAX,(double)rand()/RAND_MAX);
           				break;

        			case 1: // subtract
           				for (int i=0;i<num_ops;i++)
           					ret_value = subtract((double)rand()/RAND_MAX,(double)rand()/RAND_MAX);
           				break;

        			case 2: // multiply
           				for (int i=0;i<num_ops;i++)
           					ret_value = multiply((double)rand()/RAND_MAX,(double)rand()/RAND_MAX);
           				break;

        			case 3: // divide
           				for (int i=0;i<num_ops;i++)
           					ret_value = divide((double)rand()/RAND_MAX,(double)rand()/RAND_MAX);
           				break;

        			default:
           				printf("operation not supported, exit...\n");
           				return -1;
     			}

     			break;

        	case 1: // pipe
			pipe(fds);
			
			if(fork() == 0) // Child writes 
			{

   				switch (operation)
     				{
        				case 0: // add
						for(int i = 1; i <= num_ops; i++)
						{
							ret_value = add((double)rand()/RAND_MAX, (double)rand()/RAND_MAX);
							write(fds[1], &ret_value, sizeof(ret_value));
						}

						exit(0);
           					break;

        				case 1: // subtract
           					for(int i = 1; i <= num_ops; i++)
						{
							ret_value = subtract((double)rand()/RAND_MAX, (double)rand()/RAND_MAX);
							write(fds[1], &ret_value, sizeof(ret_value));
						}

						exit(0);
           					break;

					case 2: // multiply
						for(int i = 1; i <= num_ops; i++)
						{
							ret_value = multiply((double)rand()/RAND_MAX, (double)rand()/RAND_MAX);
							write(fds[1], &ret_value, sizeof(ret_value));
						}
						
						exit(0);
						break;

					case 3: // divide
						for(int i = 1; i <= num_ops; i++)
						{
							ret_value = divide((double)rand()/RAND_MAX, (double)rand()/RAND_MAX);
							write(fds[1], &ret_value, sizeof(ret_value));
						}

						exit(0);
						break;

					default:
           					printf("operation not supported, exit...\n");
           					return -1;
     				}
			}
			else // Parent reads
			{
				for(int i = 1; i <= num_ops; i++)
				{
					read(fds[0], &ret_value, sizeof(ret_value));
				}
			}

     			break;

        	case 2: // socket

   			switch (operation)
     			{
        			case 0: // add
           				
					if(bind(server, (struct SocketAddress *) &socketAddress, sizeof(socketAddress)) != -1) // Bind address structure to server
					{
						listen(server, 1); // Listen for a connection

						if(connect(server, (struct SocketAddress *) &socketAddress, sizeof(socketAddress)) != -1) // If a connection exists, connect.
						{
							for(int i = 1; i <= num_ops; i++) // Perform calculations.
							{
								ret_value = add((double)rand()/RAND_MAX, (double)rand()/RAND_MAX);
								sent = send(server, &ret_value, sizeof(ret_value), 0);
								received = (server, &ret_value, sizeof(ret_value), 0);
								newClient = accept(client, (struct SocketAddress *) &socketAddress, sizeof(socketAddress));
								newReceived = recv(client, &ret_value, sizeof(ret_value), 0);
								newSent = send(client, &ret_value, sizeof(ret_value), 0);
							}
						}
					}
					
					close(client);
					close(server);

           				break;

        			case 1: // subtract
           					           				
					if(bind(server, (struct SocketAddress *) &socketAddress, sizeof(socketAddress)) != -1) // Bind address structure to server
					{
						listen(server, 1); // Listen for a connection

						if(connect(server, (struct SocketAddress *) &socketAddress, sizeof(socketAddress)) != -1) // If a connection exists, connect.
						{
							for(int i = 1; i <= num_ops; i++) // Perform calculations.
							{
								ret_value = subtract((double)rand()/RAND_MAX, (double)rand()/RAND_MAX);
								sent = send(server, &ret_value, sizeof(ret_value), 0);
								received = (server, &ret_value, sizeof(ret_value), 0);
								newClient = accept(client, (struct SocketAddress *) &socketAddress, sizeof(socketAddress));
								newReceived = recv(client, &ret_value, sizeof(ret_value), 0);
								newSent = send(client, &ret_value, sizeof(ret_value), 0);
							}
						}
					}
					
					close(client);
					close(server);
           				
           				break;

				case 2: // multiply
			    		
			    		if(bind(server, (struct SocketAddress *) &socketAddress, sizeof(socketAddress)) != -1) // Bind address structure to server
					{
						listen(server, 1); // Listen for a connection

						if(connect(server, (struct SocketAddress *) &socketAddress, sizeof(socketAddress)) != -1) // If a connection exists, connect.
						{
							for(int i = 1; i <= num_ops; i++) // Perform calculations.
							{
								ret_value = multiply((double)rand()/RAND_MAX, (double)rand()/RAND_MAX);
								sent = send(server, &ret_value, sizeof(ret_value), 0);
								received = (server, &ret_value, sizeof(ret_value), 0);
								newClient = accept(client, (struct SocketAddress *) &socketAddress, sizeof(socketAddress));
								newReceived = recv(client, &ret_value, sizeof(ret_value), 0);
								newSent = send(client, &ret_value, sizeof(ret_value), 0);
							}
						}
					}
					
					close(client);
					close(server);
			    		
					break;

				case 3: // divide
					
					if(bind(server, (struct SocketAddress *) &socketAddress, sizeof(socketAddress)) != -1) // Bind address structure to server
					{
						listen(server, 1); // Listen for a connection

						if(connect(server, (struct SocketAddress *) &socketAddress, sizeof(socketAddress)) != -1) // If a connection exists, connect.
						{
							for(int i = 1; i <= num_ops; i++) // Perform calculations.
							{
								ret_value = divide((double)rand()/RAND_MAX, (double)rand()/RAND_MAX);
								sent = send(server, &ret_value, sizeof(ret_value), 0);
								received = (server, &ret_value, sizeof(ret_value), 0);
								newClient = accept(client, (struct SocketAddress *) &socketAddress, sizeof(socketAddress));
								newReceived = recv(client, &ret_value, sizeof(ret_value), 0);
								newSent = send(client, &ret_value, sizeof(ret_value), 0);
							}
						}
					}
					
					close(client);
					close(server);
					
					break;

				default:
           				printf("operation not supported, exit...\n");
           				return -1;
     			}

     			break;
			
       		 case 3: // rpc
   			switch (operation)
     			{
        			case 0: // add
           				printf("%s %s %s\n",argv[1],argv[2],argv[3]);
           				break;

        			case 1: // subtract
           				printf("%s %s %s\n",argv[1],argv[2],argv[3]);
           				break;

				case 2: // multiply
					printf("%s %s %s\n",argv[1],argv[2],argv[3]);
					break;

				case 3: // divide
					printf("%s %s %s\n",argv[1],argv[2],argv[3]);
					break;

				default:
           				printf("operation not supported, exit...\n");
           				return -1;
     			}

     			break;

       		default:
        		printf("method not supported, exit...\n");
           		return -1;
     	}		

     	gettimeofday(&end, NULL);

	double elapsed_time_us = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
	printf("==> %f ops/sec\n",(num_ops/elapsed_time_us)*1000000);
 
    }

    return 0;
}
