import { EventEmitter } from 'events';
import { isTransferDescriptor } from '../interface/transferable.js';
export function createDispatchProxyFromFn(class_, requestFn) {
    const proxy = class_.prototype instanceof EventEmitter ? new EventEmitter() : {};
    for (const fn of Object.getOwnPropertyNames(class_.prototype)) {
        if (fn === 'constructor') {
            continue;
        }
        proxy[fn] = requestFn(fn);
    }
    return proxy;
}
/**
 * Create a proxy object of our class T that uses transportClient
 * @param class_ - Our class T.
 * @param transportClient - The transport infrastructure.
 * @returns A proxy over T.
 */
export function createDispatchProxy(class_, transportClient) {
    // Create a proxy of class_ that passes along methods over our transportClient
    const proxy = createDispatchProxyFromFn(class_, (fn) => (...args) => {
        // Pass our proxied function name and arguments over our transport client
        const transfer = args.reduce((acc, a) => (isTransferDescriptor(a) ? [...acc, ...a.transferables] : acc), []);
        args = args.map(a => (isTransferDescriptor(a) ? a.send : a));
        return transportClient.request({ fn, args }, transfer);
    });
    if (proxy instanceof EventEmitter) {
        // Handle proxied 'emit' calls if our proxy object is an EventEmitter
        transportClient.on('event_msg', ({ fn, args }) => {
            if (fn === 'emit') {
                const [eventName, ...restArgs] = args;
                proxy.emit(eventName, ...restArgs);
            }
        });
    }
    return proxy;
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiY3JlYXRlX2Rpc3BhdGNoX3Byb3h5LmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vLi4vc3JjL3dhc20vdHJhbnNwb3J0L2Rpc3BhdGNoL2NyZWF0ZV9kaXNwYXRjaF9wcm94eS50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFFQSxPQUFPLEVBQUUsWUFBWSxFQUFFLE1BQU0sUUFBUSxDQUFDO0FBQ3RDLE9BQU8sRUFBRSxvQkFBb0IsRUFBc0IsTUFBTSw4QkFBOEIsQ0FBQztBQXdDeEYsTUFBTSxVQUFVLHlCQUF5QixDQUN2QyxNQUFtQyxFQUNuQyxTQUEyRDtJQUUzRCxNQUFNLEtBQUssR0FBUSxNQUFNLENBQUMsU0FBUyxZQUFZLFlBQVksQ0FBQyxDQUFDLENBQUMsSUFBSSxZQUFZLEVBQUUsQ0FBQyxDQUFDLENBQUMsRUFBRSxDQUFDO0lBQ3RGLEtBQUssTUFBTSxFQUFFLElBQUksTUFBTSxDQUFDLG1CQUFtQixDQUFDLE1BQU0sQ0FBQyxTQUFTLENBQUMsRUFBRTtRQUM3RCxJQUFJLEVBQUUsS0FBSyxhQUFhLEVBQUU7WUFDeEIsU0FBUztTQUNWO1FBQ0QsS0FBSyxDQUFDLEVBQUUsQ0FBQyxHQUFHLFNBQVMsQ0FBQyxFQUFFLENBQUMsQ0FBQztLQUMzQjtJQUNELE9BQU8sS0FBSyxDQUFDO0FBQ2YsQ0FBQztBQUVEOzs7OztHQUtHO0FBQ0gsTUFBTSxVQUFVLG1CQUFtQixDQUNqQyxNQUFtQyxFQUNuQyxlQUE2QztJQUU3Qyw4RUFBOEU7SUFDOUUsTUFBTSxLQUFLLEdBQUcseUJBQXlCLENBQUMsTUFBTSxFQUFFLENBQUMsRUFBVSxFQUFFLEVBQUUsQ0FBQyxDQUFDLEdBQUcsSUFBVyxFQUFFLEVBQUU7UUFDakYseUVBQXlFO1FBQ3pFLE1BQU0sUUFBUSxHQUFtQixJQUFJLENBQUMsTUFBTSxDQUMxQyxDQUFDLEdBQUcsRUFBRSxDQUFDLEVBQUUsRUFBRSxDQUFDLENBQUMsb0JBQW9CLENBQUMsQ0FBQyxDQUFDLENBQUMsQ0FBQyxDQUFDLENBQUMsR0FBRyxHQUFHLEVBQUUsR0FBRyxDQUFDLENBQUMsYUFBYSxDQUFDLENBQUMsQ0FBQyxDQUFDLEdBQUcsQ0FBQyxFQUMxRSxFQUFvQixDQUNyQixDQUFDO1FBQ0YsSUFBSSxHQUFHLElBQUksQ0FBQyxHQUFHLENBQUMsQ0FBQyxDQUFDLEVBQUUsQ0FBQyxDQUFDLG9CQUFvQixDQUFDLENBQUMsQ0FBQyxDQUFDLENBQUMsQ0FBQyxDQUFDLENBQUMsSUFBSSxDQUFDLENBQUMsQ0FBQyxDQUFDLENBQUMsQ0FBQyxDQUFDO1FBQzdELE9BQU8sZUFBZSxDQUFDLE9BQU8sQ0FBQyxFQUFFLEVBQUUsRUFBRSxJQUFJLEVBQUUsRUFBRSxRQUFRLENBQUMsQ0FBQztJQUN6RCxDQUFDLENBQUMsQ0FBQztJQUNILElBQUksS0FBSyxZQUFZLFlBQVksRUFBRTtRQUNqQyxxRUFBcUU7UUFDckUsZUFBZSxDQUFDLEVBQUUsQ0FBQyxXQUFXLEVBQUUsQ0FBQyxFQUFFLEVBQUUsRUFBRSxJQUFJLEVBQUUsRUFBRSxFQUFFO1lBQy9DLElBQUksRUFBRSxLQUFLLE1BQU0sRUFBRTtnQkFDakIsTUFBTSxDQUFDLFNBQVMsRUFBRSxHQUFHLFFBQVEsQ0FBQyxHQUFHLElBQUksQ0FBQztnQkFDdEMsS0FBSyxDQUFDLElBQUksQ0FBQyxTQUFTLEVBQUUsR0FBRyxRQUFRLENBQUMsQ0FBQzthQUNwQztRQUNILENBQUMsQ0FBQyxDQUFDO0tBQ0o7SUFDRCxPQUFPLEtBQUssQ0FBQztBQUNmLENBQUMifQ==