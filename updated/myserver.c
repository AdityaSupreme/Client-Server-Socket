#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define USER 0
#define ADMIN 2
#define UNAUTH_USER -1
#define RESPONSE_BYTES 512
#define REQUEST_BYTES 512
#define linesInMS 5
#define EXIT -1

typedef struct login{
	char username[100];
	char password[100];
	char type;
}user;


void sendMsgtoClient(int clientFD, char *str) {
    int numPacketsToSend = (strlen(str)-1)/RESPONSE_BYTES + 1;
    int n = write(clientFD, &numPacketsToSend, sizeof(int));
    char *msgToSend = (char*)malloc(numPacketsToSend*RESPONSE_BYTES);
    strcpy(msgToSend, str);
    int i;
    for(i = 0; i < numPacketsToSend; ++i) {
        int n = write(clientFD, msgToSend, RESPONSE_BYTES);
        msgToSend += RESPONSE_BYTES;
    }
}

char* recieveMsgFromClient(int clientFD) {
    int numPacketsToReceive = 0;
    int n = read(clientFD, &numPacketsToReceive, sizeof(int));
    if(n <= 0) {
        shutdown(clientFD, SHUT_WR);
        return NULL;
    }
    char *str = (char*)malloc(numPacketsToReceive*REQUEST_BYTES);
    memset(str, 0, numPacketsToReceive*REQUEST_BYTES);
    char *str_p = str;
    int i;
    for(i = 0; i < numPacketsToReceive; ++i) {
        int n = read(clientFD, str, REQUEST_BYTES);
        str = str+REQUEST_BYTES;
    }
    return str_p;
}


void getupcli(char *username,char *password,int client_fd)
{
	char *ruser,*rpass;
	sendMsgtoClient(client_fd,"Enter Username: ");
	ruser=recieveMsgFromClient(client_fd);

	sendMsgtoClient(client_fd,"Enter Password: ");
	rpass=recieveMsgFromClient(client_fd);

	int i=0;
	while(ruser[i]!='\0' && ruser[i]!='\n')
	{
		username[i]=ruser[i];
		i++;
	}

	username[i]='\0';

	i=0;
	while(rpass[i]!='\0' && rpass[i]!='\n')
	{
		password[i]=rpass[i];
		i++;
	}
	password[i]='\0';

}


char *printMiniStatement(char *username,int client_fd)
{
	int fp = open(username, O_RDONLY);
	char *current_balance = (char *)malloc(20*sizeof(char));
	//read(fp, current_balance, 20);
	lseek(fp, 20, SEEK_CUR);
	char *miniStatement = (char *)malloc(1000*sizeof(char));
	
	read(fp, miniStatement, 1000);
	//printf("%s", miniStatement);
	
	
   
	return miniStatement;
}



char *printBalance(char *username)
{
	int fd = open(username, O_RDONLY);
	char *current_balance = (char *)malloc(20*sizeof(char));
	read(fd, current_balance, 20);

	return current_balance;
}


void updateTrans(char *username,int choice,double balance)
{
	int fp = open(username, O_WRONLY | O_APPEND);
	int fpb = open(username, O_WRONLY);
	char *buff = (char *)malloc(20*sizeof(char));
	write(fpb, buff, 20);
	
	int length = sprintf(buff, "%20f",balance);
	lseek(fpb, 0, 0);
	write(fpb, buff, length);
	close(fpb);	
	
	char c=(choice==1)?'C':'D';
	
	char buffer[100];
	time_t ltime; /* calendar time */
	ltime=time(NULL); /* get current cal time */

	length = sprintf(buffer,"%.*s %f %c\n",(int)strlen(asctime(localtime(&ltime)))-1 , asctime(localtime(&ltime)), balance, c);
	
	write(fp, buffer, length);

	close(fp);

	free(buff);
	

}

int Credit(char *username, int client_fd){

char usrupdate[] = "User updated successfully.\n--------------------\n\nWrite exit for quitting.";


	int fd = open(username, O_RDWR);

	struct flock fl;
	memset(&fl, 0, sizeof(fl));
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 20;
	// blocking mode F_SETLK
	if(fcntl(fd, F_SETLK, &fl) == -1){
		sendMsgtoClient(client_fd, "\n\nAnother Transaction is Being Processed either Type back or exit to quit\n");
		close(fd);
		return 0;
		printf("cannot write lock\n");
		exit(1);
	}

	double balance=strtod(printBalance(username),NULL);
	double amount=0.0;
	sendMsgtoClient(client_fd, "Enter Amount");
	while(1){
		char *buff=recieveMsgFromClient(client_fd);
		amount=strtod(buff,NULL);
		if(amount<=0)
		sendMsgtoClient(client_fd,"Enter valid amount");
		else
		   break;
		}
	balance +=amount;
	updateTrans(username,'C',balance);
	sendMsgtoClient(client_fd,usrupdate);

	fl.l_type = F_UNLCK;
	if(fcntl(fd, F_SETLK, &fl) == -1){
		printf("unlocked fail\n");
		exit(1);
	}

	close(fd);
}




