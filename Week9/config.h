#define CMD_L 10
#define REQ_L 2099
#define RES_L 50
#define CONTENT_L 2048
#define BACKLOG 10
#define USN_L 50
#define ADDR_L 50

#define USN_F "username.txt"

#define SUCCESS 1
#define FAILURE 0

#define z_error(...) do{                                   \
	fprintf(stdout, "\x1b[1;38;5;196m[Error]\x1b[0m    ");   \
	fprintf(stdout, __VA_ARGS__);                            \
	fputs("\n", stdout);                                     \
  exit(FAILURE);                                           \
}while(0)

#define z_warn(...) do{                                    \
	fprintf(stdout, "\x1b[1;38;5;226m[Warn]\x1b[0m   ");     \
	fprintf(stdout, __VA_ARGS__);                            \
	fputs("\n", stdout);                                     \
}while(0)

#define z_success(...) do{                                 \
	fprintf(stdout, "\x1b[1;38;5;47m[Success]\x1b[0m    ");  \
	fprintf(stdout, __VA_ARGS__);                            \
	fputs("\n", stdout);                                     \
}while(0)