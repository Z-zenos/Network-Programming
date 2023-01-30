#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

// reverse the given null-terminated string in place
void inplace_reverse(char * str)
{
  if (str)
  {
    char * end = str + strlen(str) - 1;

    // swap the values in the two given variables
    // XXX: fails when a and b refer to same memory location
#   define XOR_SWAP(a,b) do\
    {\
      a ^= b;\
      b ^= a;\
      a ^= b;\
    } while (0)

    // walk inwards from both ends of the string, 
    // swapping until we get to the middle
    while (str < end)
    {
      XOR_SWAP(*str, *end);
      str++;
      end--;
    }
#   undef XOR_SWAP
  }
}


int main( int argc, char *argv[] )
{
    int sockfd, clisockfd, portno;
    char * start = "hello";
    char * end = "bye";
    socklen_t clilen;
    char buffer[256];
    char reversed[257];
    char contentBuffer[255];
    struct sockaddr_in serv_addr, cli_addr;
    int  n;
    //int optval;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        return(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5000;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);


    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        return(1);
    }

    listen(sockfd,5);
    clilen = (socklen_t) sizeof(cli_addr);

    clisockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (clisockfd < 0) 
    {
        perror("ERROR on accept");
        return(1);
    }

    while (strcmp(end, contentBuffer) !=0)
    {
        bzero(buffer,256);
        bzero(reversed,257);
        bzero(contentBuffer,255);
        /* If connection is established then start communicating */
        n = read( clisockfd,buffer,255 );
        buffer[n - 1] = '\0';
        if (n < 0)
        {
            perror("ERROR reading from socket");
            return(1);
        }

        strncpy(contentBuffer,buffer,strlen(buffer) - 1);  
        if (strcmp(start, contentBuffer) ==0)
        {
            printf("command: %s\n",buffer);
            n = write(clisockfd,"Roger that\n",11);
            if (n < 0)
            {
                perror("ERROR writing to socket");
                return(1);
            }
        }
        else 
        {
            inplace_reverse(buffer);
            printf("Unknown command: %s\n",buffer);
            sprintf(reversed, "%s\n", buffer);
            n = write(clisockfd,reversed,257);
            if (n < 0)
            {
                perror("ERROR writing to socket");
                return(1);
            }           
        }
    }
    close(sockfd);
    return 0;


}