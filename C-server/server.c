#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>   // stat
#include <stdbool.h>  
#include <libgen.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <strings.h>
#include <postgresql/libpq-fe.h>


#define PORT 5500
#define BACKLOG 20
#define BUFF_SIZE 1024

/* Handler process signal*/
void sig_chld(int signo);

/*
* Receive and echo message to client
* [IN] sockfd: socket descriptor that connects to client 	
*/
void echo(int sockfd);


void do_exit(PGconn *conn, PGresult *res) {
    
    fprintf(stderr, "%s\n", PQerrorMessage(conn));    

    PQclear(res);
    PQfinish(conn);    
    
    exit(1);
}


PGresult* queryDB(char *query, PGconn *conn){
    PGresult *res = PQexec(conn, query);   
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("No data retrieved\n");        
        PQclear(res);
        //do_exit(conn);
        return NULL;
    }

    printf("Success\n");
    return res;

}


int Login(int sockfd, PGconn *conn) {

	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;
	
	bytes_received = recv(sockfd, buff, BUFF_SIZE, 0); //blocking
	if (bytes_received < 0)
		perror("\nError: ");
	else if (bytes_received == 0)
		printf("Connection closed.\n");
	char* username = (char*)malloc(250*sizeof(char));
	strcpy(username,"SELECT * FROM public.taikhoan where ");
	buff[bytes_received - 1] = '\0'; 
	strcat(username, buff);
	strcat(username, "'");
	PGresult *res = PQexec(conn, username);   
    printf("buff: %s\n",username);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("No data retrieved\n"); 
		printf("%s\n",PQresultErrorMessage(res));     
		bytes_sent = send(sockfd, "s1", 2, 0); /* echo to the client */  
		return 0;
        // PQclear(res);
        // do_exit(conn);
    }  
	int rec_count = PQntuples(res);
	printf("count: %d\n",rec_count);
	if (rec_count == 0)
	{
		bytes_sent = send(sockfd, "s1", 2, 0); /* echo to the client */  
		return 0;
	}
	
	bytes_sent = send(sockfd, "s0", 2, 0); /* echo to the client */
	if (bytes_sent < 0)
		perror("\nError: ");
	return 1;
}

int addPlace(int sockfd, PGconn *conn) {

	char* place = (char*)malloc(BUFF_SIZE*sizeof(char));
	sprintf(place,"INSERT INTO public.diadiem VALUES (nextval('Address_id'), '%s', %d)","Ha Noi", 1);

	PGresult *res = PQexec(conn, place);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        do_exit(conn, res);    
	}
	free(place); 

}

void showHome(int sockfd, PGconn *conn){
	PGresult *res = PQexec(conn, "select * from public.diadiem VALUES(1,'Audi',52642)"); 
	int rec_count = PQntuples(res);
	char col[100]; 
	sprintf(col, "%d\n", rec_count);
	printf("Home\n");
    printf("We received %d records.\n", rec_count);
    puts("==========================");
	// int bytes_sent = send(sockfd, col, 2, 0); /* echo to the client */
	// if (bytes_sent < 0)
	// 	perror("\nError ");
    for (int row=0; row<rec_count; row++) {
		char value[BUFF_SIZE];
		memset(value,0,BUFF_SIZE);
        // for (int i = 1; i < 2; i++)
        // {
		// 	strcat(value,PQgetvalue(res, row, i));
        // }
		printf("%s", PQgetvalue(res, row, 1));
		int bytes_sent = send(sockfd, PQgetvalue(res, row, 1), BUFF_SIZE, 0); /* echo to the client */
		if (bytes_sent < 0)
			perror("\nError: ");
        printf("\n");
    }

	int a;
	switch (a)
	{
	case 1:
		
		break;
	
	default:
		break;
	}
}

void controller (int sockfd){

	PGconn *conn = PQconnectdb("user=postgres host=localhost password=1304 dbname=batman");
	if (PQstatus(conn) == CONNECTION_BAD) {   
        fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
        do_exit(conn);
    }
	
	// if (Login(sockfd, conn) == 1){
	// 	showHome(sockfd, conn);
	// }
	//Login(sockfd, conn);
	// printf("OKK\n");
	showHome(sockfd, conn);
	// printf("Done\n");
	close(sockfd);
	PQfinish(conn);
}

int main() {
    // Server

    int listen_sock, conn_sock; /* file descriptors */
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	pid_t pid;
	int sin_size;

	// Connect database
	// PGconn *conn = PQconnectdb("user=postgres host=localhost password=1304 dbname=batman");

	// if (PQstatus(conn) == CONNECTION_BAD) {   
    //     fprintf(stderr, "Connection to database failed: %s\n",
    //         PQerrorMessage(conn));
    //     do_exit(conn);
    // }

	if ((listen_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		printf("socket() error\n");
		return 0;
	}
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   

	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ 
		perror("\nError: ");
		return 0;
	}     

	if(listen(listen_sock, BACKLOG) == -1){  
		perror("\nError: ");
		return 0;
	}
	
	/* Establish a signal handler to catch SIGCHLD */
	signal(SIGCHLD, sig_chld);

    // Server

    while(1){
		sin_size=sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size))==-1){
			if (errno == EINTR)
				continue;
			else{
				perror("\nError: ");			
				return 0;
			}
		}
		
		/* For each client, fork spawns a child, and the child handles the new client */
		pid = fork();
		
		/* fork() is called in child process */
		if(pid  == 0){
			close(listen_sock);
			printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */
			controller(conn_sock);
			printf("done\n");
			//echo(conn_sock);					
			exit(0);
		}
		
		/* The parent closes the connected socket since the child handles the new client */
		close(conn_sock);
	}
	close(listen_sock);
	//return 0;

    // PQclear(res);
	close(conn_sock);

    return 0;
}

void sig_chld(int signo){
	pid_t pid;
	int stat;
	
	/* Wait the child process terminate */
	while((pid = waitpid(-1, &stat, WNOHANG))>0)
		printf("\nChild %d terminated\n",pid);
}