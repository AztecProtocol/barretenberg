import { TransportServer, WorkerListener } from '../../transport/index.js';
/**
 * Start the transport server corresponding to this module.
 * @param module - The WasmModule to host.
 */
export function startWebModule(module) {
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
    const transportListener = new WorkerListener(self);
    const transportServer = new TransportServer(transportListener, dispatch);
    module.addLogger((...args) => transportServer.broadcast({ fn: 'emit', args: ['log', ...args] }));
    transportServer.start();
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoic3RhcnRfd2ViX21vZHVsZS5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uLy4uL3NyYy93YXNtL3dvcmtlci9icm93c2VyL3N0YXJ0X3dlYl9tb2R1bGUudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEsT0FBTyxFQUFlLGVBQWUsRUFBRSxjQUFjLEVBQUUsTUFBTSwwQkFBMEIsQ0FBQztBQUd4Rjs7O0dBR0c7QUFDSCxNQUFNLFVBQVUsY0FBYyxDQUFDLE1BQWtCO0lBQy9DLE1BQU0sUUFBUSxHQUFHLEtBQUssRUFBRSxFQUFFLEVBQUUsRUFBRSxJQUFJLEVBQWUsRUFBRSxFQUFFO1FBQ25ELElBQUksRUFBRSxLQUFLLG1CQUFtQixFQUFFO1lBQzlCLGVBQWUsQ0FBQyxJQUFJLEVBQUUsQ0FBQztZQUN2QixPQUFPO1NBQ1I7UUFDRCxJQUFJLENBQUUsTUFBYyxDQUFDLEVBQUUsQ0FBQyxFQUFFO1lBQ3hCLE1BQU0sSUFBSSxLQUFLLENBQUMsdUNBQXVDLEVBQUUsRUFBRSxDQUFDLENBQUM7U0FDOUQ7UUFDRCxPQUFPLE1BQU8sTUFBYyxDQUFDLEVBQUUsQ0FBQyxDQUFDLEdBQUcsSUFBSSxDQUFDLENBQUM7SUFDNUMsQ0FBQyxDQUFDO0lBQ0YsTUFBTSxpQkFBaUIsR0FBRyxJQUFJLGNBQWMsQ0FBQyxJQUFJLENBQUMsQ0FBQztJQUNuRCxNQUFNLGVBQWUsR0FBRyxJQUFJLGVBQWUsQ0FBYyxpQkFBaUIsRUFBRSxRQUFRLENBQUMsQ0FBQztJQUN0RixNQUFNLENBQUMsU0FBUyxDQUFDLENBQUMsR0FBRyxJQUFXLEVBQUUsRUFBRSxDQUFDLGVBQWUsQ0FBQyxTQUFTLENBQUMsRUFBRSxFQUFFLEVBQUUsTUFBTSxFQUFFLElBQUksRUFBRSxDQUFDLEtBQUssRUFBRSxHQUFHLElBQUksQ0FBQyxFQUFFLENBQUMsQ0FBQyxDQUFDO0lBQ3hHLGVBQWUsQ0FBQyxLQUFLLEVBQUUsQ0FBQztBQUMxQixDQUFDIn0=