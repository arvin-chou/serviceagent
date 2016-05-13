#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>


#include "common.h"
#include "util.h"
#include "s2a.h"
#include "a2s.h"

struct config config;
struct json_queue json_queue;


void* job(void* data)
{
    struct config* c = &config;
    while(1)
    {
        while (c->is_register == false || c->is_register_complete == false)
        {
            a2s(PATH_GETVMINFO, NULL);
            if (json_queue.cnt > 0)
            {
                json_object* jobj = json_queue_pop();
                a2s(NULL, json_object_to_json_string(jobj));
                json_object_put(jobj);
                c->is_register_complete = true;
            }

            printf("sleep 10s for test\n");
            sleep(10);
        }

        if (json_queue.cnt > 0)
        {
            //TODO: schedule re-add queue
            json_object* jobj = json_queue_pop();
            a2s(NULL, json_object_to_json_string(jobj));
            json_object_put(jobj);
        }

        a2s(PATH_GETVMSTATUS, NULL);

        printf("sleep %ds\n", c->time_sleep);
        sleep(c->time_sleep);
    }

    pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
   int rc;
   pthread_t threads;
   memset(&config, 0x00, sizeof(struct config));
   config.time_sleep = 10; //<! default heartbeat

   memset(&json_queue, 0x00, sizeof(struct json_queue));

   rc = pthread_create(&threads, NULL, job, NULL);
   s2a(NULL); // infinity

   //free it
   while(json_queue.cnt > 0)
   {
       json_object* jobj = json_queue_pop();
       json_object_put(jobj);
   }

   pthread_exit(NULL);
   return 0;
}
