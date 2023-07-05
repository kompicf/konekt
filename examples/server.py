import ssl
import asyncio
import websockets

# SSL/TLS context setup
ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ssl_context.load_cert_chain('public.pem', 'private.pem')

# Set up an empty set to store connected clients
connected_clients = set()

async def handle_client(websocket, path):
    # Add the new client to the connected clients set
    connected_clients.add(websocket)
    try:
        while True:
            # Wait for incoming messages from the client
            message = await websocket.recv()

            # Broadcast the message to all connected clients
            await broadcast(message)
    except websockets.exceptions.ConnectionClosed:
        # Remove the client from the connected clients set when the connection is closed
        connected_clients.remove(websocket)

async def broadcast(message):
    # Send the message to all connected clients
    for client in connected_clients:
        await client.send(message)

start_server = websockets.serve(handle_client, 'localhost', 8080, ssl=ssl_context)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
