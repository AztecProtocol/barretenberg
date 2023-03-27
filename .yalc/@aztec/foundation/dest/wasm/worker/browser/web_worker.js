import { WasmModule } from '../../wasm/wasm_module.js';
import { createDispatchProxy, TransportClient, WorkerConnector } from '../../transport/index.js';
/**
 * Instantiate a web worker.
 * @param url - The URL.
 * @param initialMem - Initial memory pages.
 * @param maxMem - Maximum memory pages.
 * @returns The worker.
 */
export async function createWebWorker(url, initialMem, maxMem) {
    const worker = new Worker(url);
    const transportConnect = new WorkerConnector(worker);
    const transportClient = new TransportClient(transportConnect);
    await transportClient.open();
    const remoteModule = createDispatchProxy(WasmModule, transportClient);
    remoteModule.destroyWorker = async () => {
        await transportClient.request({ fn: '__destroyWorker__', args: [] });
        transportClient.close();
    };
    await remoteModule.init(initialMem, maxMem);
    return remoteModule;
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoid2ViX3dvcmtlci5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uLy4uL3NyYy93YXNtL3dvcmtlci9icm93c2VyL3dlYl93b3JrZXIudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEsT0FBTyxFQUFFLFVBQVUsRUFBRSxNQUFNLDJCQUEyQixDQUFDO0FBQ3ZELE9BQU8sRUFBRSxtQkFBbUIsRUFBZSxlQUFlLEVBQUUsZUFBZSxFQUFFLE1BQU0sMEJBQTBCLENBQUM7QUFHOUc7Ozs7OztHQU1HO0FBQ0gsTUFBTSxDQUFDLEtBQUssVUFBVSxlQUFlLENBQUMsR0FBVyxFQUFFLFVBQW1CLEVBQUUsTUFBZTtJQUNyRixNQUFNLE1BQU0sR0FBRyxJQUFJLE1BQU0sQ0FBQyxHQUFHLENBQUMsQ0FBQztJQUMvQixNQUFNLGdCQUFnQixHQUFHLElBQUksZUFBZSxDQUFDLE1BQU0sQ0FBQyxDQUFDO0lBQ3JELE1BQU0sZUFBZSxHQUFHLElBQUksZUFBZSxDQUFjLGdCQUFnQixDQUFDLENBQUM7SUFDM0UsTUFBTSxlQUFlLENBQUMsSUFBSSxFQUFFLENBQUM7SUFDN0IsTUFBTSxZQUFZLEdBQUcsbUJBQW1CLENBQUMsVUFBVSxFQUFFLGVBQWUsQ0FBZSxDQUFDO0lBQ3BGLFlBQVksQ0FBQyxhQUFhLEdBQUcsS0FBSyxJQUFJLEVBQUU7UUFDdEMsTUFBTSxlQUFlLENBQUMsT0FBTyxDQUFDLEVBQUUsRUFBRSxFQUFFLG1CQUFtQixFQUFFLElBQUksRUFBRSxFQUFFLEVBQUUsQ0FBQyxDQUFDO1FBQ3JFLGVBQWUsQ0FBQyxLQUFLLEVBQUUsQ0FBQztJQUMxQixDQUFDLENBQUM7SUFDRixNQUFNLFlBQVksQ0FBQyxJQUFJLENBQUMsVUFBVSxFQUFFLE1BQU0sQ0FBQyxDQUFDO0lBQzVDLE9BQU8sWUFBWSxDQUFDO0FBQ3RCLENBQUMifQ==