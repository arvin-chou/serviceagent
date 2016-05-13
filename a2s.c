#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json/json.h>
#include "common.h"
#include "util.h"

extern struct config config;
char buff[MAX_SIZE];

void setupHeartBeatFreq(json_object* jobj)
{
    char *key; struct json_object *val;
    struct lh_entry *entry;

    _json_object_object_foreach(entry, jobj, key, val)
    {
        if (!strcmp(key, "argument"))
        {
            //jobj = json_object_object_get(val, "hb_freq"); //<! hb_freq
            //int sleep = atoi(json_object_get_string(jobj));
            struct json_object *v; 
            int sleep;
            json_object_object_get_ex(val, "hb_freq", &v); //<! hb_freq
            sleep = atoi(json_object_get_string(v));
            printf("setup up heartbeat to: %d\n", sleep);
            config.time_sleep = sleep;

            //sleep
            break;
        }
    }
}

void* a2s(char* json_path, const char* json_str)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        printf("Error opening socket\n");
    server = gethostbyname(SERVER);
    if (server == NULL) {
        printf("Error: no such host\n");
        return;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        printf("Error connecting\n");

    char *buf;
    if (json_path != NULL)
        buf = ReadFile(json_path);
    else
        buf = (char*) json_str;

    if (buf)
    {
        struct lh_entry *entry;
        json_object * jobj = json_tokener_parse(buf);
        char *key; struct json_object *val;
        _json_object_object_foreach(entry, jobj, key, val)
        {
            if (!strcmp(key, "api_name"))
            {
                if (!strcmp(json_object_get_string(val), "set_VMCfgs"))
                {
                    //sleep
                    setupHeartBeatFreq(jobj);
                    break;
                }
            }
        }

        n = write(sockfd, buf, strlen(buf));
        if (n < 0)
            printf("Error writing to socket");

        bzero(buff, MAX_SIZE);
        n = read(sockfd, buff, MAX_SIZE);
        if (n < 0)
            printf("Error reading from socket");
        printf("%s\n", buff);

        free(buf);
        json_object_put(jobj);
    }
    else
    {
        printf("not found path %s\n", json_path);
    }



    close(sockfd);

    return ;
}
