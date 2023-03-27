/**
 * An implementation of a TransportSocket using MessagePorts.
 */
export class NodeListenerSocket {
    constructor(port) {
        this.port = port;
    }
    /**
     * Send a message over this port.
     * @param msg - The message.
     * @param transfer - Transferable objects.
     * @returns A void promise.
     */
    send(msg, transfer = []) {
        this.port.postMessage(msg, transfer);
        return Promise.resolve();
    }
    /**
     * Add a handler to this port.
     * @param cb - The handler function.
     */
    registerHandler(cb) {
        this.port.on('message', cb);
    }
    /**
     * Close this socket.
     */
    close() {
        void this.send(undefined);
        this.port.removeAllListeners();
        this.port.close();
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoibm9kZV9saXN0ZW5lcl9zb2NrZXQuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi8uLi9zcmMvd2FzbS90cmFuc3BvcnQvbm9kZS9ub2RlX2xpc3RlbmVyX3NvY2tldC50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFHQTs7R0FFRztBQUNILE1BQU0sT0FBTyxrQkFBa0I7SUFDN0IsWUFBb0IsSUFBaUI7UUFBakIsU0FBSSxHQUFKLElBQUksQ0FBYTtJQUFHLENBQUM7SUFFekM7Ozs7O09BS0c7SUFDSCxJQUFJLENBQUMsR0FBUSxFQUFFLFdBQTJCLEVBQUU7UUFDMUMsSUFBSSxDQUFDLElBQUksQ0FBQyxXQUFXLENBQUMsR0FBRyxFQUFFLFFBQThCLENBQUMsQ0FBQztRQUMzRCxPQUFPLE9BQU8sQ0FBQyxPQUFPLEVBQUUsQ0FBQztJQUMzQixDQUFDO0lBRUQ7OztPQUdHO0lBQ0gsZUFBZSxDQUFDLEVBQXFCO1FBQ25DLElBQUksQ0FBQyxJQUFJLENBQUMsRUFBRSxDQUFDLFNBQVMsRUFBRSxFQUFFLENBQUMsQ0FBQztJQUM5QixDQUFDO0lBRUQ7O09BRUc7SUFDSCxLQUFLO1FBQ0gsS0FBSyxJQUFJLENBQUMsSUFBSSxDQUFDLFNBQVMsQ0FBQyxDQUFDO1FBQzFCLElBQUksQ0FBQyxJQUFJLENBQUMsa0JBQWtCLEVBQUUsQ0FBQztRQUMvQixJQUFJLENBQUMsSUFBSSxDQUFDLEtBQUssRUFBRSxDQUFDO0lBQ3BCLENBQUM7Q0FDRiJ9