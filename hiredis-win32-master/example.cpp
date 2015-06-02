#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hiredis.h"

#ifdef HIREDIS_WIN
#define snprintf sprintf_s
#endif

int main(void) {
    unsigned int j;
    redisContext *c;
    redisReply *reply;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout((char*)"192.168.1.211", 6379, timeout);
    if (c->err) {
        printf("Connection error: %s\n", c->errstr);
        exit(1);
    }

    /* PING server */
    reply = (redisReply*)redisCommand(c,"scard 4503_INSERT_LOG_SIGNVERIFY");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

	reply = (redisReply*)redisCommand(c,"scard 4503_INSERT_LOG_SIGNVERIFY");
	printf("PING: %s\n", reply->str);
	freeReplyObject(reply);

    /* Set a key */
    reply = (redisReply*)redisCommand(c,"SET %s %s", "foo", "hello world");
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);

    /* Set a key using binary safe API */
    reply = (redisReply*)redisCommand(c,"SET %b %b", "bar", 3, "hello", 5);
    printf("SET (binary API): %s\n", reply->str);
    freeReplyObject(reply);

    /* Try a GET and two INCR */
    reply = (redisReply*)redisCommand(c,"scard 4503_INSERT_LOG_SIGNVERIFY");
    printf("GET foo: %s\n", reply->str);
    freeReplyObject(reply);

    reply = (redisReply*)redisCommand(c,"INCR counter");
    printf("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);
    /* again ... */
    reply = (redisReply*)redisCommand(c,"INCR counter");
    printf("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);

    /* Create a list of numbers, from 0 to 9 */
    reply = (redisReply*)redisCommand(c,"DEL mylist");
    freeReplyObject(reply);
    for (j = 0; j < 10; j++) {
        char buf[64];

        snprintf(buf,64,"%d",j);
        reply = (redisReply*)redisCommand(c,"LPUSH mylist element-%s", buf);
        freeReplyObject(reply);
    }

    /* Let's check what we have inside the list */
    reply = (redisReply*)redisCommand(c,"LRANGE mylist 0 -1");
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < reply->elements; j++) {
            printf("%u) %s\n", j, reply->element[j]->str);
        }
    }
    freeReplyObject(reply);

    return 0;
}
