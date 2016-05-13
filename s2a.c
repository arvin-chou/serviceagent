#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#include <json/json.h>

#include "curl.h"
#include "common.h"
#include "util.h"

#define PATH_SZ (50)
#define URL_SZ (100)
char path[PATH_SZ];
char url[URL_SZ];
char buff[MAX_SIZE];
extern struct config config;
extern struct json_queue json_queue;
struct config* c;

void json_queue_add(json_object* jobj)
{
    struct json_queue* e = malloc(sizeof(struct json_queue));
    e->next = NULL;
    e->data = jobj;

    json_queue.next = e;
    json_queue.cnt++;
}

json_object* json_queue_pop(void)
{
    struct json_queue* e = json_queue.next;
    json_object* jobj = e->data;
    json_queue.next = e->next;
    json_queue.cnt--;
    free(e);
    return jobj;
}

void success(char* payload)
{
    c = &config;
    if (c->is_register==false ||
            c->is_register_complete==false ||
            !strcmp(path, PATH_URI_HEARTBEAT))
    {
        json_object* jobj;
        //jobj = json_tokener_parse(payload);
        //printf("Parsed JSON: %s\n", json_object_to_json_string(jobj));
        // TODO: if has json , recursive call and send to non
#if 0
        enum json_type type;

        json_object *jarray;
        int arraylen;
        int i;

        type = json_object_get_type(jobj);
        switch (type) {
            case json_type_object:
                json_queue_add(jobj);
                break;
            case json_type_array:
                jarray = jobj; /*Simply get the array*/
                arraylen = json_object_array_length(jarray); /*Getting the length of the array*/
                json_object * jvalue;

                for (i=0; i< arraylen; i++)
                {
                    jvalue = json_object_array_get_idx(jarray, i); /*Getting the array element at position i*/
                    type = json_object_get_type(jvalue);
                    if (type == json_type_array) {
                        printf("impossible, it must like {a:1}\n");
                    }
                    else if (type != json_type_object) {
                        printf("impossible, it must like {a:1}\n");
                    }
                    else {
                        json_queue_add(jobj);
                    }
                }
                break;
            case json_type_boolean:
            case json_type_double:
            case json_type_int:
            case json_type_string:
            default:
                printf("unknown type(%d)\n", type);
                break;
        }
#endif
    }
    //printf("CURL Returned: \n%s\n", payload);
}

void do_stuff(int newsockfd)
{
    int n;
    c = &config;

    bzero(buff, MAX_SIZE);

    n = read(newsockfd, buff, MAX_SIZE-1);

    if (n < 0)
        printf("ERROR reading from socket\n");

    printf("service: Here is the message: %s\n", buff);


    if (c->is_register == false)
        c->is_register = true;

    struct lh_entry *entry;
    json_object * jobj = json_tokener_parse(buff);
    char *key; struct json_object *val;
    _json_object_object_foreach(entry, jobj, key, val)
    {
        if (!strcmp(key, "api_name"))
        {
            if (!strcmp(json_object_get_string(val), "get_VMStatus"))
            {
                //setHeartBeatPath
                sprintf(path, PATH_URI_HEARTBEAT);
            }
            else if (!strcmp(json_object_get_string(val), "get_VMInfo"))
            {
                if (c->is_register_complete == false)
                    json_object_object_add(jobj, "api_name",
                            json_object_new_string("register"));
            }
        }
    }

    sprintf(url, "%s%s", URL, path);
    printf("url: %s, post body %s\n", url, json_object_to_json_string(jobj));
    curl(url, buff, success);
    //json_parse(jobj);

#define CONST_RESP "service: I got your message\n"
    n = write(newsockfd, CONST_RESP, strlen(CONST_RESP));

    if (n < 0)
        printf("ERROR writing to socket\ns");

    sprintf(path, PATH_URI_DEFAULT);
    json_object_put(jobj);
}

void* s2a(void* data)
{
    int sockfd, newsockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        printf("ERROR opening socket\n");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0)
        printf("ERROR on binding\n");

    listen(sockfd, PORT);
    clilen = sizeof(cli_addr);

    while ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) >= 0)
    {
        if (newsockfd < 0)
            printf("ERROR on accept\n");

        do_stuff(newsockfd);

        close(newsockfd);
    }

    return;
}
