#include "utils.h"
#include "network.h"
#include "constants.h"

int main(int argc, char *argv[]) {
  char input[BUFFER];
  int option = parse_arguments(argc, argv, input);
  switch (option) {
    case 1:
      ip_to_hostname(input);
      break;

    case 2:
      hostname_to_ip(input);
      break;

    default:
      break;
  }

  return 0;
}