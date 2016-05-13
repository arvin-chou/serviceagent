all:
	#gcc a2s.c -ljson -o a
	#gcc s2a.c curl.c -ljson -lcurl -o s
	#gcc curltest.c -ljson -lcurl -o c
	gcc main.c a2s.c s2a.c util.c curl.c -ljson-c -lcurl -lpthread -o main

