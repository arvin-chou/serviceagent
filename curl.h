#ifndef __CURL_H__
#define __CURL_H__
int curl(const char* url, const char* body, void (*success)(char* payload));
#endif
