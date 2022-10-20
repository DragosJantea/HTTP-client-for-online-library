#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"


int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    char *coockieul = calloc(100, 1);
    char *tokenul = calloc(500,1);

    // initializam hostul
    char host[100];
    strcpy(host, "34.241.4.235");
    strcat(host, ":8080" );

    // variabile ajutatoare
    char input[100], input1[100], input2[100], id[100], book_title[100];
    char book_author[100], book_genre[100], book_publisher[100], book_page_count[100];
    
    // asteptam inputul de la stdin
    while(1){
        fgets(input, 50, stdin);
        input[strlen(input) - 1] = 0;

        //deschidem conexiunea cu serverul
        sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

        // daca de la stdin s-a introdus exit, inchidem conexiusea si oprim aplicatia
        if(strcmp(input, "exit")==0){
            close(sockfd);
            break;
        }

        // daca de la stdin s-a introdus register
        if(strcmp(input, "register")==0){

            // promptam userul sa introduca si username si password
            printf("username=");
            fgets(input1, 50, stdin);
            input1[strlen(input1) - 1] = 0;
            printf("password=");
            fgets(input2, 50, stdin);
            input2[strlen(input2) - 1] = 0;

            //cream un obiect JSON cu username si password
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            json_object_set_string(root_object, "username", input1);
            json_object_set_string(root_object, "password", input2);

            // convertim obiectul JSON in string pt a fi transmis ca parametru mai usor
            char *serialized_string = json_serialize_to_string_pretty(root_value);
            printf("%s\n", serialized_string);

            // cream mesajul pentru post request, la url-ul corespunzator
            message = compute_post_request(host, "/api/v1/tema/auth/register", "application/json", serialized_string,  NULL, 0, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // aratam daca operatiunea s-a realizat cu succes
            //daca nu, prin controlul erorilor, afisam care a fost cauza esecului
            char *token;
            token = strtok(response, "\r");
            printf("%s", token);


            char second[100];
            while(token!=NULL){
                strcpy(second, token);
                token = strtok(NULL, "\r");
            }

            // printam ultima parte a mesajului, care contine motivul
            printf("%s\n", second);

        }

        //daca primim ca input comanda login
        if(strcmp(input, "login")==0){
            //promptam userul sa introduca username si parola
            printf("username=");
            fgets(input1, 50, stdin);
            input1[strlen(input1) - 1] = 0;

            printf("password=");
            fgets(input2, 50, stdin);
            input2[strlen(input2) - 1] = 0;


            //cream obiectul JSON
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            json_object_set_string(root_object, "username", input1);
            json_object_set_string(root_object, "password", input2);

            char *serialized_string = json_serialize_to_string_pretty(root_value);
            printf("%s\n", serialized_string);

            // facem post request la url-ul pentru login
            message = compute_post_request(host, "/api/v1/tema/auth/login", "application/json", serialized_string,  NULL, 0, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            // facem controlul erorilor
            // daca nu s-a putut conecta, automat nu avem cookie de login
            // si afisam ultima linie din raspunsul primit care contine diagnosticul erorii
            char response_copy[1001];
            strcpy(response_copy, response);

            char *token;
            token = strtok(response, "\r");
            printf("%s\n", token);


            char *pos_init = strstr(response_copy, "Set-Cookie:");

            if(pos_init != NULL){
                char *pos_finn = strstr(pos_init, ";");
                
                strncpy(coockieul, pos_init+12, pos_finn - pos_init - 12 );
            }else{
                char second[100];
                while(token!=NULL){
                    strcpy(second, token);
                    token = strtok(NULL, "\r\n");
                }
                printf("%s\n", second);
            }
            

        }


        //daca primim de la input entry_library, pentru acces in biblioteca
        if(strcmp(input, "enter_library")==0){

            // verficam daca user-ul s-a autentificat in prealabil 
            if(strlen(coockieul)!=0){
                message = compute_get_request(host, "/api/v1/tema/library/access", NULL, &coockieul, 1, NULL);

                send_to_server(sockfd, message);

                response = receive_from_server(sockfd);

                // extragem tokenul autentificarii
                char *pos_prima = strstr(response, "{\"token\":\"");
                char *pos_ultima = strstr(pos_prima+1, "\"}");

                strncpy(tokenul, pos_prima+10, pos_ultima - pos_prima - 10 );
                printf("V-ati autentificat cu succes\n");
            }else{
                printf("Nu sunteti autentificat");
            }
        }

        // daca primim de la stdin comanda get_books
        if(strcmp(input, "get_books")==0){

            //verificam daca userul are acces la biblioteca
            if(strlen(tokenul)!=0){
                message = compute_get_request(host, "/api/v1/tema/library/books",NULL,NULL,0,tokenul );
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                char *rez;
                char second[100];
                rez = strtok(response, "\r\n");
                while(rez!=NULL){
                    strcpy(second, rez);
                    rez = strtok(NULL, "\r");
                }

                // pentru afisarea cartilor ca format JSON
                // pe care dupa-l convertim in string
                JSON_Value *value = json_parse_string(second);
                JSON_Object *object = json_value_get_object(value);

                char *serialized_string = json_serialize_to_string_pretty(value);
                printf("%s\n", serialized_string);
           }else{
               printf("Nu aveti acces la biblioteca");
           }

        }

        // daca primim de la stdin comanda add_books
        if(strcmp(input, "add_book")==0){

            //verificam daca userul are acces la biblioteca
            if(strlen(tokenul)!=0){

                // promptam userul sa introduca parametrii noii carti
                printf("title=");
                fgets(book_title, 50, stdin);
                book_title[strlen(book_title) - 1] = 0;

                printf("author=");
                fgets(book_author, 50, stdin);
                book_author[strlen(book_author) - 1] = 0;

                printf("genre=");
                fgets(book_genre, 50, stdin);
                book_genre[strlen(book_genre) - 1] = 0;

                printf("publisher=");
                fgets(book_publisher, 50, stdin);
                book_publisher[strlen(book_publisher) - 1] = 0;

                printf("page_count=");
                fgets(book_page_count, 50, stdin);
                book_page_count[strlen(book_page_count) - 1] = 0;

                //numarrul de pagini ca Number nu String
                int page_count_int = atoi(book_page_count);

                // cream noua carte
                JSON_Value *root_value1 = json_value_init_object();
                JSON_Object *root_object1 = json_value_get_object(root_value1);
                json_object_set_string(root_object1, "title", book_title);
                json_object_set_string(root_object1, "author", book_title);
                json_object_set_string(root_object1, "genre", book_genre);
                json_object_set_string(root_object1, "publisher", book_publisher);
                json_object_set_number(root_object1, "page_count", page_count_int);

                // convertim cartea la string ca sa o putem trimite ca parametru
                char *serialized_string = json_serialize_to_string_pretty(root_value1);
                printf("%s\n", serialized_string);

                message = compute_post_request(host, "/api/v1/tema/library/books", "application/json", serialized_string,  NULL, 0, tokenul);
                // printf("%s\n", message);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                char *token;
                token = strtok(response, "\r");
                printf("%s", token);
            }else{
               printf("Nu aveti acces la biblioteca");
           }
        }

        // daca primim de la stdin comanda get_book
        if(strcmp(input, "get_book")==0){

            // verificam accesul la biblioteca
            if(strlen(tokenul)!=0){
                printf("id=");
                fgets(id, 50, stdin);
                id[strlen(id) - 1] = 0;

                // de data asta la cale, la url, punem si id-ul cartii
                char *super_url =calloc(100,1);
                strcpy(super_url, "/api/v1/tema/library/books/");
                strcat(super_url, id);

                message = compute_get_request(host, super_url,NULL,NULL,0,tokenul );
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                char response_copy[1000];

                // verific statusul requestului si actionez conform
                strcpy(response_copy, response);


                char *rez;
                char second[100];
                rez = strtok(response, "\r\n");

                char rez2[50];
                strcpy(rez2, rez);

                char* cod = strtok(rez2, " ");
                cod = strtok(NULL, " ");
                
                if(cod[0] != '2'){
                    printf("Ati introdus un id inexistent!\n");
                }else{
                    
                    rez = strtok(response_copy, "\r");
                    while(rez!=NULL){
                        strcpy(second, rez);
                        rez = strtok(NULL, "\r");
                    }
                    
                    printf("%s\n", second);
                }

            }else{
               printf("Nu aveti acces la biblioteca");
            }
        }

        // daca primim de la stdin comanda delete_book
        if(strcmp(input, "delete_book")==0){
            if(strlen(tokenul)!=0){
                // cerem id-ul cartii de sters
                printf("id=");
                fgets(id, 50, stdin);
                id[strlen(id) - 1] = 0;

                // adaugam acel id la cale
                char *super_url =calloc(100,1);
                strcpy(super_url, "/api/v1/tema/library/books/");
                strcat(super_url, id);

                message = compute_delete_request(host, super_url,NULL,NULL,0,tokenul );
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                
                // verific statusul requestului si actionez conform
                char response_copy[1000];
                strcpy(response_copy, response);
                char *rez;
                char second[100];
                rez = strtok(response, "\r\n");

                char rez2[50];
                strcpy(rez2, rez);

                char* cod = strtok(rez2, " ");
                cod = strtok(NULL, " ");
                
                if(cod[0] != '2'){
                    printf("Ati introdus un id inexistent!\n");
                }else{
                    
                    rez = strtok(response_copy, "\r");
                    while(rez!=NULL){
                        strcpy(second, rez);
                        rez = strtok(NULL, "\r");
                    }
                    
                    printf("%s\n", second);
                }


            }else{
               printf("Nu aveti acces la biblioteca");
            }
        }

        // daca primim de la stdin comanda logout
        if(strcmp(input, "logout")==0){
            // verificam daca userul era autentificat
            if(strlen(coockieul)!=0){
                message = compute_get_request(host, "/api/v1/tema/auth/logout",NULL, &coockieul, 1, tokenul);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // stergem cookieul de login si tokenul de login ale sesiunii curente
                memset(coockieul, 0, 100);
                memset(tokenul, 0, 500);

                printf("%s", response);
            }else{
                printf("Nu sunteti autentificat");
            }
        }

        close(sockfd);
        
    }

    

    return 0;
}