int Debit(char *username, int client_fd){

char usrupdate[] = "User updated successfully.\n--------------------\n\nWrite exit for quitting.";
char usrupdateu[] = "Update UnsucessFull Low balance\n--------------------\n\nWrite exit for quitting.";


	int fd = open(username, O_RDWR);

	struct flock fl;
	memset(&fl, 0, sizeof(fl));
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 20;
	// blocking mode F_SETLK
	if(fcntl(fd, F_SETLK, &fl) == -1){
		sendMsgtoClient(client_fd, "\n\nAnother Transaction is Being Processed either Type back or exit to quit\n");
		close(fd);
		return 0;
		printf("cannot write lock\n");
		exit(1);
	}

	double balance=strtod(printBalance(username),NULL);
	double amount=0.0;
	sendMsgtoClient(client_fd, "Enter Amount");
	while(1){
		char *buff=recieveMsgFromClient(client_fd);
		amount=strtod(buff,NULL);
		if(amount<=0)
		sendMsgtoClient(client_fd,"Enter valid amount");
		else
		   break;
		}
		if(balance >= amount){
			balance -=amount;
			updateTrans(username,'D',balance);
			sendMsgtoClient(client_fd,usrupdate);
		}
			
		else{
			sendMsgtoClient(client_fd,usrupdateu);					
		}
	
	fl.l_type = F_UNLCK;
	if(fcntl(fd, F_SETLK, &fl) == -1){
		printf("unlocked fail\n");
		exit(1);
	}
	
	close(fd);
}

int Passwordchng(char *username, int client_fd){
char usrupdate[] = "Password updated successfully.\n--------------------\n\nWrite exit for quitting.";

	int fd = open("login.dat", O_RDWR);
	user login;
	char *pass = NULL;
	char *cpass = NULL;
	while(read(fd, &login, sizeof(login)) > 0){
		if(strcmp(login.username, username) == 0){
			sendMsgtoClient(client_fd, "Enter New Password::");
			ReEntry:
			pass = recieveMsgFromClient(client_fd);
			sendMsgtoClient(client_fd, "ReEnter New Password::");
			cpass = recieveMsgFromClient(client_fd);
			if(strcmp(pass,cpass) == 0){
				lseek(fd, -1*sizeof(user), SEEK_CUR);
				strcpy(login.password, pass);
				write(fd, &login, sizeof(login));
				sendMsgtoClient(client_fd, usrupdate);
				return 1;		
			}
			else{
				sendMsgtoClient(client_fd, "Passwords do not match\nEnter New Password::");
				goto ReEntry;			
			}
		}
	
	}
	return 0;
}


void userRequests(char *username,char *password,int client_fd)
{
	int flag=1;
	char option[] = "\n------------------\n\nEnter your choice\n1. Available Balance\n2. Mini Statement\n3. Deposit\n4. WithDraw\n5. Password Change\n6. View Details\nWrite exit for quitting.";
	sendMsgtoClient(client_fd,option);
	
	char *buff=NULL;
	while(flag)
	{
		if(flag == 121){
			flag = 1;
			sendMsgtoClient(client_fd,option);		
		}		
		
		if(buff!=NULL)
			buff=NULL;
		buff=recieveMsgFromClient(client_fd);

		int choice;

		if(strcmp(buff,"exit")==0)
			choice=7;
		else
		    choice=atoi(buff);
		char *bal,*str;
		bal=(char *)malloc(1000*sizeof(char));
		str=(char *)malloc(100000*sizeof(char));
		strcpy(bal,"------------------\nAvailable Balance: ");
		strcpy(str,"------------------\nMini Statement: \n");
		
		switch(choice)
		{
			case 1:
				strcat(bal,printBalance(username));
				sendMsgtoClient(client_fd,strcat(bal,option));
				free(bal);
				break;
			case 2:
				strcat(str, printMiniStatement(username,client_fd));
			 	sendMsgtoClient(client_fd,strcat(str,option));
				free(str);
				break;
			case 3:
				Credit(username, client_fd);
				buff=recieveMsgFromClient(client_fd);
				if(strcmp(buff,"exit")==0)
					flag = 0;
				else
					flag = 121;
				break;
			case 4:
				Debit(username, client_fd);
				buff=recieveMsgFromClient(client_fd);
				if(strcmp(buff,"exit")==0)
					flag = 0;
				else
					flag = 121;
				break;
			case 5:
				Passwordchng(username, client_fd);
				buff=recieveMsgFromClient(client_fd);
				if(strcmp(buff,"exit")==0)
					flag = 0;
				else
					flag = 121;
				break;
			case 7:
				flag=0;
				break;
			default:
				sendMsgtoClient(client_fd, "Unknown Query");
				break;
		}
	}
}


