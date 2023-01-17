
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

int main() {
  char input[] = "hello";
  int length = strlen(input);
  SHA256_CTX context;
  unsigned char md[SHA256_DIGEST_LENGTH];
  SHA256_Init(&context);
  SHA256_Update(&context, (unsigned char *)input, length);
  SHA256_Final(md, &context);

  int i;
  for(i = 0; i < sizeof(md); i++) {
    printf("%0x", md[i]);
  }
  printf("\n");

  char password[] = "hello";
//  printf("Password: ");
//  scanf("%[^\n]s", password);

  unsigned char md1[SHA256_DIGEST_LENGTH];
  int pl = strlen(password);
  SHA256_Init(&context);
  SHA256_Update(&context, (unsigned char *)password, pl);
  SHA256_Final(md1, &context);

  char str[63];
  int k = 0;
  for(i = 0; i < sizeof(md1); i++) {
    printf("%0x", md1[i]);
    k += sprintf(str + k, "%x", md1[i]);
  }

  printf("\n%s\n", str);

  for(i = 0; i < sizeof md; i++) {
    if(md[i] != md1[i]) {
      printf("\nFAIL");
      return 0;
    }
  }
  printf("\nSuceess");
  return 0;
}