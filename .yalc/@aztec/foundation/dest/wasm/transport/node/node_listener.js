import { parentPort } from 'worker_threads';
import EventEmitter from 'events';
import { NodeListenerSocket } from './node_listener_socket.js';
/**
 * A socket listener that works with Node.
 */
export class NodeListener extends EventEmitter {
    constructor() {
        super();
    }
    /**
     * Open the listener.
     */
    open() {
        this.emit('new_socket', new NodeListenerSocket(parentPort));
    }
    /**
     * Close the listener.
     */
    close() { }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoibm9kZV9saXN0ZW5lci5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uLy4uL3NyYy93YXNtL3RyYW5zcG9ydC9ub2RlL25vZGVfbGlzdGVuZXIudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEsT0FBTyxFQUFFLFVBQVUsRUFBRSxNQUFNLGdCQUFnQixDQUFDO0FBQzVDLE9BQU8sWUFBWSxNQUFNLFFBQVEsQ0FBQztBQUVsQyxPQUFPLEVBQUUsa0JBQWtCLEVBQUUsTUFBTSwyQkFBMkIsQ0FBQztBQUUvRDs7R0FFRztBQUNILE1BQU0sT0FBTyxZQUFhLFNBQVEsWUFBWTtJQUM1QztRQUNFLEtBQUssRUFBRSxDQUFDO0lBQ1YsQ0FBQztJQUVEOztPQUVHO0lBQ0gsSUFBSTtRQUNGLElBQUksQ0FBQyxJQUFJLENBQUMsWUFBWSxFQUFFLElBQUksa0JBQWtCLENBQUMsVUFBaUIsQ0FBQyxDQUFDLENBQUM7SUFDckUsQ0FBQztJQUVEOztPQUVHO0lBQ0gsS0FBSyxLQUFJLENBQUM7Q0FDWCJ9