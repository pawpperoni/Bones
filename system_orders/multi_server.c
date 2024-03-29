/* 
 * multi_server - Opens a multi connection socket server
 * Default port 20448
 * Usage: multi_server [PORT]
 * Bruno Mondelo Giaramita
 * 2017-06-07
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//~ #include <sys/select.h>
//~ #include <sys/types.h>

#include <sys/socket.h>
#include <arpa/inet.h>
/* Socket includes */

#define PROGRAM_NAME "multi_server"
/* The program official name */

#define MAX_CONNECTIONS 10
/* The maximum connections to attend */

#define MAX_DATA_SIZE 2048
/* The maximum buffer for message */

int
init_socket (int port)
{
  
  /*
   * Creates a new socket
   * Input: int
   * Output: int (socket descriptor)
  */
  
  int new_socket;
  /* The new socket to create */
  
  struct sockaddr_in addr;
  /* The socket address */
  
  if ((new_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    fprintf(stderr, "%s - error allocating memory for socket\n",
      PROGRAM_NAME);
    return -1;
  }
  /* Allocate memory for the new socket */
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  /* Set the socket address variables */
  
  if (bind(new_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    fprintf(stderr, "%s - error binding socket to port : %d\n",
      PROGRAM_NAME, port);
    return -2;
  }
  /* Bind the new socket */
  
  if (listen(new_socket, 1) < 0)
  {
    fprintf(stderr, "%s - error setting socket to listen\n", 
      PROGRAM_NAME);
    return -3;
  }
  /* Set socket to listening */
  
  return new_socket;
  
}
/* The settings of the socket */

int
handle_new_connection (int mainsocket,
  int clientsockets[MAX_CONNECTIONS])
{
  
  /*
   * Function to handle a new connection
   * Input: int, int []
   * Output: int
  */
  
  int notfoundslot;
  /* Value to determine a found slot */
  
  int clientposition;
  /* The position on the array */
  
  int new_connection;
  /* The descriptor of the new connection */
  
  struct sockaddr_in addr;
  /* The client address */
  
  uint addrlen;
  /* The length of the address structure */
  
  addrlen = sizeof(addr);
  /* Asssign the length of the address */
  
  if ((new_connection = accept(mainsocket, (struct sockaddr *)&addr,
      &addrlen)) < 0)
  {
    fprintf(stderr, "%s - error accepting new connection from : %s\n",
      PROGRAM_NAME, inet_ntoa(addr.sin_addr));
    return -1;
  }
  /* Accept new connection */
  
  clientposition = 0;
  while (clientposition < MAX_CONNECTIONS && clientposition != -1)
  {
    if (clientsockets[clientposition] == 0)
    {
      printf("accepted new client from %s and placed in slot : %d\n",
        inet_ntoa(addr.sin_addr), clientposition);
      clientsockets[clientposition] = new_connection;
      clientposition = -1;
      continue;
    }
    /* Search if the slot is 0, meaning it's free */
    
    clientposition++;
  }
  /* Search for a free slot */
  
  if (clientposition != -1)
  {
    fprintf(stderr, "%s - error, no free space for client from : %s\n",
      PROGRAM_NAME, inet_ntoa(addr.sin_addr));
    return -2;
  }
  
  return 0;
  
}
/* The function to handle a new connection */

int
handle_client_change (int clientsocket, int slot)
{
  
  /*
   * Handle the client change of status
   * Input: int
   * Output: int
  */
  
  char buf[MAX_DATA_SIZE];
  /* The received message */
  
  int bytes;
  /* The number of bytes received */
  
  struct sockaddr_in addr;
  /* The client address */
  
  uint addrlen;
  /* The length of the address structure */
  
  addrlen = sizeof(addr);
  /* Asssign the length of the address */
  
  if (getpeername(clientsocket, (struct sockaddr *)&addr, &addrlen) < 0)
  {
    fprintf(stderr, "%s - error getting the address for client in\
slot : %d\n",
      PROGRAM_NAME, slot);
    return -1;
  }
  /* Get the address of the client */
  
  if ((bytes = recv(clientsocket, &buf, MAX_DATA_SIZE, 0)) <= 0)
  {
    fprintf(stderr, "error receiving message from client %s\n",
      inet_ntoa(addr.sin_addr));
    fprintf(stderr, "client %s closed connection\n",
      inet_ntoa(addr.sin_addr));
    return 1;
  }
  /* Get the client message */
  
  if ((bytes = send(clientsocket, &buf, strlen(buf), 0)) < 0)
  {
    fprintf(stderr, "error sending message to client %s\n",
      inet_ntoa(addr.sin_addr));
    fprintf(stderr, "client %s closed connection\n",
      inet_ntoa(addr.sin_addr));
    return 2;
  }
  /* Echo back the message */
  
  buf[strlen(buf) - 1] = '\0';
  printf("received message from %s : %s\n", inet_ntoa(addr.sin_addr),
    buf);
  /* Show message */
  
  return 0;
  
}
/* The function to handle a change in a connected client */

void
read_sockets (fd_set * socket_set, int mainsocket,
  int clientsockets[MAX_CONNECTIONS])
{
  
  /*
   * Function to handle the new socket changes
   * Input: fd_set *
   * Output:
  */
  
  if (FD_ISSET(mainsocket, socket_set))
    handle_new_connection (mainsocket, clientsockets);
  /* A new connection was added */
  
  for (int i = 0; i < MAX_CONNECTIONS; i++)
    if (FD_ISSET(clientsockets[i], socket_set))
      if (handle_client_change (clientsockets[i], i) != 0)
        clientsockets[i] = 0;
  /* Check for changes in the connected sockets, if the return is
      a non-zero number the client closed connection */
  
}
/* The function */

int
build_select_list (fd_set * socket_set, int mainsocket,
  int clientsockets[MAX_CONNECTIONS])
{
  
  /*
   * Function to build the select and return the high socket
   * Input: fd_set *
   * Output: int
  */
  
  int highsocket = mainsocket;
  /* The highest socket, by default is the main socket */
  
  FD_ZERO(socket_set);
  /* Clear the old set */
  
  FD_SET(mainsocket, socket_set);
  /* Set the first socket */
  
  for (int i = 0; i < MAX_CONNECTIONS; i++)
    if (clientsockets[i] > 0)
    {
      FD_SET(clientsockets[i], socket_set);
      if (clientsockets[i] > highsocket)
        highsocket = clientsockets[i];
    }
  /* Add all the client sockets and search for the higher one */
  
  return highsocket;
  
}
/* The select cleanup and configuration */

int
main (int argc, char * argv[])
{
  
  int port;
  /* The service port by default 20448 */
  
  int mainsocket;
  /* The main socket for attend new connections */
  
  int highsocket;
  /* The highest socket */
  
  int connectedlist[MAX_CONNECTIONS];
  /* The list of connected clients */
  
  fd_set socketset;
  /* The set of sockets */
  
  int readsocks;
  /* Number of sockets ready for reading */
  
  struct timeval timeout;
  /* The timeout for select */
  
  if (argc > 2)
  {
    fprintf(stderr, "%s - error, bad usage\n", PROGRAM_NAME);
    fprintf(stderr, "usage: %s [PORT]\n", PROGRAM_NAME);
    return -1;
  }
  else if (argc == 2)
  {
    port = atoi (argv[1]);
  }
  else
  {
    port = 20448;
  }
  /* Parse arguments. Assign the port of the server, by default if 
      no argument is passed is the 20448 */
  
  if ((mainsocket = init_socket (port)) < 0)
    return 3;
  printf("%s - openned socket via port : %d\n", PROGRAM_NAME, port);
  /* Create the new socket*/
  
  memset(&connectedlist, 0, sizeof(connectedlist));
  /* Fill all the connections list with zero, meaning free slots */
  
  while (1)
  {
    
    highsocket = build_select_list(&socketset, mainsocket,
      connectedlist);
    /* Clean all the select list and search for the highest socket */
    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    /* Set the timeout to one second */
    
    readsocks = select(highsocket + 1, &socketset, NULL, NULL, &timeout);
    /* Read the select */
    
    if (readsocks < 0)
    {
      fprintf(stderr, "%s - error while doing the select\n");
      return 4;
    }
    /* Attend an error ocurred while doing the select */
    
    //~ if (readsocks == 0)
      //~ printf("timeout ended...\n")
    else
      read_sockets(&socketset, mainsocket, connectedlist);
    /* Check for the timeout and if a change has ocurred attend it */
    
  }
  /* Stay listening for new connections */
  
  return 0;
  
}
