import { MessagePortSocket } from './message_port_socket.js';
/**
 * Connector implementation which wraps a SharedWorker.
 */
export class SharedWorkerConnector {
    /**
     * Create a SharedWorkerConnector.
     * @param worker - A shared worker.
     */
    constructor(worker) {
        this.worker = worker;
    }
    /**
     * Create a Socket implementation with our mesage port.
     * @returns The socket.
     */
    createSocket() {
        return Promise.resolve(new MessagePortSocket(this.worker.port));
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoic2hhcmVkX3dvcmtlcl9jb25uZWN0b3IuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi8uLi9zcmMvd2FzbS90cmFuc3BvcnQvYnJvd3Nlci9zaGFyZWRfd29ya2VyX2Nvbm5lY3Rvci50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFDQSxPQUFPLEVBQUUsaUJBQWlCLEVBQUUsTUFBTSwwQkFBMEIsQ0FBQztBQUU3RDs7R0FFRztBQUNILE1BQU0sT0FBTyxxQkFBcUI7SUFDaEM7OztPQUdHO0lBQ0gsWUFBb0IsTUFBb0I7UUFBcEIsV0FBTSxHQUFOLE1BQU0sQ0FBYztJQUFHLENBQUM7SUFFNUM7OztPQUdHO0lBQ0gsWUFBWTtRQUNWLE9BQU8sT0FBTyxDQUFDLE9BQU8sQ0FBQyxJQUFJLGlCQUFpQixDQUFDLElBQUksQ0FBQyxNQUFNLENBQUMsSUFBSSxDQUFDLENBQUMsQ0FBQztJQUNsRSxDQUFDO0NBQ0YifQ==