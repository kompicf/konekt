#include "../konekt.h"
#include <pthread.h>

struct klient *client;

void *thread_stuff();

int main(void){
    client = konekt("127.0.0.1", 8080);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_stuff, NULL);

    char buffer[0x1000] = {0};
    scanf("%s", buffer);
    while(buffer[0] != 'x' || buffer[1] != '\0'){
        Ksend_text(client, buffer, strlen(buffer)+1);
        scanf("%s", buffer);
    }
    destroy(client);
    return 0;
}

void *thread_stuff(){
    char buffer[0x1000];
    for(;;){
        Krecive(client, buffer);
        printf("%s\n", buffer);
    }
    return 0;
}
