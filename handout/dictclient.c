/* 
 * dictclient.c - A simple dictionary client
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <ctype.h>

#define BUFSIZE 1024

/* error - wrapper for perror */
void error(char *msg) {
  perror(msg);
  exit(1);
}

/*
 * This function returns a pointer to a substring of 
 * the original string str with white leading and trailing
 * white space removed.  It is used by find_synonym().
 */
char *trim_whitespace(char *str) {
  char *end;

  if (str == NULL) return NULL;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  // Is string all spaces?
  if(*str == 0) return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator at end of string
  *(end+1) = 0;

  return str;
}

/*
 * Parse result from dictionary server.  Return pointer to string.
 */
char *find_synonym(char* fromserver, int fd) {
  char *syn, *trimsyn, *save;
  int n;
  
  /* Read the server's reply.  Continue reading until 
   * line beginning with 151 is found.
   */
  bzero(fromserver, BUFSIZE);
  while ((n = read(fd, fromserver, BUFSIZE)) !=0){
    if (n < 0) {
      error("ERROR reading from socket");
      break;
    }
    
    //found a synonym!
    if(strstr(fromserver, "151") != NULL) break;
    
    //no synonym found
    if(strstr(fromserver, "552") != NULL) return NULL;
    
    bzero(fromserver, BUFSIZE);
  }
  
  //Parse result. Find first synonym
  syn = strtok_r(fromserver, ",", &save);
  syn = strtok_r(NULL, ":", &save);
  syn = strtok_r(NULL, ",", &save);
  
  //Remove leading and trailing white space
  trimsyn = trim_whitespace(syn);

  return trimsyn;
}

/*
 * Handle socket setup.  Return socket fd.
 */

int setup_socket() {

  return 0;
}


int main(int argc, char **argv) {
  char *trimmed_syn="";
  
  /* get word from stdin */
  printf("Please enter word: ");

  /* print result to stdout */
  printf("Synonym: %s \n", trimmed_syn);
  
  return 0;
}

