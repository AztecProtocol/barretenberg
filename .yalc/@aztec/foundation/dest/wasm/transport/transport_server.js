import { isTransferDescriptor } from './interface/transferable.js';
/**
 * Keeps track of clients, providing a broadcast, and request/response api with multiplexing.
 */
export class TransportServer {
    constructor(listener, msgHandlerFn) {
        this.listener = listener;
        this.msgHandlerFn = msgHandlerFn;
        this.sockets = [];
    }
    /**
     * Start accepting new connections.
     */
    start() {
        this.listener.on('new_socket', client => this.handleNewSocket(client));
        this.listener.open();
    }
    /**
     * Stops accepting new connections. It doesn't close existing sockets.
     * It's expected the clients will gracefully complete by closing their end, sending an `undefined` message.
     */
    stop() {
        this.listener.close();
    }
    /**
     * Broadcast a message.
     * @param msg - The message.
     */
    async broadcast(msg) {
        await Promise.all(this.sockets.map(s => s.send({ payload: msg })));
    }
    /**
     * New socket registration.
     * @param socket - The socket to register.
     */
    handleNewSocket(socket) {
        socket.registerHandler(async (msg) => {
            if (msg === undefined) {
                // Client socket has closed. Remove it from the list of sockets. Call close on it for any cleanup.
                const socketIndex = this.sockets.findIndex(s => s === socket);
                const [closingSocket] = this.sockets.splice(socketIndex, 1);
                closingSocket.close();
                return;
            }
            return await this.handleSocketMessage(socket, msg);
        });
        this.sockets.push(socket);
    }
    /**
     * Detect the 'transferables' argument to our socket from our message
     * handler return type.
     * @param data - The return object.
     * @returns - The data and the.
     */
    getPayloadAndTransfers(data) {
        if (isTransferDescriptor(data)) {
            // We treat PayloadWithTransfers specially so that we're able to
            // attach transferables while keeping a simple return-type based usage
            return [data.send, data.transferables];
        }
        if (data instanceof Uint8Array) {
            // We may want to devise a better solution to this. We maybe given a view over a non cloneable/transferrable
            // ArrayBuffer (such as a view over wasm memory). In this case we want to take a copy, and then transfer it.
            const respPayload = data instanceof Uint8Array && ArrayBuffer.isView(data) ? new Uint8Array(data) : data;
            const transferables = data instanceof Uint8Array ? [respPayload.buffer] : [];
            return [respPayload, transferables];
        }
        return [data, []];
    }
    /**
     * Handles a socket message from a listener.
     * @param socket - The socket.
     * @param requestMessage - The message to handle.
     * @returns The socket response.
     */
    async handleSocketMessage(socket, { msgId, payload }) {
        try {
            const data = await this.msgHandlerFn(payload);
            const [respPayload, transferables] = this.getPayloadAndTransfers(data);
            const rep = { msgId, payload: respPayload };
            await socket.send(rep, transferables);
        }
        catch (err) {
            const rep = { msgId, error: err.stack };
            await socket.send(rep);
        }
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoidHJhbnNwb3J0X3NlcnZlci5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uL3NyYy93YXNtL3RyYW5zcG9ydC90cmFuc3BvcnRfc2VydmVyLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUdBLE9BQU8sRUFBRSxvQkFBb0IsRUFBRSxNQUFNLDZCQUE2QixDQUFDO0FBRW5FOztHQUVHO0FBQ0gsTUFBTSxPQUFPLGVBQWU7SUFHMUIsWUFBb0IsUUFBa0IsRUFBVSxZQUE0QztRQUF4RSxhQUFRLEdBQVIsUUFBUSxDQUFVO1FBQVUsaUJBQVksR0FBWixZQUFZLENBQWdDO1FBRnBGLFlBQU8sR0FBYSxFQUFFLENBQUM7SUFFZ0UsQ0FBQztJQUVoRzs7T0FFRztJQUNILEtBQUs7UUFDSCxJQUFJLENBQUMsUUFBUSxDQUFDLEVBQUUsQ0FBQyxZQUFZLEVBQUUsTUFBTSxDQUFDLEVBQUUsQ0FBQyxJQUFJLENBQUMsZUFBZSxDQUFDLE1BQU0sQ0FBQyxDQUFDLENBQUM7UUFDdkUsSUFBSSxDQUFDLFFBQVEsQ0FBQyxJQUFJLEVBQUUsQ0FBQztJQUN2QixDQUFDO0lBRUQ7OztPQUdHO0lBQ0gsSUFBSTtRQUNGLElBQUksQ0FBQyxRQUFRLENBQUMsS0FBSyxFQUFFLENBQUM7SUFDeEIsQ0FBQztJQUVEOzs7T0FHRztJQUNILEtBQUssQ0FBQyxTQUFTLENBQUMsR0FBWTtRQUMxQixNQUFNLE9BQU8sQ0FBQyxHQUFHLENBQUMsSUFBSSxDQUFDLE9BQU8sQ0FBQyxHQUFHLENBQUMsQ0FBQyxDQUFDLEVBQUUsQ0FBQyxDQUFDLENBQUMsSUFBSSxDQUFDLEVBQUUsT0FBTyxFQUFFLEdBQUcsRUFBRSxDQUFDLENBQUMsQ0FBQyxDQUFDO0lBQ3JFLENBQUM7SUFFRDs7O09BR0c7SUFDSyxlQUFlLENBQUMsTUFBYztRQUNwQyxNQUFNLENBQUMsZUFBZSxDQUFDLEtBQUssRUFBQyxHQUFHLEVBQUMsRUFBRTtZQUNqQyxJQUFJLEdBQUcsS0FBSyxTQUFTLEVBQUU7Z0JBQ3JCLGtHQUFrRztnQkFDbEcsTUFBTSxXQUFXLEdBQUcsSUFBSSxDQUFDLE9BQU8sQ0FBQyxTQUFTLENBQUMsQ0FBQyxDQUFDLEVBQUUsQ0FBQyxDQUFDLEtBQUssTUFBTSxDQUFDLENBQUM7Z0JBQzlELE1BQU0sQ0FBQyxhQUFhLENBQUMsR0FBRyxJQUFJLENBQUMsT0FBTyxDQUFDLE1BQU0sQ0FBQyxXQUFXLEVBQUUsQ0FBQyxDQUFDLENBQUM7Z0JBQzVELGFBQWEsQ0FBQyxLQUFLLEVBQUUsQ0FBQztnQkFDdEIsT0FBTzthQUNSO1lBQ0QsT0FBTyxNQUFNLElBQUksQ0FBQyxtQkFBbUIsQ0FBQyxNQUFNLEVBQUUsR0FBRyxDQUFDLENBQUM7UUFDckQsQ0FBQyxDQUFDLENBQUM7UUFDSCxJQUFJLENBQUMsT0FBTyxDQUFDLElBQUksQ0FBQyxNQUFNLENBQUMsQ0FBQztJQUM1QixDQUFDO0lBRUQ7Ozs7O09BS0c7SUFDSyxzQkFBc0IsQ0FBQyxJQUFTO1FBQ3RDLElBQUksb0JBQW9CLENBQUMsSUFBSSxDQUFDLEVBQUU7WUFDOUIsZ0VBQWdFO1lBQ2hFLHNFQUFzRTtZQUN0RSxPQUFPLENBQUMsSUFBSSxDQUFDLElBQUksRUFBRSxJQUFJLENBQUMsYUFBYSxDQUFDLENBQUM7U0FDeEM7UUFDRCxJQUFJLElBQUksWUFBWSxVQUFVLEVBQUU7WUFDOUIsNEdBQTRHO1lBQzVHLDRHQUE0RztZQUM1RyxNQUFNLFdBQVcsR0FBRyxJQUFJLFlBQVksVUFBVSxJQUFJLFdBQVcsQ0FBQyxNQUFNLENBQUMsSUFBSSxDQUFDLENBQUMsQ0FBQyxDQUFDLElBQUksVUFBVSxDQUFDLElBQUksQ0FBQyxDQUFDLENBQUMsQ0FBQyxJQUFJLENBQUM7WUFDekcsTUFBTSxhQUFhLEdBQUcsSUFBSSxZQUFZLFVBQVUsQ0FBQyxDQUFDLENBQUMsQ0FBQyxXQUFXLENBQUMsTUFBTSxDQUFDLENBQUMsQ0FBQyxDQUFDLEVBQUUsQ0FBQztZQUM3RSxPQUFPLENBQUMsV0FBVyxFQUFFLGFBQWEsQ0FBQyxDQUFDO1NBQ3JDO1FBQ0QsT0FBTyxDQUFDLElBQUksRUFBRSxFQUFFLENBQUMsQ0FBQztJQUNwQixDQUFDO0lBQ0Q7Ozs7O09BS0c7SUFDSyxLQUFLLENBQUMsbUJBQW1CLENBQUMsTUFBYyxFQUFFLEVBQUUsS0FBSyxFQUFFLE9BQU8sRUFBMkI7UUFDM0YsSUFBSTtZQUNGLE1BQU0sSUFBSSxHQUFHLE1BQU0sSUFBSSxDQUFDLFlBQVksQ0FBQyxPQUFPLENBQUMsQ0FBQztZQUU5QyxNQUFNLENBQUMsV0FBVyxFQUFFLGFBQWEsQ0FBQyxHQUFHLElBQUksQ0FBQyxzQkFBc0IsQ0FBQyxJQUFJLENBQUMsQ0FBQztZQUN2RSxNQUFNLEdBQUcsR0FBNkIsRUFBRSxLQUFLLEVBQUUsT0FBTyxFQUFFLFdBQVcsRUFBRSxDQUFDO1lBRXRFLE1BQU0sTUFBTSxDQUFDLElBQUksQ0FBQyxHQUFHLEVBQUUsYUFBYSxDQUFDLENBQUM7U0FDdkM7UUFBQyxPQUFPLEdBQVEsRUFBRTtZQUNqQixNQUFNLEdBQUcsR0FBNkIsRUFBRSxLQUFLLEVBQUUsS0FBSyxFQUFFLEdBQUcsQ0FBQyxLQUFLLEVBQUUsQ0FBQztZQUNsRSxNQUFNLE1BQU0sQ0FBQyxJQUFJLENBQUMsR0FBRyxDQUFDLENBQUM7U0FDeEI7SUFDSCxDQUFDO0NBQ0YifQ==