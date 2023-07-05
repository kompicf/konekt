# konekt
(very bad) websocket client library for C (works only with ssl)

# functions
```c
// konekt(ip, port) - returns struct klient
// example:
struct klient *client = konekt("127.0.0.1", 8080);

// Ksend_binary(client, data, size);
// example:
uint8_t data[4] = {1, 2, 3, 4};
Ksend_binary(client, data, 4);

// Krecive(client, buffer);
// example:
// reccommended to use in separate thread since its blocking
for(;;){
  uint8_t buffer[6000];
  if(Krecive(client, buffer){
    // do stuff with buffer

    // konvert(out, in, size, little_endian);
    // example:
    int32_t some32bitLittleEndianData;
    konvert(&some32bitLittleEndianData, buffer, 4, 1);
    int64_t some64bitData;
    konvert(&some64bitData, &buffer[4], 8, 0);
  }
}
// destroy(client);
// example:
destroy(client);
```
# compile
```bash
gcc FileWhereYouUseThis.c konekt.c -o ExeFile -lcrypto -lssl
```
you will probably need -lpthread if you use seperate thread for Krecive

# real example
client.c is example chat client and server.py is example chat server
to compile client.c use
```bash
gcc client.c konekt.c -o wsclient -lcrypto -lssl -lpthread
```
to use server.py
```bash
# install websocket thing
pip install websockets
# generate ssl stuff
cd examples
openssl genrsa 2048 > private.pem
openssl req -x509 -days 1000 -new -key private.pem -out public.pem
# start server
python3 server.py
```

# TODO
- add support for dragmented messages
- add support for sending messages longer than 125 bytes (done but not tested)
- ping pong
- add support for all opcodes
- clean includes
