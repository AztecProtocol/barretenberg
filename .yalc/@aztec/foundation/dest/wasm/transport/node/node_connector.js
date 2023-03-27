import { NodeConnectorSocket } from './node_connector_socket.js';
/**
 * Creates sockets backed by a Node worker.
 */
export class NodeConnector {
    constructor(worker) {
        this.worker = worker;
    }
    /**
     * Creates a socket backed by a node worker.
     * @returns The socket.
     */
    createSocket() {
        return Promise.resolve(new NodeConnectorSocket(this.worker));
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoibm9kZV9jb25uZWN0b3IuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi8uLi9zcmMvd2FzbS90cmFuc3BvcnQvbm9kZS9ub2RlX2Nvbm5lY3Rvci50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFFQSxPQUFPLEVBQUUsbUJBQW1CLEVBQUUsTUFBTSw0QkFBNEIsQ0FBQztBQUVqRTs7R0FFRztBQUNILE1BQU0sT0FBTyxhQUFhO0lBQ3hCLFlBQW9CLE1BQWM7UUFBZCxXQUFNLEdBQU4sTUFBTSxDQUFRO0lBQUcsQ0FBQztJQUV0Qzs7O09BR0c7SUFDSCxZQUFZO1FBQ1YsT0FBTyxPQUFPLENBQUMsT0FBTyxDQUFDLElBQUksbUJBQW1CLENBQUMsSUFBSSxDQUFDLE1BQU0sQ0FBQyxDQUFDLENBQUM7SUFDL0QsQ0FBQztDQUNGIn0=