#include "webthreads/webthreads.h"
#include "utilities.h"
#include "strutilities.h"
#include <netinet/in.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

const char *GET = "GET";

static volatile int static_requests = 0;
static volatile int query_requests = 0;

const int MAX_CWD = 100;

void writeln_to_socket(int sockfd, const char *message)
{
  write(sockfd, message, strlen(message));
  write(sockfd, "\r\n", 2);
}

void write_content_to_socket(int sockfd, const char *content)
{
  char length_str[100];
  sprintf(length_str, "%d", (int)strlen(content));

  char *content_length_str = concat("Content-Length: ", length_str);

  writeln_to_socket(sockfd, "Server: FastkaServer/1.0");
  writeln_to_socket(sockfd, "Content-Type: text/html");
  writeln_to_socket(sockfd, content_length_str);
  writeln_to_socket(sockfd, "");
  writeln_to_socket(sockfd, content);

  free(content_length_str);
}

void http_404_reply(int sockfd)
{
  writeln_to_socket(sockfd, "HTTP/1.1 404 Not Found");

  static const char *content =
      "<html>\r\n"
      "<head>\r\n"
      "<title>404 Not Found</title>\r\n"
      "<style>\r\n"
      "body {\r\n"
      "    font-family: Arial, sans-serif;\r\n"
      "    background-color: #f8f9fa;\r\n"
      "    margin: 0;\r\n"
      "    padding: 0;\r\n"
      "}\r\n"
      ".container {\r\n"
      "    max-width: 800px;\r\n"
      "    margin: 100px auto;\r\n"
      "    text-align: center;\r\n"
      "}\r\n"
      "h1 {\r\n"
      "    color: #dc3545;\r\n"
      "}\r\n"
      "</style>\r\n"
      "</head>\r\n"
      "<body>\r\n"
      "<div class=\"container\">\r\n"
      "    <h1>404 Not Found</h1>\r\n"
      "    <p>The requested URL was not found on this server.</p>\r\n"
      "</div>\r\n"
      "</body>\r\n"
      "</html>\r\n";

  write_content_to_socket(sockfd, content);
}

void http_get_reply(int sockfd, const char *content)
{
  writeln_to_socket(sockfd, "HTTP/1.1 200 OK");
  write_content_to_socket(sockfd, content);
}

int is_get(char *text)
{
  return starts_with(text, GET);
}

char *get_path(char *text)
{
  int beg_pos = strlen(GET) + 1;
  char *end_of_path = strchr(text + beg_pos, ' ');
  int end_pos = end_of_path - text;

  int pathlen = end_pos - beg_pos;
  char *path = malloc(pathlen + 1);
  substr(text, beg_pos, pathlen, path);
  path[pathlen] = '\0';

  return path;
}

int is_query_request(const char *path)
{
  printf("Path: %s\n", path);
  if (contains(path, "/query"))
  {
    printf("Query request\n");
    return 1;
  }
  return 0;
}

char *read_file(FILE *fpipe)
{
  int capacity = 10;
  char *buf = malloc(capacity);
  int index = 0;

  int c;
  while ((c = fgetc(fpipe)) != EOF)
  {
    assert(index < capacity);
    buf[index++] = c;

    if (index == capacity)
    {
      char *newbuf = malloc(capacity * 2);
      memcpy(newbuf, buf, capacity);
      free(buf);
      buf = newbuf;
      capacity *= 2;
    }
}

  buf[index] = '\0';
  return buf;
}

struct request_pair
{
  char *path;
  char *query;
};

struct request_pair extract_query(const char *cgipath_param)
{
  struct request_pair ret;
  char *qq = strchr(cgipath_param, '?');

  if (qq == NULL)
  {
    ret.path = strdup(cgipath_param);
    ret.query = NULL;
  }
  else
  {
    int path_len = qq - cgipath_param;
    ret.path = malloc(path_len + 1);
    strncpy(ret.path, cgipath_param, path_len);
    ret.path[path_len] = 0;

    int query_len = strlen(cgipath_param) - path_len - 1;
    ret.query = malloc(query_len + 1);
    const char *query_start_pos = cgipath_param + path_len + 1;
    strncpy(ret.query, query_start_pos, query_len);
    ret.query[query_len] = '\0';
  }

  return ret;
}

void query_function(const char *query)
{
  printf("hello from query function\n");
}

void run_query(int sockfd, const char *curdir, const char *cgipath_param)
{
  // Extract the request path and query parameters
  struct request_pair req = extract_query(cgipath_param);

  // Check if the request path matches a specific function
  if (strcmp(req.path, "/query") == 0)
  {
    // Call the C function with the query parameter
    query_function(req.query);
  }
  else
  {
    // Handle unknown CGI paths (e.g., return a 404 response)
    http_404_reply(sockfd);
  }

  // Free allocated memory
  free(req.path);
  free(req.query);
}

void output_static_file(int sockfd, const char *curdir, const char *path)
{

  static_requests++;

  printf("Static request No. : %d\n", static_requests);

  clock_t start_time = clock();

  char *fullpath = malloc(strlen(curdir) + strlen(path) + 1);
  strcpy(fullpath, curdir);
  strcat(fullpath, path);

  printf("Opening static file: [%s]\n", fullpath);

  FILE *f = fopen(fullpath, "r");
  if (!f)
  {
    perror("Problem with fopen");
    http_404_reply(sockfd);
  }
  else
  {
    char *result = read_file(f);
    http_get_reply(sockfd, result);

    free(result);
  }

  clock_t end_time = clock();

  double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  printf("Total time taken: %f seconds\n", total_time);
}

void *handle_socket_thread(void *sockfd_arg)
{
  int sockfd = *((int *)sockfd_arg);

  printf("Handling socket: %d\n", sockfd);

  char *text = read_text_from_socket(sockfd);
  printf("From socket: %s\n\n", text);

  if (is_get(text))
  {
    char curdir[MAX_CWD];

    if (!getcwd(curdir, MAX_CWD))
    {
      error("Couldn't read curdir");
    }

    char *path = get_path(text);

    if (is_query_request(path))
    {
      run_query(sockfd, curdir, path);
    }
    else
    {
      printf("cwd[%s]\n", curdir);
      printf("path[%s]\n", path);
      output_static_file(sockfd, curdir, path);
    }

    free(path);
  }
  else
  {
    http_404_reply(sockfd);
  }

  free(text);
  close(sockfd);
  free(sockfd_arg);

  return NULL;
}

int create_listening_socket()
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    error("ERROR opening socket");
  }
  int setopt = 1;

  // Reuse the port. Otherwise, on restart, port 8000 is usually still occupied for a bit
  // and we need to start at another port.
  if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&setopt, sizeof(setopt)))
  {
    error("ERROR setting socket options");
  }

  struct sockaddr_in serv_addr;

  uint16_t port = 8000;

  while (1)
  {
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
      port++;
    }
    else
    {
      break;
    }
  }

  if (listen(sockfd, SOMAXCONN) < 0)
    error("Couldn't listen");
  printf("Running on port: %d\n", port);

  return sockfd;
}

int main()
{
  int sockfd = create_listening_socket();

  struct sockaddr_in client_addr;
  int cli_len = sizeof(client_addr);

  struct thread_pool *pool = pool_init(4);

  while (1)
  {
    int newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&cli_len);
    if (newsockfd < 0)
      error("Error on accept");
    printf("New socket: %d\n", newsockfd);

    int *arg = malloc(sizeof(int));
    *arg = newsockfd;
    pool_add_task(pool, handle_socket_thread, arg);
  }

  close(sockfd);

  return 0;
}