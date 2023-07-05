#ifndef KONEKT_H_274547
#define KONEKT_H_274547

#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define Ksend_text(client, data, size) (Ksend(client, data, size, 1))
#define Ksend_binary(client, data, size) (Ksend(client, data, size, 2))

struct klient{
	SSL *ssl;
	SSL_CTX *ctx;
	int sockfd;
};

struct klient *konekt(const char *ip, int port);
void Ksend(struct klient *client, uint8_t *data, uint64_t size, uint8_t opcode);
int Krecive(struct klient *client, uint8_t *buffer);
void destroy(struct klient *client);
void konvert(void *out, void *in, int size, uint8_t little_endian);

#endif