int checkUser(char *username)
{

	user usr;

	int fd = open("login.dat", O_RDONLY);

	while((read(fd, &usr, sizeof(usr))) > 0) 
	{
		if(strcmp(usr.username,username)==0){
			if(usr.type =='C')
			{
				close(fd);
				return 1;
			}
        }
    }

    close(fd);
    return 0;


}



int query(char *username, int client_fd)
{
	double amount=0.0;
	char option []= "Choose an option\n1. Credit\n2. Debit\nWrite exit to terminate";
	char usrupdate[] = "User updated successfully.\n--------------------\nWrite quit to terminate current\nChoose an option\n1. Credit\n2. Debit\n";

	char usrupdateu[] = "Update UnsucessFull Low balance\n--------------------\nWrite quit to terminate current\nChoose an option\n1. Credit\n2. Debit\n";

	sendMsgtoClient(client_fd,option);
	while(1)
	{
		char* buff=recieveMsgFromClient(client_fd);

		if(strcmp(buff,"quit")==0)
			return EXIT;
		else
		{
			int choice=atoi(buff);
			double balance=strtod(printBalance(username),NULL);
			
			switch(choice){
			
				case 1:
					sendMsgtoClient(client_fd,"Enter amount");
					while(1){
					char *buff=recieveMsgFromClient(client_fd);
					amount=strtod(buff,NULL);
					if(amount<=0)
						sendMsgtoClient(client_fd,"Enter valid amount");
					else
					   break;
					}
					balance +=amount;
					updateTrans(username,choice,balance);
					sendMsgtoClient(client_fd,usrupdate);
					break;
				case 2:
					sendMsgtoClient(client_fd,"Enter amount");
					while(1){
					char *buff=recieveMsgFromClient(client_fd);
					amount=strtod(buff,NULL);
					if(amount<=0)
						sendMsgtoClient(client_fd,"Enter valid amount");
					else
					   break;
					}
					if(balance >= amount){
						balance -=amount;
						updateTrans(username,choice,balance);
						sendMsgtoClient(client_fd,usrupdate);
					}
					
					else{
						sendMsgtoClient(client_fd,usrupdateu);					
					}
					break;
				
				default:
					sendMsgtoClient(client_fd,option);
					break;
			}

		}
	}
}

void AddUser(char *name, char *pass, char *type, int client_fd){
	
	user newuser;
	strcpy(newuser.username, name);
	strcpy(newuser.password, pass);
	newuser.type = *type;
	int fd = open("login.dat", O_WRONLY | O_APPEND);
	write(fd, &newuser,  sizeof(newuser));
	close(fd);

	creat(name, 0766);
	fd = open(name, O_WRONLY | O_APPEND, 0766);
	double balance = 1000.000000000000000;
	char buff[20] = {0};
	sprintf(buff, "%f", balance);
	write(fd, buff, sizeof(buff));
	close(fd);
}	


