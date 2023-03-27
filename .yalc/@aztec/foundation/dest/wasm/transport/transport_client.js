import { createDebugLogger } from '@aztec/foundation';
import EventEmitter from 'events';
import { isEventMessage } from './dispatch/messages.js';
const debug = createDebugLogger('aztec:transport_client');
/**
 * A TransportClient provides a request/response and event api to a corresponding TransportServer.
 * If `broadcast` is called on TransportServer, TransportClients will emit an `event_msg`.
 * The `request` method will block until a response is returned from the TransportServer's dispatch function.
 * Request multiplexing is supported.
 */
export class TransportClient extends EventEmitter {
    constructor(transportConnect) {
        super();
        this.transportConnect = transportConnect;
        this.msgId = 0;
        this.pendingRequests = [];
    }
    /**
     * Create and register our socket using our Connector.
     */
    async open() {
        this.socket = await this.transportConnect.createSocket();
        this.socket.registerHandler(msg => this.handleSocketMessage(msg));
    }
    /**
     * Close this and stop listening for messages.
     */
    close() {
        this.socket?.close();
        this.socket = undefined;
        this.removeAllListeners();
    }
    /**
     * Queue a request.
     * @param payload - The request payload.
     * @param transfer - Objects to transfer ownership of.
     * @returns A promise of the query result.
     */
    request(payload, transfer) {
        if (!this.socket) {
            throw new Error('Socket not open.');
        }
        const msgId = this.msgId++;
        const msg = { msgId, payload };
        debug(`->`, msg);
        return new Promise((resolve, reject) => {
            this.pendingRequests.push({ resolve, reject, msgId });
            this.socket.send(msg, transfer).catch(reject);
        });
    }
    /**
     * Handle an incoming socket message.
     * @param msg - The message.
     */
    handleSocketMessage(msg) {
        if (msg === undefined) {
            // The remote socket closed.
            this.close();
            return;
        }
        debug(`<-`, msg);
        if (isEventMessage(msg)) {
            this.emit('event_msg', msg.payload);
            return;
        }
        const reqIndex = this.pendingRequests.findIndex(r => r.msgId === msg.msgId);
        if (reqIndex === -1) {
            return;
        }
        const [pending] = this.pendingRequests.splice(reqIndex, 1);
        if (msg.error) {
            pending.reject(new Error(msg.error));
        }
        else {
            pending.resolve(msg.payload);
        }
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoidHJhbnNwb3J0X2NsaWVudC5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uL3NyYy93YXNtL3RyYW5zcG9ydC90cmFuc3BvcnRfY2xpZW50LnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBLE9BQU8sRUFBRSxpQkFBaUIsRUFBRSxNQUFNLG1CQUFtQixDQUFDO0FBQ3RELE9BQU8sWUFBWSxNQUFNLFFBQVEsQ0FBQztBQUNsQyxPQUFPLEVBQWdCLGNBQWMsRUFBbUIsTUFBTSx3QkFBd0IsQ0FBQztBQUl2RixNQUFNLEtBQUssR0FBRyxpQkFBaUIsQ0FBQyx3QkFBd0IsQ0FBQyxDQUFDO0FBc0IxRDs7Ozs7R0FLRztBQUNILE1BQU0sT0FBTyxlQUF5QixTQUFRLFlBQVk7SUFLeEQsWUFBb0IsZ0JBQTJCO1FBQzdDLEtBQUssRUFBRSxDQUFDO1FBRFUscUJBQWdCLEdBQWhCLGdCQUFnQixDQUFXO1FBSnZDLFVBQUssR0FBRyxDQUFDLENBQUM7UUFDVixvQkFBZSxHQUFxQixFQUFFLENBQUM7SUFLL0MsQ0FBQztJQUVEOztPQUVHO0lBQ0gsS0FBSyxDQUFDLElBQUk7UUFDUixJQUFJLENBQUMsTUFBTSxHQUFHLE1BQU0sSUFBSSxDQUFDLGdCQUFnQixDQUFDLFlBQVksRUFBRSxDQUFDO1FBQ3pELElBQUksQ0FBQyxNQUFNLENBQUMsZUFBZSxDQUFDLEdBQUcsQ0FBQyxFQUFFLENBQUMsSUFBSSxDQUFDLG1CQUFtQixDQUFDLEdBQUcsQ0FBQyxDQUFDLENBQUM7SUFDcEUsQ0FBQztJQUVEOztPQUVHO0lBQ0gsS0FBSztRQUNILElBQUksQ0FBQyxNQUFNLEVBQUUsS0FBSyxFQUFFLENBQUM7UUFDckIsSUFBSSxDQUFDLE1BQU0sR0FBRyxTQUFTLENBQUM7UUFDeEIsSUFBSSxDQUFDLGtCQUFrQixFQUFFLENBQUM7SUFDNUIsQ0FBQztJQUVEOzs7OztPQUtHO0lBQ0gsT0FBTyxDQUFDLE9BQWdCLEVBQUUsUUFBeUI7UUFDakQsSUFBSSxDQUFDLElBQUksQ0FBQyxNQUFNLEVBQUU7WUFDaEIsTUFBTSxJQUFJLEtBQUssQ0FBQyxrQkFBa0IsQ0FBQyxDQUFDO1NBQ3JDO1FBQ0QsTUFBTSxLQUFLLEdBQUcsSUFBSSxDQUFDLEtBQUssRUFBRSxDQUFDO1FBQzNCLE1BQU0sR0FBRyxHQUFHLEVBQUUsS0FBSyxFQUFFLE9BQU8sRUFBRSxDQUFDO1FBQy9CLEtBQUssQ0FBQyxJQUFJLEVBQUUsR0FBRyxDQUFDLENBQUM7UUFDakIsT0FBTyxJQUFJLE9BQU8sQ0FBTSxDQUFDLE9BQU8sRUFBRSxNQUFNLEVBQUUsRUFBRTtZQUMxQyxJQUFJLENBQUMsZUFBZSxDQUFDLElBQUksQ0FBQyxFQUFFLE9BQU8sRUFBRSxNQUFNLEVBQUUsS0FBSyxFQUFFLENBQUMsQ0FBQztZQUN0RCxJQUFJLENBQUMsTUFBTyxDQUFDLElBQUksQ0FBQyxHQUFHLEVBQUUsUUFBUSxDQUFDLENBQUMsS0FBSyxDQUFDLE1BQU0sQ0FBQyxDQUFDO1FBQ2pELENBQUMsQ0FBQyxDQUFDO0lBQ0wsQ0FBQztJQUVEOzs7T0FHRztJQUNLLG1CQUFtQixDQUFDLEdBQWlFO1FBQzNGLElBQUksR0FBRyxLQUFLLFNBQVMsRUFBRTtZQUNyQiw0QkFBNEI7WUFDNUIsSUFBSSxDQUFDLEtBQUssRUFBRSxDQUFDO1lBQ2IsT0FBTztTQUNSO1FBQ0QsS0FBSyxDQUFDLElBQUksRUFBRSxHQUFHLENBQUMsQ0FBQztRQUNqQixJQUFJLGNBQWMsQ0FBQyxHQUFHLENBQUMsRUFBRTtZQUN2QixJQUFJLENBQUMsSUFBSSxDQUFDLFdBQVcsRUFBRSxHQUFHLENBQUMsT0FBTyxDQUFDLENBQUM7WUFDcEMsT0FBTztTQUNSO1FBQ0QsTUFBTSxRQUFRLEdBQUcsSUFBSSxDQUFDLGVBQWUsQ0FBQyxTQUFTLENBQUMsQ0FBQyxDQUFDLEVBQUUsQ0FBQyxDQUFDLENBQUMsS0FBSyxLQUFLLEdBQUcsQ0FBQyxLQUFLLENBQUMsQ0FBQztRQUM1RSxJQUFJLFFBQVEsS0FBSyxDQUFDLENBQUMsRUFBRTtZQUNuQixPQUFPO1NBQ1I7UUFDRCxNQUFNLENBQUMsT0FBTyxDQUFDLEdBQUcsSUFBSSxDQUFDLGVBQWUsQ0FBQyxNQUFNLENBQUMsUUFBUSxFQUFFLENBQUMsQ0FBQyxDQUFDO1FBQzNELElBQUksR0FBRyxDQUFDLEtBQUssRUFBRTtZQUNiLE9BQU8sQ0FBQyxNQUFNLENBQUMsSUFBSSxLQUFLLENBQUMsR0FBRyxDQUFDLEtBQUssQ0FBQyxDQUFDLENBQUM7U0FDdEM7YUFBTTtZQUNMLE9BQU8sQ0FBQyxPQUFPLENBQUMsR0FBRyxDQUFDLE9BQU8sQ0FBQyxDQUFDO1NBQzlCO0lBQ0gsQ0FBQztDQUNGIn0=