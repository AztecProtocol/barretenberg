/**
 * An implementation of a TransportSocket using MessagePorts.
 */
export class MessagePortSocket {
    /**
     * Create a MessagePortSocket.
     * @param port - MessagePort object to wrap.
     */
    constructor(port) {
        this.port = port;
    }
    /**
     * Send a message over our message port.
     * @param msg - The message.
     * @param transfer - Objects to transfer ownership of.
     */
    send(msg, transfer = []) {
        this.port.postMessage(msg, transfer);
        return Promise.resolve();
    }
    /**
     * Add a message handler.
     * @param cb - The handler.
     */
    registerHandler(cb) {
        this.port.onmessage = event => cb(event.data);
    }
    /**
     * Close this message port.
     */
    close() {
        void this.send(undefined);
        this.port.onmessage = null;
        this.port.close();
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoibWVzc2FnZV9wb3J0X3NvY2tldC5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uLy4uL3NyYy93YXNtL3RyYW5zcG9ydC9icm93c2VyL21lc3NhZ2VfcG9ydF9zb2NrZXQudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBRUE7O0dBRUc7QUFDSCxNQUFNLE9BQU8saUJBQWlCO0lBQzVCOzs7T0FHRztJQUNILFlBQW9CLElBQWlCO1FBQWpCLFNBQUksR0FBSixJQUFJLENBQWE7SUFBRyxDQUFDO0lBRXpDOzs7O09BSUc7SUFDSCxJQUFJLENBQUMsR0FBUSxFQUFFLFdBQTJCLEVBQUU7UUFDMUMsSUFBSSxDQUFDLElBQUksQ0FBQyxXQUFXLENBQUMsR0FBRyxFQUFFLFFBQVEsQ0FBQyxDQUFDO1FBQ3JDLE9BQU8sT0FBTyxDQUFDLE9BQU8sRUFBRSxDQUFDO0lBQzNCLENBQUM7SUFFRDs7O09BR0c7SUFDSCxlQUFlLENBQUMsRUFBcUI7UUFDbkMsSUFBSSxDQUFDLElBQUksQ0FBQyxTQUFTLEdBQUcsS0FBSyxDQUFDLEVBQUUsQ0FBQyxFQUFFLENBQUMsS0FBSyxDQUFDLElBQUksQ0FBQyxDQUFDO0lBQ2hELENBQUM7SUFFRDs7T0FFRztJQUNILEtBQUs7UUFDSCxLQUFLLElBQUksQ0FBQyxJQUFJLENBQUMsU0FBUyxDQUFDLENBQUM7UUFDMUIsSUFBSSxDQUFDLElBQUksQ0FBQyxTQUFTLEdBQUcsSUFBSSxDQUFDO1FBQzNCLElBQUksQ0FBQyxJQUFJLENBQUMsS0FBSyxFQUFFLENBQUM7SUFDcEIsQ0FBQztDQUNGIn0=