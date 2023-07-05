#include "konekt.h"

struct klient *konekt(const char *ip, int port){
    struct klient *client = malloc(sizeof(struct klient)); // make new client to return

    // create tcp socket and connect
    client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    if(connect(client->sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        printf("error - connect\n");
        return 0;
    }


    // ssl stuff
    client->ctx = SSL_CTX_new(TLS_client_method());
    client->ssl = SSL_new(client->ctx);
    SSL_set_fd(client->ssl, client->sockfd);

    if(SSL_connect(client->ssl) != 1){
        printf("error - SSL_connect\n");
        return 0;
    }

    // for valid Sec-WebSocket-Key
    unsigned char key[16];
    RAND_bytes(key, sizeof(key));
    char encoded_key[100];
    EVP_EncodeBlock((unsigned char *)encoded_key, key, 16);

    // handshake
    char request[256];
    sprintf(request,"GET / HTTP/1.1\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Key: %s\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "Upgrade: websocket\r\n"
                    "\r\n", encoded_key);

    printf("sending request\n");

    // send handshake message
    SSL_write(client->ssl, request, strlen(request));


    // read response until "\r\n\r\n" is reached
    char last4[4] = {'a'};
    char t = 'a';
    int i = 0;
    int terminate = 0;

    // TODO: remove debug
    // printf("response:\n");
    while(!terminate){
        SSL_read(client->ssl, &t, 1);
        last4[i%4] = t;
        ++i;
        // printf("%c", t);

        terminate = 1;
        for(int j = 0; j < 4; ++j)
            if(last4[j] != '\r' && last4[j] != '\n') terminate = 0;
    }
    // printf("read %d characters\n", i);

    return client;
}

void Ksend(struct klient *client, uint8_t *data, uint64_t size, uint8_t opcode){
    uint8_t mask_key[4];
    RAND_bytes(mask_key, sizeof(mask_key));

    int payload_size;
    int first_byte;
    if(size < 126){
        payload_size = 0;
        first_byte = size;
    } else if(size < 0x10000){ // if size fits in 2 bytes
        payload_size = 2;
        first_byte = 126;
    } else{
        payload_size = 8;
        first_byte = 127;
    }


    uint8_t msg[size + 6 + payload_size];
    msg[0] = 0x80 | opcode; // final fragment, opcode
    msg[1] = 0x80 | first_byte; // mask=1

    int i = 2 + payload_size;
    memcpy(&msg[2], &size, payload_size); // size
    for(int j = 0; j < 4; ++i, ++j)
        msg[i] = mask_key[j]; // making key
    for(int j = 0; j < size; ++i, ++j)
        msg[i] = data[j]^mask_key[j%4]; // payload data

    SSL_write(client->ssl, msg, size+6);
}

int Krecive(struct klient *client, uint8_t *buffer){
    // read first byte
    char line = 0;
    SSL_read(client->ssl, &line, 1);
    // check if message is valid
    // if(line & 0b01110000) return 0;
    // fin = final fragment
    uint8_t fin = (line & 0x80) >> 7;
    // switch opcode
    uint8_t opcode = line & 0x0f;
    switch(opcode){
        // text data
        case 1:
        // binary data
        case 2:
            SSL_read(client->ssl, &line, 1); // line = payload lenght
            uint64_t lenght = line;

            // extended lenght
            if(line == 126){
                lenght = 0;
                SSL_read(client->ssl, &line, 1);
                lenght = line << 8;
                SSL_read(client->ssl, &line, 1);
                lenght += line;
                lenght &= 0xffff;
            }
            // double extended lenght
            else if(line == 127){
                lenght = 0;
                for(int i = 7; i >= 0; ++i){
                    SSL_read(client->ssl, &line, 1);
                    lenght += line << (8*i);
                }
            }

            // read all
            int contin = 0;
            while(lenght > 0){
                // 4000 bytes per read because its too much otherwise
                int read = SSL_read(client->ssl, &buffer[contin], lenght > 4000 ? 4000 : lenght);
                lenght -= read;
                contin += read;
            }
            return 1;

        case 8:
            printf("connection close\n");
            SSL_read(client->ssl, &line, 1); // line = payload lenght
            printf("size: %d\n", line);
            SSL_read(client->ssl, buffer, line);
            printf("%X %X\n", buffer[0], buffer[1]);
            return 0;

        case 0xA:
            SSL_read(client->ssl, &line, 1); // line = payload lenght
            SSL_read(client->ssl, buffer, line);
            // printf("pong\n");
            return 0;

        case 0:
            return 0;

        default:
            // printf("opcode: %d\n", opcode);
            return 0;
    }
    return 0;
}

// clean
void destroy(struct klient *client){
    SSL_shutdown(client->ssl);
    SSL_free(client->ssl);
    SSL_CTX_free(client->ctx);
    close(client->sockfd);
    free(client);
}

void konvert(void *out, void *in, int size, uint8_t little_endian){
    if(little_endian){
        uint8_t *temp = in;
        for(int i = 0; i < size/2; ++i){
            uint8_t t = temp[i];
            temp[i] = temp[size-i-1];
            temp[size-i-1] = t;
        }
        memcpy(out, temp, size);
        return;
    }

    memcpy(out, in, size);
}
