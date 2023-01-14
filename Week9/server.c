#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>

#include "http.h"
#include "config.h"
#include "linkedlist.h"
#include "serverHelper.h"

int serv_sock;
XOR_LL client_list = XOR_LL_INITIALISER;
XOR_LL username_list = XOR_LL_INITIALISER;

void z_exit() {
  close(serv_sock);
  xor_ll_destroy(&client_list);
  xor_ll_destroy(&username_list);
  exit(SUCCESS);
}

bool z_find_username(char *username) {
  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&username_list, &itr) {
    char *curr_username = (char *)itr.node_data.ptr;
    if(strcmp(username, curr_username) == 0) return SUCCESS;
  }
  return FAILURE;
}

void z_load_username() {
  FILE *username_f = fopen(USN_F, "r");
  if(!username_f) {
    z_error(__func__, "Can't open username.txt");
    z_exit();
  }

  char line[USN_L];
  rewind(username_f);
  while (fgets(line, sizeof(line), username_f)) {
    xor_ll_push_tail(&username_list, line, strlen(line));
  }
  fclose(username_f);
}

void signalHandler(int signo) {
  switch (signo) {
    case SIGINT:
      z_warn("Caught signal Ctrl + C, coming out...\n");
      break;
    case SIGQUIT:
      z_warn("Caught signal Ctrl + \\, coming out...\n");
      break;
    case SIGHUP:
      z_warn("The terminal with the program (or some other parent if relevant) dies, coming out...\n");
      break;
    case SIGTERM:    /* Handle login */

      z_warn("The termination request (sent by the kill program by default and other similar tools), coming out...\n");
      break;
    case SIGUSR1:
      z_warn("Killing the program, coming out...\n");
      break;
    default:
      break;
  }

  z_exit();
}

Message z_parse(char *req) {
  Message msg;
  sscanf(req, "%s HTTP/1.1\r\nContent-Length: %d\r\n\r\n%[^\n]s", msg.header.command, &msg.header.content_length, msg.body.content);
  if(msg.header.content_length == 0)
    memset(msg.body.content, 0, CONTENT_L);
  return msg;
}

bool z_str_is_empty(char *str) {
  int length = (int)strlen(str);
  for(int i = 0; i < length; i++) {
    if(str[i] != 32)
      return FAILURE;
  }
  return SUCCESS;
}

void z_handle_message(int clnt_sock, Message msg, char *res) {
  char cmd[CMD_L], content[CONTENT_L];
  int content_length = msg.header.content_length;
  strcpy(cmd, msg.header.command);
  strcpy(content, msg.body.content);

  if (strcmp(cmd, "AUTH") == 0) {
    if(!z_is_usr(content) || strlen(content) < 4) {
      z_send_res(clnt_sock, res, 400, "Login name must be alpha [A-Za-z] or digit [0-9] and more than 4 characters");
      return;
    }

    FILE *username_f = fopen(USN_F, "a+");
    if (!username_f) {
      z_error(__func__ , "Can't open username.txt");
      z_exit();
    }

    if (content_length == 0 || z_str_is_empty(content)) {
      z_send_res(clnt_sock, res, 400, "Login name empty");
      return;
    }
    if (content_length > CONTENT_L) {
      z_send_res(clnt_sock, res, 400, "Login name too long ( > 50 )");
      return;
    }

    if (!z_find_username(content)) {
      fprintf(username_f, "%s\r\n", content);
      xor_ll_push_tail(&username_list, content, sizeof *content);
    }
    z_send_res(clnt_sock, res, 100, "Login successfully");
    ClientInfo *client = findClient(&client_list, clnt_sock);
    strcpy(client->username, content);
    fclose(username_f);

  } else if (strcmp(cmd, "MSG") == 0) {
    ClientInfo *client = findClient(&client_list, clnt_sock);
    if(strlen(client->username) == 0) {
      z_send_res(clnt_sock, res, -1, "Non authorized");
      return;
    }
    /* [file_name.log].length = USN_L.length + [.log].length */
    char log_f_name[USN_L + 4];
    sprintf(log_f_name, "%s.log", client->username);
    FILE *log_f = fopen(log_f_name, "a+");
    if (!log_f) {
      z_warn("Can't open [username].log");
      return;
    }

    fprintf(log_f, "%s\r\n", content);
    z_send_res(clnt_sock, res, 200, "Received");
    fclose(log_f);
  } else if (strcmp(cmd, "EXIT") == 0) {
    removeClient(&client_list, clnt_sock);
    close(clnt_sock);
  } else {
    z_send_res(clnt_sock, res, -1, "Invalid Message Command");
  }
}

void z_listen() {
  fd_set master, read_fds;
  int i, nbytes, fdmax;
  char *req, *res, req_time[100];
  time_t now = time(0);
  req = (char*) malloc(REQ_L);
  res = (char*) malloc(RES_L);


  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(serv_sock, &master);

  fdmax = serv_sock;

  while(1) {
    read_fds = master;

    if(select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
      printf("Server still listening...");
    }

    for(i = 0; i <= fdmax; i++) {
      if(FD_ISSET(i, &read_fds)) {
        now = time(0);
        strftime(req_time, 100, "%Y-%m-%d %H:%M:%S", localtime(&now));
        if(i == serv_sock) {
          Client client = z_accept(serv_sock);
          FD_SET(client.sock, &master);
          if(client.sock > fdmax) fdmax = client.sock;
          char address[ADDR_L];
          strcpy(address, z_socket_addr((struct sockaddr *) &client.addr));
          addClient(&client_list, client.sock, address);
          printf("\x1b[1;38;5;256m%s>\x1b[0m [@\x1b[1;38;5;202m%s\x1b[0m] \x1b[1;38;5;47mONLINE\x1b[0m\n", req_time, address);
        }
        else {
          z_clear(req, res);
          strcpy(req_time, "");
          ClientInfo *requester = findClient(&client_list, i);

          if((nbytes = z_get_req(i, req)) <= 0) {
            if(nbytes == 0) {
              printf("\x1b[1;38;5;256m%s>\x1b[0m [@\x1b[1;38;5;202m%s\x1b[0m] \x1b[1;38;5;226mOFFLINE\x1b[0m\n", req_time, requester->address);
            }
            else
              z_warn("Get request failed");
            removeClient(&client_list, i);
            close(i);
            FD_CLR(i, &master);
          }
          else {
            now = time(0);
            strftime(req_time, 100, "%Y-%m-%d %H:%M:%S", localtime(&now));
            Message msg = z_parse(req);
            printf("\x1b[1;38;5;256m%s>\x1b[0m [@\x1b[1;38;5;202m%s\x1b[0m] \x1b[1;38;5;47m%s\x1b[0m \x1b[4m%s\x1b[0m \x1b[1;38;5;226m%d\x1b[0m\n", req_time, requester->address, msg.header.command, msg.body.content, msg.header.content_length);
            z_handle_message(i, msg, res);
          }
        }
      }
    }
  }
}

void z_handle_signal() {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGHUP, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGUSR1, signalHandler);
}

int main(int argc,char *argv[]) {
  if(argc < 2 || !z_is_port(argv[1])) {
    z_error(__func__, "Invalid parameter\nUsage: ./server <port>\n");
    exit(FAILURE);
  }

  serv_sock = z_setup_server(argv[1]);
  xor_ll_init(&client_list);
  xor_ll_init(&username_list);

  z_handle_signal();
  z_load_username();
  z_listen();
  z_exit();

  return 0;
}