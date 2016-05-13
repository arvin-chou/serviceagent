#include <stdio.h>
#include <stdlib.h>
#include <json/json.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <json/json.h>

#include "util.h"

char* ReadFile(char *filename)
{
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");

    if (handler)
    {
        // Seek the last byte of the file
        fseek(handler, 0, SEEK_END);
        // Offset from the first to the last byte, or in other words, filesize
        string_size = ftell(handler);
        // go back to the start of the file
        rewind(handler);

        // Allocate a string that can hold it all
        buffer = (char*) malloc(sizeof(char) * (string_size + 1) );

        // Read it all in one operation
        read_size = fread(buffer, sizeof(char), string_size, handler);

        // fread doesn't set it so put a \0 in the last position
        // and buffer is now officially a string
        buffer[string_size] = '\0';

        if (string_size != read_size)
        {
            // Something went wrong, throw away the memory and set
            // the buffer to NULL
            free(buffer);
            buffer = NULL;
        }

        // Always remember to close the file.
        fclose(handler);
    }

    return buffer;
}

void print_json_value(json_object *jobj)
{
    enum json_type type;
    JSON_PRINT("type: ",type);
    type = json_object_get_type(jobj); /*Getting the type of the json object*/
    switch (type) {
        case json_type_boolean: JSON_PRINT("json_type_boolean\n");
                                JSON_PRINT("value: %s\n", json_object_get_boolean(jobj)? "true": "false");
                                break;
        case json_type_double: JSON_PRINT("json_type_double\n");
                               JSON_PRINT("          value: %lf\n", json_object_get_double(jobj));
                               break;
        case json_type_int: JSON_PRINT("json_type_int\n");
                            JSON_PRINT("          value: %d\n", json_object_get_int(jobj));
                            break;
        case json_type_string: JSON_PRINT("json_type_string\n");
                               JSON_PRINT("          value: %s\n", json_object_get_string(jobj));
                               break;
    }

}

void json_parse_array( json_object *jobj, char *key) {
    void json_parse(json_object * jobj); /*Forward Declaration*/
    enum json_type type;

    json_object *jarray = jobj; /*Simply get the array*/
    if(key) {
        json_object_object_get_ex(jobj, key, &jarray); /*Getting the array if it is a key value pair*/
    }

    int arraylen = json_object_array_length(jarray); /*Getting the length of the array*/
    JSON_PRINT("Array Length: %d\n",arraylen);
    int i;
    json_object * jvalue;

    for (i=0; i< arraylen; i++){
        jvalue = json_object_array_get_idx(jarray, i); /*Getting the array element at position i*/
        type = json_object_get_type(jvalue);
        if (type == json_type_array) {
            json_parse_array(jvalue, NULL);
        }
        else if (type != json_type_object) {
            JSON_PRINT("value[%d]: ",i);
            print_json_value(jvalue);
        }
        else {
            json_parse(jvalue);
        }
    }
}



/*Parsing the json object*/
void json_parse(json_object * jobj) {
    enum json_type type;
    struct lh_entry *e;
    char *key; struct json_object *val;
    struct json_object *v; 
    _json_object_object_foreach(e, jobj, key, val) { /*Passing through every array element*/
        JSON_PRINT("type: ",type);
        type = json_object_get_type(val);
        switch (type) {
            case json_type_boolean:
            case json_type_double:
            case json_type_int:
            case json_type_string: print_json_value(val);
                                   break;
            case json_type_object: JSON_PRINT("json_type_object\n");
                                   json_object_object_get_ex(jobj, key, &v);
                                   jobj = v;
                                   json_parse(jobj);
                                   break;
            case json_type_array: JSON_PRINT("type: json_type_array, ");
                                  json_parse_array(jobj, key);
                                  break;
        }
    }
}

