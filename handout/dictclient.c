/*
 * dictclient.c - A simple dictionary client
 * A lab by Will Fung and Jian Lu
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFSIZE 1024

void *thread_func(void *vargp);

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
  char *res;
  char buf[BUFSIZE];

  /* get word from stdin */
  printf("Please enter a phrase: ");
  bzero(buf, BUFSIZE);
  res = fgets(buf, BUFSIZE, stdin);
  if (res == NULL) error("ERROR reading from stdin\n");
  char *token;
  char *word = res;
  char *save;
  int counter = 0;

  pthread_t **tidarray = malloc(50 * sizeof(pthread_t*));
  char **wordarray = malloc(50 * sizeof(char*));

  while ((token = strtok_r(word, " ", &save))) {
    word = NULL;

    char *heapword = malloc(strlen(token) * sizeof(char));
    pthread_t *tidtemp = malloc(sizeof(pthread_t));

    strcpy(heapword, token);
    wordarray[counter] = heapword;
    tidarray[counter] = tidtemp;
    //create new peer thread, call thread_func
    if (pthread_create(tidarray[counter], NULL, thread_func, heapword) != 0) {
      printf("error creating thread\n");
      exit(1);
    }
    counter++;
  }

  //wait for peer thread to complete before exiting
  for (int i = 0; i < counter; i++){
    if (pthread_join(*(tidarray[i]), (void **)(&wordarray[i])) != 0) {
      printf("error joining thread\n");
      exit(1);
    }

  }
  printf("New phrase:");

  for (int i = 0; i < counter; i++){
    printf(" %s", wordarray[i]);
  }

  printf("\n");
  for (int i = counter; i > 0; i--) {
    free(tidarray[i]);
    free(wordarray[i]);
  }
  free(wordarray);
  free(tidarray);
  return 0;
}


/* thread routine */
void *thread_func(void *vargp) {
  char *trimmed_syn="";
  int sockfd, n, status;
  struct addrinfo hints;
  struct addrinfo *servinfo;
  char *hostname, *portno;
  char buf[BUFSIZE];
  char prefix[100] = "define moby-thesaurus ";

  hostname = "www.dict.org";
  portno = "2628";

  memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
  hints.ai_family = AF_INET;        // guess that we're using IPv4
  hints.ai_socktype = SOCK_STREAM; // guess that we're using TCP

  status = getaddrinfo(hostname, portno, &hints, &servinfo);
  if (status)  error("ERROR, no such host");

  sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if (sockfd < 0) error("ERROR opening socket");

  if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
   error("ERROR connecting");


  // printf("%s\n", (char*) vargp);
  char *word = (char*) vargp;
  strcat(prefix, word);
  strcat(prefix, "\n");

  /* Writing to the server */
  n = write(sockfd, prefix, strlen(prefix));
  if (n < 0) error("Error Writing to socket");

  /* read: print the server's reply */
  //bzero(buf, BUFSIZE);
  //n = read(sockfd, buf, BUFSIZE);
  //if (n < 0) error("ERROR reading from socket");

  trimmed_syn = find_synonym(buf, sockfd);
  if (trimmed_syn == NULL) {
    // printf("Synonym: %s\n", word);
    trimmed_syn = word;
  } else {
    // printf("Synonym: %s \n", trimmed_syn);
  }

  close(sockfd);
  freeaddrinfo(servinfo);

  // vargp = trimmed_syn
  char *result = malloc(strlen(trimmed_syn) * sizeof(char));
  strcpy(result, trimmed_syn);

  // return 0;
  free(word);
  return result;
}
