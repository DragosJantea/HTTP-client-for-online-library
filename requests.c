#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *tokenul)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // adaugam tipul requestului si url-ul
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // adaugam hostul
    memset(line, 0 ,LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // adaugam coockie-uri
    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        for(int i=0; i<cookies_count-1;i++){
            strcat(line, cookies[i]);
            strcat(line, ";");
        }

        strcat(line, cookies[cookies_count-1]);
        compute_message(message, line);
    }

    // adaugam autorizatia
    if(tokenul!=NULL){
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", tokenul);
        compute_message(message, line);
    }
    // new line de final
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char *body_data, char **cookies, int cookies_count,char *tokenul)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    // char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // post request
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // adaugam hostul
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if(tokenul!=NULL){
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", tokenul);
        compute_message(message, line);
    }

    sprintf(line, "Content-Type: %s\r\nContent-length: %d", content_type, strlen(body_data));
    compute_message(message, line);


    // adaugam cookie-uri
    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        for(int i=0; i<cookies_count-1;i++){
            strcat(line, cookies[i]);
            strcat(line, ";");
        }

        strcat(line, cookies[cookies_count-1]);
        compute_message(message, line);
    }
    // linie noua la final 
    compute_message(message, "");

    compute_message(message, body_data);

    free(line);
    return message;
}


// request pt stergerea unei carti, la fel ca get
char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *tokenul)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    memset(line, 0 ,LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        for(int i=0; i<cookies_count-1;i++){
            strcat(line, cookies[i]);
            strcat(line, ";");
        }

        strcat(line, cookies[cookies_count-1]);
        compute_message(message, line);
    }

    if(tokenul!=NULL){
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", tokenul);
        compute_message(message, line);
    }
    compute_message(message, "");
    return message;
}
