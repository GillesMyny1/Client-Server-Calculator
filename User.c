#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>
#include <string.h>

#define SERVER_MQUEUE 1234
#define CLIENT_MQUEUE 4321
#define MAX_TEXT 35

//Message structure to receive from Calculator
struct msg_rcv {
	long int msg_type;
	double standard;
	double optional;
};

//Message structure to send to Calculator
struct msg_pass {
	long int msg_type;
	char some_text[MAX_TEXT];
};

static int serv_qid = -1;
static int cli_qid = -1;

int main() {
	//Initializers
	int running = 1;
	struct msg_pass data_send;
	struct msg_rcv data_get;
	long int msg_to_receive = 0;
	char buffer[MAX_TEXT];

	//Getting Server and Client Message Queues
	serv_qid = msgget((key_t)SERVER_MQUEUE, 0666);
	if(serv_qid == -1) {
		fprintf(stderr, "msgget for server failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	cli_qid = msgget((key_t)CLIENT_MQUEUE, 0666);
	if(cli_qid == -1) {
		fprintf(stderr, "msgget for client failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	
	while(running) {
		//Receive and store given command
		printf("Enter a command: ");
		fgets(buffer, MAX_TEXT, stdin);
		data_send.msg_type = 1;
		strcpy(data_send.some_text, buffer);

		//Send command and check for error
		if(msgsnd(serv_qid, (void *)&data_send, MAX_TEXT, 0) == -1) {
			fprintf(stderr, "msgsnd for server failed\n");
			exit(EXIT_FAILURE);
		}

		//Receive all returned messages and check for error
		if(msgrcv(cli_qid, (void *)&data_get, MAX_TEXT, msg_to_receive, 0)==-1){
			fprintf(stderr, "msgrcv client failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		
		//Check for user inputted command and print appropriate returned message
		if(strncmp(data_send.some_text, "Sum", 3) == 0) {
			printf("Sum of values is: %.2f\n", data_get.standard);
		} else if(strncmp(data_send.some_text, "Average", 7) == 0) {
			printf("Average of values is: %.2f\n", data_get.standard);
		} else if(strncmp(data_send.some_text, "Min", 3) == 0) {
			printf("Minimum of values is: %.2f\n", data_get.standard);
		} else if(strncmp(data_send.some_text, "Median", 6) == 0) {
			if(data_get.optional == 0) {
				printf("Median of values is: %.2f\n", data_get.standard);
			} else {
				printf("Median of values is: %.2f %.2f\n", data_get.standard, data_get.optional);
			}
		} else if(strncmp(data_send.some_text, "end", 3) == 0) {
			running = 0;
		}
	}
	exit(EXIT_SUCCESS);
}
