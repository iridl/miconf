
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>

/* Generic Proxy 1.1 */

static void sig_usr(int);

void Usage(void);

#define BUFSIZE 1024

int main(int argc,char *argv[])
{
    char c;

    struct hostent *hostptr;
    struct hostent *l_hostptr;
    int rnewsockfd;
    char rhost[100] = "127.0.0.1";
    char lhost[100] = "0.0.0.0";
    int rport = 9080;
    int lport = 9081;

    int len;
    char buffer[BUFSIZE];

    char logfile[MAXPATHLEN] = "";
    char filename[MAXPATHLEN];
    int status;
    int inf;
    int outf;
    int to_log=0;
    int in_bgr=1;

    int rpid,lpid;
    int sockfd, lnewsockfd, clilen;
    struct sockaddr_in cli_addr, serv_addr;
    
    

    while( (c = getopt(argc, argv, "A:a:S:s:R:r:L:l:D:d:Ffh")) != EOF ) {
	switch( toupper(c) ) {
	case 'S':
	    strcpy(rhost, optarg);
	    break;
	case 'A':
	    strcpy(lhost, optarg);
	    break;
	case 'R':
	    rport = atoi(optarg);
	    break;
	case 'L':
	    lport = atoi(optarg);
	    break;
	case 'D':
	    to_log = 1;
	    strcpy(logfile, optarg);
	    break;
	case 'F':
	    in_bgr = 0;
	    break;
	default:
	    Usage();
	    return -1;
	}
    }

    if( strcmp(rhost, "") == 0 ) {
	Usage();
	return(-1);
    }

    printf("Starting proxy from %s:%d to remote %s:%d\n", lhost, lport, rhost, 
	   rport);

    hostptr = gethostbyname(rhost);
    if( hostptr == NULL )
    {
      fprintf( stderr, "failed to resolve remote ip address: %s (h_errno=%d)\n",
               rhost, h_errno );
      exit(2);
    }
    l_hostptr = gethostbyname(lhost);
    if( l_hostptr == NULL )
    {
      fprintf( stderr, "failed to resolve local ip address: %s (h_errno=%d)\n",
               lhost, h_errno );
      exit(2);
    }
    
    /*
    ** Opening a socket 
    */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    (void) memcpy(&serv_addr.sin_addr.s_addr, *(l_hostptr->h_addr_list),
                      sizeof(serv_addr.sin_addr.s_addr)) ;
    serv_addr.sin_port = htons(lport);
    status = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&status, sizeof(int));
    if( bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
	perror("failed binding server socket");
	exit(1);
    }

    /*
    ** Start running in background by default
    */
    if( in_bgr && fork() ) {
	exit(0);
    }

    /* 
    ** Set queue length and signal interrupt
    */
    listen(sockfd,5);
    signal(SIGCHLD,sig_usr); 

    for ( ; ; ) {
   
	/*
	** Wait for new clients
	*/
	clilen = sizeof(cli_addr);
	lnewsockfd = -1;
	while( lnewsockfd < 0 ) {
	    lnewsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
	    if( lnewsockfd > 0 )
		break;

	    /*
	    ** It might happen that we got a signal and exited accept
	    ** In this case restart accept and continue to wait
	    */
	    if( (lnewsockfd == -1) && (errno == EINTR) ) 
		continue;

	    /*
	    ** Otherwise, we've just failed, exit
	    */
	    perror("failed in accept");
	    exit(0);
	}

	printf( "Connecting client from %s to %s:%d\n",
		inet_ntoa(cli_addr.sin_addr), rhost, rport);

	/*
	** Pass the new connection throught to the server
	*/
	rnewsockfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	(void) memcpy(&serv_addr.sin_addr.s_addr, *(hostptr->h_addr_list),
		      sizeof(serv_addr.sin_addr.s_addr)) ;
	serv_addr.sin_port = htons(rport);
	connect(rnewsockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	/*
	** Forking the baby-sitter
	*/
	if (fork() == 0) {
	    /*
	    ** Starting first child. It will read from client and sent to
	    ** remote server
	    */
	    if ((lpid = fork()) == 0) {
		if( strcmp(logfile, "") != 0 ) {
		    sprintf(filename, "%s.%d.out", logfile, getpid());
		    outf = open(filename, O_RDWR|O_TRUNC|O_CREAT, 0666);
		}
		while( (len = read(lnewsockfd, buffer, BUFSIZE)) > 0 ) {
		    if( len ) {
			if( outf > 0 )
			    write(outf, buffer, len);
			write(rnewsockfd, buffer, len);
		    }
		}
		
		/*
		** When finished reading, exit
		*/
		close(lnewsockfd);
		close(rnewsockfd);
		close(outf);
		exit(0);
	    } else if (lpid < 0) {
		/*
		** Fork failed
		*/
		perror("failed forking 1st child");
		exit(1);
	    }

	    /*
	    ** Start second child. It will read from the server and write
	    ** to our client
	    */
	    if ((rpid = fork()) == 0  ) {
		if( strcmp(logfile, "") != 0 ) {
		    sprintf(filename, "%s.%d.in", logfile, getpid());
		    inf = open(filename, O_RDWR|O_TRUNC|O_CREAT, 0666);
		}
		while( (len = read(rnewsockfd, buffer, BUFSIZE)) > 0) {
		    if( len ) {
			if( inf > 0 )
			    write(inf, buffer, len);
			write(lnewsockfd, buffer, len);
		    }
		}
		
		/*
		** When finished writing, exist
		*/
		close(lnewsockfd);
		close(rnewsockfd);
		close(inf);
		exit(0);
	    }
	    else if (rpid < 0) {
		/*
		** Fork failed
		*/
		perror("failed forking 2nd child");
		exit(1);
	    }
    
	    /*
	    ** Now baby-sitter waits for any child to finish
	    */
	    close(lnewsockfd);
	    close(rnewsockfd);
	    wait(&status);

	    /*
	    ** If any of the children terminated, kill all of them
	    */
	    kill(rpid,SIGKILL);
	    kill(lpid,SIGKILL);
	    exit(0);
	} /* baby-sitter */
	
	/* 
	** The main process closes files and goes on to wait for new clients
	*/
	close(lnewsockfd);
	close(rnewsockfd);
    } /*  forever */

} /* main */

void Usage()
{
    printf("Usage: vproxy [-a local_ip=0.0.0.0] [-l local_port=9081] [-s remote_ip=127.0.0.1] [-r remote_port=9080] [-d log_prefix]\n");

}

static void sig_usr(int signo)
{
    
    int status,pid;

    pid = wait(&status);
    signal(SIGCHLD,sig_usr);

}

