import { parentPort } from 'worker_threads';
import { NodeListener, TransportServer } from '../../transport/index.js';
if (!parentPort) {
    throw new Error('InvalidWorker');
}
/**
 * Start the transport server corresponding to this module.
 * @param module - The WasmModule to host.
 */
export function startNodeModule(module) {
    const dispatch = async ({ fn, args }) => {
        if (fn === '__destroyWorker__') {
            transportServer.stop();
            return;
        }
        if (!module[fn]) {
            throw new Error(`dispatch error, function not found: ${fn}`);
        }
        return await module[fn](...args);
    };
    const transportListener = new NodeListener();
    const transportServer = new TransportServer(transportListener, dispatch);
    module.addLogger((...args) => transportServer.broadcast({ fn: 'emit', args: ['log', ...args] }));
    transportServer.start();
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoic3RhcnRfbm9kZV9tb2R1bGUuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi8uLi9zcmMvd2FzbS93b3JrZXIvbm9kZS9zdGFydF9ub2RlX21vZHVsZS50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQSxPQUFPLEVBQUUsVUFBVSxFQUFFLE1BQU0sZ0JBQWdCLENBQUM7QUFDNUMsT0FBTyxFQUFFLFlBQVksRUFBZSxlQUFlLEVBQUUsTUFBTSwwQkFBMEIsQ0FBQztBQUd0RixJQUFJLENBQUMsVUFBVSxFQUFFO0lBQ2YsTUFBTSxJQUFJLEtBQUssQ0FBQyxlQUFlLENBQUMsQ0FBQztDQUNsQztBQUVEOzs7R0FHRztBQUNILE1BQU0sVUFBVSxlQUFlLENBQUMsTUFBa0I7SUFDaEQsTUFBTSxRQUFRLEdBQUcsS0FBSyxFQUFFLEVBQUUsRUFBRSxFQUFFLElBQUksRUFBZSxFQUFFLEVBQUU7UUFDbkQsSUFBSSxFQUFFLEtBQUssbUJBQW1CLEVBQUU7WUFDOUIsZUFBZSxDQUFDLElBQUksRUFBRSxDQUFDO1lBQ3ZCLE9BQU87U0FDUjtRQUNELElBQUksQ0FBRSxNQUFjLENBQUMsRUFBRSxDQUFDLEVBQUU7WUFDeEIsTUFBTSxJQUFJLEtBQUssQ0FBQyx1Q0FBdUMsRUFBRSxFQUFFLENBQUMsQ0FBQztTQUM5RDtRQUNELE9BQU8sTUFBTyxNQUFjLENBQUMsRUFBRSxDQUFDLENBQUMsR0FBRyxJQUFJLENBQUMsQ0FBQztJQUM1QyxDQUFDLENBQUM7SUFDRixNQUFNLGlCQUFpQixHQUFHLElBQUksWUFBWSxFQUFFLENBQUM7SUFDN0MsTUFBTSxlQUFlLEdBQUcsSUFBSSxlQUFlLENBQWMsaUJBQWlCLEVBQUUsUUFBUSxDQUFDLENBQUM7SUFDdEYsTUFBTSxDQUFDLFNBQVMsQ0FBQyxDQUFDLEdBQUcsSUFBVyxFQUFFLEVBQUUsQ0FBQyxlQUFlLENBQUMsU0FBUyxDQUFDLEVBQUUsRUFBRSxFQUFFLE1BQU0sRUFBRSxJQUFJLEVBQUUsQ0FBQyxLQUFLLEVBQUUsR0FBRyxJQUFJLENBQUMsRUFBRSxDQUFDLENBQUMsQ0FBQztJQUN4RyxlQUFlLENBQUMsS0FBSyxFQUFFLENBQUM7QUFDMUIsQ0FBQyJ9