void adminRequests(char *username, int client_fd)
{
char options[] = "\n1.Add User\n2.Access User\n3.Delete User\n4.Change Password\nWrite exit to quit";
	sendMsgtoClient(client_fd, options);	
char *dat = (char *)malloc(1000*sizeof(char));
char *name = NULL;
while(1){	
	char *buff=NULL;
	buff=recieveMsgFromClient(client_fd);	
	if(strcmp(buff,"exit")==0)
		break;
	else{	
		int choice=atoi(buff);
		switch(choice){
			case 1:
				sendMsgtoClient(client_fd,"Enter username::");
				name = NULL;
			NAME:				
				name = recieveMsgFromClient(client_fd);
				if(checkUser(name)){
		                    sendMsgtoClient(client_fd, "UserName already exists\nEnter username::");	
				    goto NAME;		
				}
				sendMsgtoClient(client_fd, "Enter Password::");
				char *pass = NULL, *cpass = NULL;
				ReEntry:
				pass = recieveMsgFromClient(client_fd);
				sendMsgtoClient(client_fd, "ReEnter New Password::");
				cpass = recieveMsgFromClient(client_fd);
				if(strcmp(pass,cpass) == 0){
					sendMsgtoClient(client_fd, "\n\nTYPE OF ACCOUNT\nC.Current\nJ.Joint\n");
					char *type = recieveMsgFromClient(client_fd);
					AddUser(name, pass, type,client_fd);		
				}
				else{
					sendMsgtoClient(client_fd, "Passwords do not match\nEnter New Password::");
					goto ReEntry;			
				}
				
				strcpy(dat, "USER ADDED SUCCESSFULLY----------------\n");
				sendMsgtoClient(client_fd, strcat(dat,options));
				break;
			case 2:
				sendMsgtoClient(client_fd,"Enter username of the account holder or 'exit' to quit");
				name = NULL;
				name = recieveMsgFromClient(client_fd);
				if(checkUser(name)){
					userRequests(name, NULL, client_fd);				
				}
				else
					sendMsgtoClient(client_fd,"Wrong Username. Please enter a valid user or exit to quit");
				break;
			case 3:
				break;
			case 4:
				Passwordchng(username, client_fd);
				break;
			default:
				strcpy(dat, "Invalid Choice"); 
				sendMsgtoClient(client_fd, strcat(dat,options));
				//free(dat);
				break;
		}	
	}
}
}

int authorize(char* username,char *password)
{
	printf("Authorizing\n");
        ssize_t readc;
	
	user usr;
	int fd = open("login.dat", O_RDONLY);

	while((readc = read(fd, &usr, sizeof(usr))) > 0) 
	{
		
		if(strcmp(usr.username,username)==0)
		{

			if(strcmp(usr.password,password)==0)
			{
				
                if(usr.type=='C' || usr.type == 'J')
                {
		    close(fd);
                    return USER;    //return the user type
                }
                else if(usr.type =='A')
                {
                    close(fd);
                    return ADMIN;
                }
               
            }
        }
    }

    	close(fd);
	return UNAUTH_USER;
}





void closeclient(int client_fd,char *str)
{
	sendMsgtoClient(client_fd, str);
    shutdown(client_fd, SHUT_RDWR);
}



void talkToClient(int client_fd)
{
	char *username,*password;
	username=(char *)malloc(100);
	password=(char *)malloc(100);
	int utype;
	
	getupcli(username,password,client_fd);
	utype=authorize(username,password);

	char *str=(char *)malloc(sizeof(char)*60);
	strcpy(str,"Thanks ");

	switch(utype)
	{
		case USER:
			printf("User is IN \n");
			userRequests(username,password,client_fd);
			closeclient(client_fd,strcat(str,username));
			break;
		case ADMIN:
			printf("ADMIN is IN \n");
			adminRequests(username, client_fd);
			closeclient(client_fd,strcat(str,username));
			break;	
		
		case UNAUTH_USER:
			closeclient(client_fd,"unauthorised");
			break;
		default:
			closeclient(client_fd,"unauthorised");
			break;
	}
}



int main(int argc,char **argv)
{
	int sock_fd,client_fd,port_no;
	struct sockaddr_in serv_addr, cli_addr;

	memset((void*)&serv_addr, 0, sizeof(serv_addr));
	port_no=atoi(argv[1]);

	sock_fd=socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_port = htons(port_no);         //set the port number
	serv_addr.sin_family = AF_INET;             //setting DOMAIN
	serv_addr.sin_addr.s_addr = INADDR_ANY;     //permits any incoming IP

	if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	    printf("Error on binding.\n");
	    exit(EXIT_FAILURE);
	}
	int reuse=1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	listen(sock_fd, 5); 
	int clisize=sizeof(cli_addr);

	while(1) {
	    //blocking call
	    memset(&cli_addr, 0, sizeof(cli_addr));
	    if((client_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &clisize)) < 0) {
	        printf("Error on accept.\n");
	        exit(EXIT_FAILURE);
	    }

	    switch(fork()) {
	        case -1:
	            printf("Error in fork.\n");
	            break;
	        case 0: {
	            close(sock_fd);
	            talkToClient(client_fd);
	            exit(EXIT_SUCCESS);
	            break;
	        }
	        default:
	            close(client_fd);
	            break;
	    }
	}

}
