#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define SERVER_MQUEUE 1234
#define CLIENT_MQUEUE 4321
#define MAX_TEXT 35

//Node structure for linked-list
struct Node {
	double data;
	struct Node *next;
};

//Message structure to send back to User
struct msg_pass {
	long int msg_type;
	double standard;
	double optional;
};

//Message structure to receive from User
struct msg_rcv {
	long int msg_type;
	char some_text[MAX_TEXT];
};

static int serv_qid = -1;
static int cli_qid = -1;

void insertBegin(struct Node **start_ref, double data);
void deleteKey(struct Node **head_ref, double key);
void bubbleSort(struct Node *starter);
void swap(struct Node *a, struct Node *b);
void sumNode(struct Node *head, double* sum);
int getSize(struct Node *head);
double smallestElement(struct Node *head);
double getNth(struct Node *head, int index);
void median(struct Node *head, double arr[2]);

int main() {
	//Initializers
	int running = 1;
	struct msg_rcv data_get;
	struct msg_pass data_send;
	long int msg_to_receive = 0;

	struct timeval start, end;
	double insertTime = 0, deleteTime = 0, sumTime = 0, avgTime = 0, minTime = 0, medianTime = 0, averageTime = 0;

	struct Node *starter = NULL;

	//Initializing Server and Client Message Queues
	serv_qid = msgget((key_t)SERVER_MQUEUE, 0666 | IPC_CREAT);
	if(serv_qid == -1) {
		fprintf(stderr, "msgget for server failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	cli_qid = msgget((key_t)CLIENT_MQUEUE, 0666 | IPC_CREAT);
	if(cli_qid == -1) {
		fprintf(stderr, "msgget for client failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	//Loop to keep checking for messages received
	while(running) {
		//Receive messages and store in some_data.some_text
		if(msgrcv(serv_qid, (void *)&data_get, MAX_TEXT, msg_to_receive, 0) == -1) {
			fprintf(stderr, "msgrcv for server failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		//Check all the possible commands given
		if(strncmp(data_get.some_text, "Insert", 6) == 0) {
			gettimeofday(&start, NULL);
			memmove(data_get.some_text, data_get.some_text+7, MAX_TEXT);
			char *ptr;
			double num;
			num = strtod(data_get.some_text, &ptr);
			insertBegin(&starter, num);
			gettimeofday(&end, NULL);
			insertTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
		} else if(strncmp(data_get.some_text, "Delete", 6) == 0) {
			gettimeofday(&start, NULL);
			memmove(data_get.some_text, data_get.some_text+7, MAX_TEXT);
			char *ptr;
			long num;
			num = strtol(data_get.some_text, &ptr, 10);
			deleteKey(&starter, num);
			gettimeofday(&end, NULL);
			deleteTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
		} else if(strncmp(data_get.some_text, "Sum", 3) == 0) {
			gettimeofday(&start, NULL);
			double sum = 0;
			sumNode(starter, &sum);
			data_send.standard = sum;
			gettimeofday(&end, NULL);
			sumTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
		} else if(strncmp(data_get.some_text, "Average", 7) == 0) {
			gettimeofday(&start, NULL);
			double sum = 0;
			sumNode(starter, &sum);
			int size = 0;
			size = getSize(starter);
			double avg = sum/(double)size;
			data_send.standard = avg;
			gettimeofday(&end, NULL);
			avgTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
		} else if(strncmp(data_get.some_text, "Min", 3) == 0) {
			gettimeofday(&start, NULL);
			double min = 0;
			min = smallestElement(starter);
			data_send.standard = min;
			gettimeofday(&end, NULL);
			minTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
		} else if(strncmp(data_get.some_text, "Median", 6) == 0) {
			gettimeofday(&start, NULL);
			double values[2];
			median(starter, values);
			data_send.standard = values[0];
			data_send.optional = values[1];
			gettimeofday(&end, NULL);
			medianTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
		} else if(strncmp(data_get.some_text, "end", 3) == 0) {
			running = 0;
		}
		//Send result of given command
		data_send.msg_type = 1;
		if(msgsnd(cli_qid, (void *)&data_send, MAX_TEXT, 0) == -1) {
			fprintf(stderr, "msgsnd for client failed\n");
			exit(EXIT_FAILURE);
		}
	}

	//Calculate and Print average time for all commands
	averageTime = (insertTime+deleteTime+sumTime+avgTime+minTime+medianTime)/6;
	printf("Average time to perform all operations was %.6f ms\n", averageTime);	

	//Deleting Server and Client Message Queues
	if(msgctl(serv_qid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "msgctl(IPC_RMID) for server failed\n");
		exit(EXIT_FAILURE);
	}
	if(msgctl(cli_qid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "msgctl(IPC_RMID) for client failed\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

//Insert a new Node in linked list
void insertBegin(struct Node **start_ref, double data) {
	struct Node *ptr1 = (struct Node*)malloc(sizeof(struct Node));
	ptr1->data = data;
	ptr1->next = *start_ref;
	*start_ref = ptr1;
}

//Delete all instances of given key in linked list
void deleteKey(struct Node **head_ref, double key) {
	struct Node *temp = *head_ref, *prev;
	while(temp != NULL && temp->data == key) {
		*head_ref = temp->next;
		free(temp);
		temp = *head_ref;
	}
	while(temp != NULL) {
		while(temp != NULL && temp->data != key) {
			prev = temp;
			temp = temp->next;
		}
		if(temp == NULL) return;
		prev->next = temp->next;
		free(temp);
		temp = prev->next;
	}
}

//BubbleSort algorithm for linked list
void bubbleSort(struct Node *starter) {
	double swapped, i;
	struct Node *ptr1;
	struct Node *lptr = NULL;
	if(starter == NULL) {
		return;
	}
	do {
		swapped = 0;
		ptr1 = starter;
		while(ptr1->next != lptr) {
			if(ptr1->data > ptr1->next->data) {
				swap(ptr1, ptr1->next);
				swapped = 1;
			}
			ptr1 = ptr1->next;
		}
		lptr = ptr1;
	}
	while(swapped);
}

//Swap values in linked list
void swap(struct Node *a, struct Node *b) {
	double temp = a->data;
	a->data = b->data;
	b->data = temp;
}

//Sum all the nodes in linked list
void sumNode(struct Node *head, double* sum) {
	if(!head) return;
	sumNode(head->next, sum);
	*sum = *sum + head->data;
}

//Get the size of linked list
int getSize(struct Node *head) {
	int count = 0;
	struct Node *current = head;
	while(current != NULL) {
		count++;
		current = current->next;
	}
	return count;
}

//Get the smallest number in linked list
double smallestElement(struct Node *head) {
	double min = head->data;
	while(head != NULL) {
		if(head->data < min) {
			min = head->data;
		}
		head = head->next;
	}
	return min;
}

//Return value given index in linked list
double getNth(struct Node *head, int index) {
	struct Node *current = head;
	int count = 1;
	while(current != NULL) {
		if(count == index) return (current->data);
		count++;
		current = current->next;
	}
}

//Update array pointer for median value(s)
void median(struct Node *head, double arr[2]) {
	bubbleSort(head);
	int size = getSize(head)/2;
	if((getSize(head) % 2) == 0) {
		arr[0] = getNth(head,size);
		arr[1] = getNth(head,size+1);
	} else {
		arr[0] = getNth(head,size+1);
		arr[1] = 0;
	}
}	 
