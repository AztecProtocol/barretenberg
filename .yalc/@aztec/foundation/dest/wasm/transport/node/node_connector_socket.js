/**
 * A socket implementation using a Node worker.
 */
export class NodeConnectorSocket {
    constructor(worker) {
        this.worker = worker;
    }
    /**
     * Send a message.
     * @param msg - The message.
     * @param transfer - Objects to transfer ownership of.
     * @returns A void promise.
     */
    send(msg, transfer = []) {
        this.worker.postMessage(msg, transfer);
        return Promise.resolve();
    }
    /**
     * Register a message handler.
     * @param cb - The handler function.
     */
    registerHandler(cb) {
        this.worker.on('message', cb);
    }
    /**
     * Remove all listeners from our worker.
     */
    close() {
        void this.send(undefined);
        this.worker.removeAllListeners();
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoibm9kZV9jb25uZWN0b3Jfc29ja2V0LmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vLi4vc3JjL3dhc20vdHJhbnNwb3J0L25vZGUvbm9kZV9jb25uZWN0b3Jfc29ja2V0LnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUdBOztHQUVHO0FBQ0gsTUFBTSxPQUFPLG1CQUFtQjtJQUM5QixZQUFvQixNQUFjO1FBQWQsV0FBTSxHQUFOLE1BQU0sQ0FBUTtJQUFHLENBQUM7SUFFdEM7Ozs7O09BS0c7SUFDSCxJQUFJLENBQUMsR0FBUSxFQUFFLFdBQTJCLEVBQUU7UUFDMUMsSUFBSSxDQUFDLE1BQU0sQ0FBQyxXQUFXLENBQUMsR0FBRyxFQUFFLFFBQThCLENBQUMsQ0FBQztRQUM3RCxPQUFPLE9BQU8sQ0FBQyxPQUFPLEVBQUUsQ0FBQztJQUMzQixDQUFDO0lBRUQ7OztPQUdHO0lBQ0gsZUFBZSxDQUFDLEVBQXFCO1FBQ25DLElBQUksQ0FBQyxNQUFNLENBQUMsRUFBRSxDQUFDLFNBQVMsRUFBRSxFQUFFLENBQUMsQ0FBQztJQUNoQyxDQUFDO0lBRUQ7O09BRUc7SUFDSCxLQUFLO1FBQ0gsS0FBSyxJQUFJLENBQUMsSUFBSSxDQUFDLFNBQVMsQ0FBQyxDQUFDO1FBQzFCLElBQUksQ0FBQyxNQUFNLENBQUMsa0JBQWtCLEVBQUUsQ0FBQztJQUNuQyxDQUFDO0NBQ0YifQ==