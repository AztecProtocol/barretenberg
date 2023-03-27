import EventEmitter from 'events';
import { MessagePortSocket } from './message_port_socket.js';
/**
 *
 */
export class WorkerListener extends EventEmitter {
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
        this.worker.onmessage = this.handleMessageEvent;
    }
    /**
     *
     */
    close() {
        this.worker.onmessage = () => { };
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoid29ya2VyX2xpc3RlbmVyLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vLi4vc3JjL3dhc20vdHJhbnNwb3J0L2Jyb3dzZXIvd29ya2VyX2xpc3RlbmVyLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBLE9BQU8sWUFBWSxNQUFNLFFBQVEsQ0FBQztBQUVsQyxPQUFPLEVBQUUsaUJBQWlCLEVBQUUsTUFBTSwwQkFBMEIsQ0FBQztBQVk3RDs7R0FFRztBQUNILE1BQU0sT0FBTyxjQUFlLFNBQVEsWUFBWTtJQUM5Qzs7O09BR0c7SUFDSCxZQUFvQixNQUFrQztRQUNwRCxLQUFLLEVBQUUsQ0FBQztRQURVLFdBQU0sR0FBTixNQUFNLENBQTRCO1FBa0J0RDs7O1dBR0c7UUFDSyx1QkFBa0IsR0FBRyxDQUFDLEtBQW1CLEVBQUUsRUFBRTtZQUNuRCxNQUFNLENBQUMsSUFBSSxDQUFDLEdBQUcsS0FBSyxDQUFDLEtBQUssQ0FBQztZQUMzQixJQUFJLENBQUMsSUFBSSxFQUFFO2dCQUNULE9BQU87YUFDUjtZQUNELElBQUksQ0FBQyxJQUFJLENBQUMsWUFBWSxFQUFFLElBQUksaUJBQWlCLENBQUMsSUFBSSxDQUFDLENBQUMsQ0FBQztRQUN2RCxDQUFDLENBQUM7SUExQkYsQ0FBQztJQUVEOztPQUVHO0lBQ0gsSUFBSTtRQUNGLElBQUksQ0FBQyxNQUFNLENBQUMsU0FBUyxHQUFHLElBQUksQ0FBQyxrQkFBa0IsQ0FBQztJQUNsRCxDQUFDO0lBRUQ7O09BRUc7SUFDSCxLQUFLO1FBQ0gsSUFBSSxDQUFDLE1BQU0sQ0FBQyxTQUFTLEdBQUcsR0FBRyxFQUFFLEdBQUUsQ0FBQyxDQUFDO0lBQ25DLENBQUM7Q0FhRiJ9