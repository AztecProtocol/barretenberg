import { Worker } from 'worker_threads';
import { createDispatchProxy, TransportClient } from '../../transport/index.js';
import { NodeConnector } from '../../transport/index.js';
import { WasmModule } from '../../wasm/wasm_module.js';
/**
 *
 */
export async function createNodeWorker(filepath, initialMem, maxMem) {
    const worker = new Worker(filepath);
    const transportConnect = new NodeConnector(worker);
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
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoibm9kZV93b3JrZXIuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi8uLi9zcmMvd2FzbS93b3JrZXIvbm9kZS9ub2RlX3dvcmtlci50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQSxPQUFPLEVBQUUsTUFBTSxFQUFFLE1BQU0sZ0JBQWdCLENBQUM7QUFDeEMsT0FBTyxFQUFFLG1CQUFtQixFQUFlLGVBQWUsRUFBRSxNQUFNLDBCQUEwQixDQUFDO0FBQzdGLE9BQU8sRUFBRSxhQUFhLEVBQUUsTUFBTSwwQkFBMEIsQ0FBQztBQUN6RCxPQUFPLEVBQUUsVUFBVSxFQUFFLE1BQU0sMkJBQTJCLENBQUM7QUFHdkQ7O0dBRUc7QUFDSCxNQUFNLENBQUMsS0FBSyxVQUFVLGdCQUFnQixDQUFDLFFBQWdCLEVBQUUsVUFBbUIsRUFBRSxNQUFlO0lBQzNGLE1BQU0sTUFBTSxHQUFHLElBQUksTUFBTSxDQUFDLFFBQVEsQ0FBQyxDQUFDO0lBQ3BDLE1BQU0sZ0JBQWdCLEdBQUcsSUFBSSxhQUFhLENBQUMsTUFBTSxDQUFDLENBQUM7SUFDbkQsTUFBTSxlQUFlLEdBQUcsSUFBSSxlQUFlLENBQWMsZ0JBQWdCLENBQUMsQ0FBQztJQUMzRSxNQUFNLGVBQWUsQ0FBQyxJQUFJLEVBQUUsQ0FBQztJQUM3QixNQUFNLFlBQVksR0FBRyxtQkFBbUIsQ0FBQyxVQUFVLEVBQUUsZUFBZSxDQUFlLENBQUM7SUFDcEYsWUFBWSxDQUFDLGFBQWEsR0FBRyxLQUFLLElBQUksRUFBRTtRQUN0QyxNQUFNLGVBQWUsQ0FBQyxPQUFPLENBQUMsRUFBRSxFQUFFLEVBQUUsbUJBQW1CLEVBQUUsSUFBSSxFQUFFLEVBQUUsRUFBRSxDQUFDLENBQUM7UUFDckUsZUFBZSxDQUFDLEtBQUssRUFBRSxDQUFDO0lBQzFCLENBQUMsQ0FBQztJQUNGLE1BQU0sWUFBWSxDQUFDLElBQUksQ0FBQyxVQUFVLEVBQUUsTUFBTSxDQUFDLENBQUM7SUFDNUMsT0FBTyxZQUFZLENBQUM7QUFDdEIsQ0FBQyJ9