void example_create_json()
{
    /*Creating a json object*/
    json_object *jobj = json_object_new_object();

    /*Creating a json string*/
    json_object *jstring = json_object_new_string("Joys of Programming");

    /*Creating a json integer*/
    json_object *jint = json_object_new_int(10);

    /*Creating a json boolean*/
    json_object *jboolean = json_object_new_boolean(1);

    /*Creating a json double*/
    json_object *jdouble = json_object_new_double(2.14);

    /*Creating a json array*/
    json_object *jarray = json_object_new_array();

    /*Creating json strings*/
    json_object *jstring1 = json_object_new_string("c");
    json_object *jstring2 = json_object_new_string("c++");
    json_object *jstring3 = json_object_new_string("php");

    /*Adding the above created json strings to the array*/
    json_object_array_add(jarray,jstring1);
    json_object_array_add(jarray,jstring2);
    json_object_array_add(jarray,jstring3);

    /*Form the json object*/
    /*Each of these is like a key value pair*/
    json_object_object_add(jobj,"Site Name", jstring);
    json_object_object_add(jobj,"Technical blog", jboolean);
    json_object_object_add(jobj,"Average posts per day", jdouble);
    json_object_object_add(jobj,"Number of posts", jint);
    json_object_object_add(jobj,"Categories", jarray);

    printf("Size of JSON object- %lu\n", sizeof(jobj));
    printf("Size of JSON_TO_STRING- %lu,\n %s\n", sizeof(json_object_to_json_string(jobj)), json_object_to_json_string(jobj));

    /*printf("Size of string- %lu\n", sizeof(json_object_to_json_string(jobj)))*/


    //{ "Site Name": "Joys of Programming", "Technical blog": true, "Average posts per day": 2.140000, "Number of posts": 10, "Categories": [ "c", "c++", "php" ] }


    MC_SET_DEBUG(1);
    /*
     * Check that replacing an existing object keeps the key valid,
     * and that it keeps the order the same.
     */
    json_object *my_object = json_object_new_object();
    json_object_object_add(my_object, "foo1", json_object_new_string("bar1"));
    json_object_object_add(my_object, "foo2", json_object_new_string("bar2"));
    json_object_object_add(my_object, "deleteme", json_object_new_string("bar2"));
    json_object_object_add(my_object, "foo3", json_object_new_string("bar3"));
    printf("==== delete-in-loop test starting ====\n");
    int orig_count = 0;
    struct lh_entry *entry;
    char *key; struct json_object *val;
    _json_object_object_foreach(entry, my_object, key, val)
    {
        printf("Key at index %d is [%s]", orig_count, key);
        if (strcmp(key, "deleteme") == 0)
        {
            json_object_object_del(my_object, key);
            printf(" (deleted)\n");
        }
        else
            printf(" (kept)\n");
        orig_count++;
    }
    printf("==== replace-value first loop starting ====\n");
    const char *original_key = NULL;
    orig_count = 0;
    _json_object_object_foreach(entry, my_object, key, val)
    {
        printf("Key at index %d is [%s]\n", orig_count, key);
        orig_count++;
        if (strcmp(key, "foo2") != 0)
            continue;
        printf("replacing value for key [%s]\n", key);
        original_key = key;
        //json_object_object_add(my_object, key, json_object_new_string("zzz"));
        json_object_object_add(my_object, "foo2", json_object_new_string("bar2"));

        //json_object_object_del(my_object, key);
        //json_object_put(val);
        //json_object_object_add(my_object, key, json_object_new_string("zzz"));
    }
    printf("==== second loop starting ====\n");
    int new_count = 0;
    int retval = 0;
    _json_object_object_foreach(entry, my_object, key, val)
    {
        printf("Key at index %d is [%s]\n", new_count, key);
        new_count++;
        if (strcmp(key, "foo2") != 0)
            continue;
        printf("pointer for key [%s] does %smatch\n", key,
                (key == original_key) ? "" : "NOT ");
        if (key != original_key)
            retval = 1;
    }
    if (new_count != orig_count)
    {
        printf("mismatch between original count (%d) and new count (%d)\n",
                orig_count, new_count);
        retval = 1;
    }
    json_object_put( my_object );
    //return retval;



}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


