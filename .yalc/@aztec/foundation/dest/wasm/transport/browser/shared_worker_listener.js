import EventEmitter from 'events';
import { MessagePortSocket } from './message_port_socket.js';
/**
 * Listens for connections to a shared worker.
 */
export class SharedWorkerListener extends EventEmitter {
    /**
     *
     * @param worker
     */
    constructor(worker) {
        super();
        this.worker = worker;
        /**
         *
         * @param event
         */
        this.handleMessageEvent = (event) => {
            const [port] = event.ports;
            if (!port) {
                return;
            }
            this.emit('new_socket', new MessagePortSocket(port));
        };
    }
    /**
     *
     */
    open() {
        this.worker.onconnect = this.handleMessageEvent;
    }
    /**
     *
     */
    close() {
        this.worker.onconnect = () => { };
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoic2hhcmVkX3dvcmtlcl9saXN0ZW5lci5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uLy4uL3NyYy93YXNtL3RyYW5zcG9ydC9icm93c2VyL3NoYXJlZF93b3JrZXJfbGlzdGVuZXIudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEsT0FBTyxZQUFZLE1BQU0sUUFBUSxDQUFDO0FBRWxDLE9BQU8sRUFBRSxpQkFBaUIsRUFBRSxNQUFNLDBCQUEwQixDQUFDO0FBWTdEOztHQUVHO0FBQ0gsTUFBTSxPQUFPLG9CQUFxQixTQUFRLFlBQVk7SUFDcEQ7OztPQUdHO0lBQ0gsWUFBb0IsTUFBK0I7UUFDakQsS0FBSyxFQUFFLENBQUM7UUFEVSxXQUFNLEdBQU4sTUFBTSxDQUF5QjtRQWtCbkQ7OztXQUdHO1FBQ0ssdUJBQWtCLEdBQUcsQ0FBQyxLQUFtQixFQUFFLEVBQUU7WUFDbkQsTUFBTSxDQUFDLElBQUksQ0FBQyxHQUFHLEtBQUssQ0FBQyxLQUFLLENBQUM7WUFDM0IsSUFBSSxDQUFDLElBQUksRUFBRTtnQkFDVCxPQUFPO2FBQ1I7WUFDRCxJQUFJLENBQUMsSUFBSSxDQUFDLFlBQVksRUFBRSxJQUFJLGlCQUFpQixDQUFDLElBQUksQ0FBQyxDQUFDLENBQUM7UUFDdkQsQ0FBQyxDQUFDO0lBMUJGLENBQUM7SUFFRDs7T0FFRztJQUNILElBQUk7UUFDRixJQUFJLENBQUMsTUFBTSxDQUFDLFNBQVMsR0FBRyxJQUFJLENBQUMsa0JBQWtCLENBQUM7SUFDbEQsQ0FBQztJQUVEOztPQUVHO0lBQ0gsS0FBSztRQUNILElBQUksQ0FBQyxNQUFNLENBQUMsU0FBUyxHQUFHLEdBQUcsRUFBRSxHQUFFLENBQUMsQ0FBQztJQUNuQyxDQUFDO0NBYUYifQ==