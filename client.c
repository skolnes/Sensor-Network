/**
 * client.c
 *
 * @author Your name here
 *
 * USD COMP 375: Computer Networks
 * Project 1
 *
 * This projects uses our knowledge of packet sniffing and networks to reverse
 * engineer a client based executable to connect with sensor.sandiego.edu and
 * request either air temp, humidity, windspeed, and/or exit from the server.
 */

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFF_SIZE 1024

long prompt();
int connectToHost(char *hostname, char *port);
void mainLoop();
//Made seperate calling function for each sensor to connect to
void airTemp(char *serverPass, char *sensorPass);
void relHumidity(char *serverPass, char *sensorPass);
void windSpeed(char *serverPass, char *sensorPass);

/*
 * Sends message over given socket or exits if something went wrong.
 */
void send_or_exit(int fd, char *buff, size_t buff_len) {
	int sent = send(fd, buff, buff_len, 0);
	if (sent == 0) {
		printf("Server SEND connection closed unexpectedly. Good bye.\n");
		exit(1);
	}
	else if (sent == -1) {
		perror("send");
		exit(1);
	}
}

void recv_or_exit(int fd, char *buff, size_t max_len) {
	int recvd = recv(fd, buff, max_len, 0);
	if (recvd == 0) {
		 printf("Server RECV connection closed unexpectedly. Good bye.\n");
		  exit(1);
	}
	else if (recvd  == -1) {
	perror("recv");
	exit(1);
	}
}
					    
int main() {
	mainLoop();
	return 0;
}

/**
 * Loop to keep asking user what they want to do and calling the appropriate
 * function to handle the selection.
 *
 */
void mainLoop() {
	printf("WELCOME TO THE COMP375 SENSOR NETWORK. \n\n\n");

	//comp375.sandiego.edu password
	char serverPass[17];
	memset(serverPass, 0, 17);
	strcpy(serverPass, "AUTH password123\n");

	//sensor.sandiego.edu password
	char sensorPass[19];
	memset(sensorPass, 0, 19);
	strcpy(sensorPass, "AUTH sensorpass321\n");

	while (1) {
		//Displays the prompt
		long selection = prompt();
		//Waits for user to input desired sensor or to exit
		switch (selection) {
			case 1:
				airTemp(serverPass, sensorPass);
				break;
			case 2:
				relHumidity(serverPass, sensorPass);
				break;
			case 3:
				windSpeed(serverPass, sensorPass);
				break;
			case 4: 
				printf("GOODBYE!!\n");
				//close(server_fd);
				//close(sensor_fd);
				exit(0);
				break;
			default:
				fprintf(stderr, "ERROR: Invalid selection\n");
				break;
		}
	}
	//Error if gets to this point
	//close(server_fd);
	printf("Main loop Error!\n\nBoi Bye\n\n");
	exit(1);
}

/** 
 * Print command prompt to user and obtain user input.
 *
 * @return The user's desired selection, or -1 if invalid selection.
 */
long prompt() {
	printf("Which sensor would you like to read \n");
	printf("\t1.) Air Temperature\n");
	printf("\t2.) Relative Humidity\n");
	printf("\t3.) Wind Speed\n");
	printf("\t4.) Quit Program\n\n");
	printf("Selection #: ");

	// Read in a value from standard input
	char input[10];
	memset(input, 0, 10); // set all characters in input to '\0' (i.e. nul)
	char *read_str = fgets(input, 10, stdin);

	// Check if EOF or an error, exiting the program in both cases.
	if (read_str == NULL) {
		if (feof(stdin)) {
			exit(0);
		}
		else if (ferror(stdin)) {
			perror("fgets");
			exit(1);
		}
	}

	// get rid of newline, if there is one
	char *new_line = strchr(input, '\n');
	if (new_line != NULL) new_line[0] = '\0';

	// convert string to a long int
	char *end;
	long selection = strtol(input, &end, 10);

	if (end == input || *end != '\0') {
		selection = -1;
	}

	return selection;
}

/**
 * Socket implementation of connecting to a host at a specific port.
 *
 * @param hostname The name of the host to connect to (e.g. "foo.sandiego.edu")
 * @param port The port number to connect to
 * @return File descriptor of new socket to use.
 */
