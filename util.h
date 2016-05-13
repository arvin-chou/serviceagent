#ifndef __UTIL_H__
#define __UTIL_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <json/json.h>

//#define JSON_DBG_EN //<! uncomment means dump json
#ifdef JSON_DBG_EN
#define JSON_PRINT(fmt, args...) printf(fmt, ##args)
#else
#define JSON_PRINT(fmr, args...)
#endif

#define _json_object_object_foreach(e, obj,key,val) \
for(e = json_object_get_object(obj)->head; ({ if(e) { key = (char*)e->k; val = (struct json_object*)e->v; } ; e; }); e = e->next )

#define MAX_SIZE (500)
struct config {
    int time_sleep;
    bool is_register_complete;
    bool is_register;
};

struct json_queue {
    int cnt;
    json_object* data;
    struct json_queue* next;
};

char* ReadFile(char *filename);
void *get_in_addr(struct sockaddr *sa);
#endif