int connectToHost(char *hostname, char *port) {
	// Step 1: fill in the address info in preparation for setting 
	//   up the socket
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;       // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// get ready to connect
	if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Step 2: Make a call to socket
	int fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (fd == -1) {
		perror("socket");
		exit(1);
	}

	// Step 3: connect!
	if (connect(fd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
		perror("connect");
		exit(1);
	}

	freeaddrinfo(servinfo); // free's the memory allocated by getaddrinfo

	return fd;
}

/**
 * Function that takes in the server fd and sensor fd and requests the air temperature from
 * the sensor.sandiego.edu server
 *
 * @param serverPass     The password string used to authenticate connection to comp375.sandiego.edu
 * @param sensorPass     The password string used to authenticate connection to sensor.sandiego.edu
 *
 */
void airTemp(char *serverPass, char *sensorPass)	{
	char buff[BUFF_SIZE];
	memset(buff, 0, BUFF_SIZE);

	//send request to comp375.sandiego.edu server
	int server_fd = connectToHost("comp375.sandiego.edu", "47789");
	send_or_exit(server_fd, serverPass, 17);

    //Gets response from server
	recv_or_exit(server_fd, buff, BUFF_SIZE);

    //Parse response to get just port number
    char *portToken;
    portToken = strtok(buff, " ");
    portToken = strtok(NULL, " ");
    portToken = strtok(NULL, " ");
                                            
	//send request to the sensor.sandiego.edu server
    int sensor_fd = connectToHost("sensor.sandiego.edu", portToken);
	printf("Succesfully connected to sensor.sandiego.edu server. Welcome. Port #: %s\n\n", portToken);
	
	//Sends desired sensor, in this case air temp
	char airPass[16];
	memset(airPass, 0, 16);
	strcpy(airPass, "AIR TEMPERATURE\n");

	//Saves CLOSE message for later
	char closeConnect[6];
	memset(closeConnect, 0, 6);	
	strcpy(closeConnect, "CLOSE\n");

	//Send to port
	send_or_exit(sensor_fd, sensorPass, 19);
	
	memset(buff, 0, BUFF_SIZE);
	recv_or_exit(sensor_fd, buff, BUFF_SIZE);
	//checks for server response to be SUCCESS, if it is not it exits
	//accordingly with an error
	if (strcmp(buff, "SUCCESS\n") == 0)
	{
		send_or_exit(sensor_fd, airPass, 16);

		memset(buff, 0, BUFF_SIZE);
		recv_or_exit(sensor_fd, buff, BUFF_SIZE);
		printf("\n");

		char *tempToken;
		char *timeToken;

		timeToken = strtok(buff, " ");
		tempToken = strtok(NULL, " ");

		time_t readTime = (time_t) strtol(timeToken, NULL, 10);


		printf("The last AIR TEMPERATURE reading was %s F, taken at %s", tempToken, ctime(&readTime));
		printf("\n");

		send_or_exit(sensor_fd, closeConnect, 6);

		memset(buff, 0, BUFF_SIZE);
		recv_or_exit(sensor_fd, buff, BUFF_SIZE);
		//closes connection with server if the server responds to close
		//message
		if (strcmp(buff, "BYE\n") == 0)
		{
			printf("Server Connection Succesfully Closed!!\n\n");
			close(sensor_fd);
			close(server_fd);
		}
		else
		{
			printf("Port Closing error!\n\n BYE!\n");
			exit(0);
		}
	}
	else
	{
		printf("ERROR: Recieve Failure\n");
		exit(1);
	}                                                                                                
}

/**
* Function that takes in the server password and the sensor password, and then requests 
* the relative Humidity from the sensor.sandiego.edu server
*
* @param serverPass		The password string used to authenticate connection to comp375.sandiego.edu
* @param sensorPass		The password string used to authenticate connection to
* 						sensor.sandiego.edu
*
*/
void relHumidity(char *serverPass, char *sensorPass)	{
	char buff[BUFF_SIZE];
    memset(buff, 0, BUFF_SIZE);
    
    //send request to comp375.sandiego.edu server
    int server_fd = connectToHost("comp375.sandiego.edu", "47789");
    send_or_exit(server_fd, serverPass, 17);
    
    //Gets response from server
    recv_or_exit(server_fd, buff, BUFF_SIZE);
    
    //Parse here
    char *portToken;
    portToken = strtok(buff, " ");
    portToken = strtok(NULL, " ");
    portToken = strtok(NULL, " ");
    
	//connects to the sensor.sandiego.edu server
    int sensor_fd = connectToHost("sensor.sandiego.edu", portToken);
    printf("Succesfully connected to sensor.sandiego.edu server. Welcome. Port #: %s\n\n", portToken);
    
	//Saves humidity password to send to server
    char humidityPass[18];
    memset(humidityPass, 0, 18);
    strcpy(humidityPass, "RELATIVE HUMIDITY\n");
    
	//Saves the close message for later
    char closeConnect[6];
    memset(closeConnect, 0, 6);	
    strcpy(closeConnect, "CLOSE\n");
    
    //Send to port
    send_or_exit(sensor_fd, sensorPass, 19);
    	
    memset(buff, 0, BUFF_SIZE);
    recv_or_exit(sensor_fd, buff, BUFF_SIZE);
	//Checks to see if server connected successfully and if not exits with
	//error
    if (strcmp(buff, "SUCCESS\n") == 0)
    {
    	send_or_exit(sensor_fd, humidityPass, 19);
    
    	memset(buff, 0, BUFF_SIZE);
    	recv_or_exit(sensor_fd, buff, BUFF_SIZE);
    	printf("\n");
    
    	char *tempToken;
    	char *timeToken;
    
    	timeToken = strtok(buff, " ");
    	tempToken = strtok(NULL, " ");
    
    	time_t readTime = (time_t) strtol(timeToken, NULL, 10);
    
    
    	printf("The last RELATIVE HUMIDITY reading was %s percent, taken at %s", tempToken, ctime(&readTime));
    	printf("\n");
    
    	send_or_exit(sensor_fd, closeConnect, 6);
    
    	memset(buff, 0, BUFF_SIZE);
    	recv_or_exit(sensor_fd, buff, BUFF_SIZE);
    	if (strcmp(buff, "BYE\n") == 0)
    	{
    		printf("Server Connection Succesfully Closed!!\n\n");
    		close(sensor_fd);
			close(server_fd);
    	}
    	else
    	{
    		printf("Port Closing error!\n\n BYE!\n");
    		exit(0);
    	}
    }
    else
    {
    	printf("ERROR: Recieve Failure\n");
    	exit(1);
    }
}

/**
* Function that takes the server password and the sensor password to connect
* to the sensor.sandiego.edu server and read the wind speed
*
* @param serverPass     The password string used to authenticate connection to comp375.sandiego.edu
* @param sensorPass     The password string used to authenticate connection to sensor.sandiego.edu
*
*/
void windSpeed(char *serverPass, char *sensorPass)	{
	char buff[BUFF_SIZE];
 	memset(buff, 0, BUFF_SIZE);
 
 	//send request to comp375.sandiego.edu server
 	int server_fd = connectToHost("comp375.sandiego.edu", "47789");
 	send_or_exit(server_fd, serverPass, 17);
 
     //Gets response from server
 	recv_or_exit(server_fd, buff, BUFF_SIZE);
 
    //Parse here
    char *portToken;
    portToken = strtok(buff, " ");
    portToken = strtok(NULL, " ");
    portToken = strtok(NULL, " ");
    
	//Connects to the sensor network 
    int sensor_fd = connectToHost("sensor.sandiego.edu", portToken);
 	printf("Succesfully connected to sensor.sandiego.edu server. Welcome. Port #: %s\n\n", portToken);
 	
	//Saves the wind speed password for later
 	char windPass[11];
 	memset(windPass, 0, 11);
 	strcpy(windPass, "WIND SPEED\n");
 
 	//saves the close passphrase for later use
 	char closeConnect[6];
 	memset(closeConnect, 0, 6);	
 	strcpy(closeConnect, "CLOSE\n");
 
 	//Send to port
 	send_or_exit(sensor_fd, sensorPass, 19);
 	
 	memset(buff, 0, BUFF_SIZE);
 	recv_or_exit(sensor_fd, buff, BUFF_SIZE);
 	if (strcmp(buff, "SUCCESS\n") == 0)
 	{
 		send_or_exit(sensor_fd, windPass, 11);
 
 		memset(buff, 0, BUFF_SIZE);
 		recv_or_exit(sensor_fd, buff, BUFF_SIZE);
 		printf("\n");
 
 		char *tempToken;
 		char *timeToken;
 
 		timeToken = strtok(buff, " ");
 		tempToken = strtok(NULL, " ");
 
 		time_t readTime = (time_t) strtol(timeToken, NULL, 10);
 
 
 		printf("The last WIND SPEED reading was %s mph, taken at %s", tempToken, ctime(&readTime));
 		printf("\n");
 
 		send_or_exit(sensor_fd, closeConnect, 6);
 
 		memset(buff, 0, BUFF_SIZE);
 		recv_or_exit(sensor_fd, buff, BUFF_SIZE);
 		if (strcmp(buff, "BYE\n") == 0)
 		{
 			printf("Server Connection Succesfully Closed!!\n\n");
 			close(sensor_fd);
			close(server_fd);
 		}
 		else
 		{
 			printf("Port Closing error!\n\n BYE!\n");
 			exit(0);
 		}
 	}
 	else
 	{
 		printf("ERROR: Recieve Failure\n");
 		exit(1);
 	}
}
