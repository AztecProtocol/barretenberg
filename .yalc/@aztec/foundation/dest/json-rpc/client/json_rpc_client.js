import { createDebugLogger } from '../../log/index.js';
import { retry } from '../../retry/index.js';
import { ClassConverter } from '../class_converter.js';
import { convertFromJsonObj, convertToJsonObj } from '../convert.js';
const debug = createDebugLogger('json-rpc:json_rpc_client');
/**
 * A normal fetch function that does not retry.
 * Alternatives are a fetch function with retries, or a mocked fetch.
 * @param host - The host URL.
 * @param method - The RPC method name.
 * @param body - The RPC payload.
 * @returns The parsed JSON response, or throws an error.
 */
export async function defaultFetch(host, rpcMethod, body) {
    debug(`JsonRpcClient.fetch`, host, rpcMethod, '<-', body);
    const resp = await fetch(`${host}/${rpcMethod}`, {
        method: 'POST',
        body: JSON.stringify(body),
        headers: { 'content-type': 'application/json' },
    });
    if (!resp.ok) {
        throw new Error(resp.statusText);
    }
    const text = await resp.text();
    try {
        return JSON.parse(text);
    }
    catch (err) {
        throw new Error(`Failed to parse body as JSON: ${text}`);
    }
}
/**
 * A fetch function with retries.
 */
export async function mustSucceedFetch(host, rpcMethod, body) {
    return await retry(() => defaultFetch(host, rpcMethod, body), 'JsonRpcClient request');
}
/**
 * Creates a Proxy object that delegates over RPC and satisfies RemoteObject<T>.
 * The server should have ran new JsonRpcServer().
 */
export function createJsonRpcClient(host, classMap, fetch = defaultFetch) {
    const classConverter = new ClassConverter(classMap);
    let id = 0;
    const request = async (method, params) => {
        const body = {
            jsonrpc: '2.0',
            id: id++,
            method,
            params: params.map(param => convertToJsonObj(classConverter, param)),
        };
        debug(`JsonRpcClient.request`, method, '<-', params);
        const res = await fetch(host, method, body);
        debug(`JsonRpcClient.request`, method, '->', res);
        if (res.error) {
            throw res.error;
        }
        return convertFromJsonObj(classConverter, res.result);
    };
    // Intercept any RPC methods with a proxy
    // This wraps 'request' with a method-call syntax wrapper
    return new Proxy({}, {
        get: (_, rpcMethod) => (...params) => {
            debug(`JsonRpcClient.constructor`, 'proxy', rpcMethod, '<-', params);
            return request(rpcMethod, params);
        },
    });
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoianNvbl9ycGNfY2xpZW50LmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vc3JjL2pzb24tcnBjL2NsaWVudC9qc29uX3JwY19jbGllbnQudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBS0EsT0FBTyxFQUFFLGlCQUFpQixFQUFFLE1BQU0sb0JBQW9CLENBQUM7QUFDdkQsT0FBTyxFQUFFLEtBQUssRUFBRSxNQUFNLHNCQUFzQixDQUFDO0FBQzdDLE9BQU8sRUFBRSxjQUFjLEVBQXVCLE1BQU0sdUJBQXVCLENBQUM7QUFDNUUsT0FBTyxFQUFFLGtCQUFrQixFQUFFLGdCQUFnQixFQUFFLE1BQU0sZUFBZSxDQUFDO0FBRXJFLE1BQU0sS0FBSyxHQUFHLGlCQUFpQixDQUFDLDBCQUEwQixDQUFDLENBQUM7QUFDNUQ7Ozs7Ozs7R0FPRztBQUNILE1BQU0sQ0FBQyxLQUFLLFVBQVUsWUFBWSxDQUFDLElBQVksRUFBRSxTQUFpQixFQUFFLElBQVM7SUFDM0UsS0FBSyxDQUFDLHFCQUFxQixFQUFFLElBQUksRUFBRSxTQUFTLEVBQUUsSUFBSSxFQUFFLElBQUksQ0FBQyxDQUFDO0lBQzFELE1BQU0sSUFBSSxHQUFHLE1BQU0sS0FBSyxDQUFDLEdBQUcsSUFBSSxJQUFJLFNBQVMsRUFBRSxFQUFFO1FBQy9DLE1BQU0sRUFBRSxNQUFNO1FBQ2QsSUFBSSxFQUFFLElBQUksQ0FBQyxTQUFTLENBQUMsSUFBSSxDQUFDO1FBQzFCLE9BQU8sRUFBRSxFQUFFLGNBQWMsRUFBRSxrQkFBa0IsRUFBRTtLQUNoRCxDQUFDLENBQUM7SUFFSCxJQUFJLENBQUMsSUFBSSxDQUFDLEVBQUUsRUFBRTtRQUNaLE1BQU0sSUFBSSxLQUFLLENBQUMsSUFBSSxDQUFDLFVBQVUsQ0FBQyxDQUFDO0tBQ2xDO0lBRUQsTUFBTSxJQUFJLEdBQUcsTUFBTSxJQUFJLENBQUMsSUFBSSxFQUFFLENBQUM7SUFDL0IsSUFBSTtRQUNGLE9BQU8sSUFBSSxDQUFDLEtBQUssQ0FBQyxJQUFJLENBQUMsQ0FBQztLQUN6QjtJQUFDLE9BQU8sR0FBRyxFQUFFO1FBQ1osTUFBTSxJQUFJLEtBQUssQ0FBQyxpQ0FBaUMsSUFBSSxFQUFFLENBQUMsQ0FBQztLQUMxRDtBQUNILENBQUM7QUFFRDs7R0FFRztBQUNILE1BQU0sQ0FBQyxLQUFLLFVBQVUsZ0JBQWdCLENBQUMsSUFBWSxFQUFFLFNBQWlCLEVBQUUsSUFBUztJQUMvRSxPQUFPLE1BQU0sS0FBSyxDQUFDLEdBQUcsRUFBRSxDQUFDLFlBQVksQ0FBQyxJQUFJLEVBQUUsU0FBUyxFQUFFLElBQUksQ0FBQyxFQUFFLHVCQUF1QixDQUFDLENBQUM7QUFDekYsQ0FBQztBQUVEOzs7R0FHRztBQUNILE1BQU0sVUFBVSxtQkFBbUIsQ0FDakMsSUFBWSxFQUNaLFFBQTZCLEVBQzdCLEtBQUssR0FBRyxZQUFZO0lBRXBCLE1BQU0sY0FBYyxHQUFHLElBQUksY0FBYyxDQUFDLFFBQVEsQ0FBQyxDQUFDO0lBQ3BELElBQUksRUFBRSxHQUFHLENBQUMsQ0FBQztJQUNYLE1BQU0sT0FBTyxHQUFHLEtBQUssRUFBRSxNQUFjLEVBQUUsTUFBYSxFQUFnQixFQUFFO1FBQ3BFLE1BQU0sSUFBSSxHQUFHO1lBQ1gsT0FBTyxFQUFFLEtBQUs7WUFDZCxFQUFFLEVBQUUsRUFBRSxFQUFFO1lBQ1IsTUFBTTtZQUNOLE1BQU0sRUFBRSxNQUFNLENBQUMsR0FBRyxDQUFDLEtBQUssQ0FBQyxFQUFFLENBQUMsZ0JBQWdCLENBQUMsY0FBYyxFQUFFLEtBQUssQ0FBQyxDQUFDO1NBQ3JFLENBQUM7UUFDRixLQUFLLENBQUMsdUJBQXVCLEVBQUUsTUFBTSxFQUFFLElBQUksRUFBRSxNQUFNLENBQUMsQ0FBQztRQUNyRCxNQUFNLEdBQUcsR0FBRyxNQUFNLEtBQUssQ0FBQyxJQUFJLEVBQUUsTUFBTSxFQUFFLElBQUksQ0FBQyxDQUFDO1FBQzVDLEtBQUssQ0FBQyx1QkFBdUIsRUFBRSxNQUFNLEVBQUUsSUFBSSxFQUFFLEdBQUcsQ0FBQyxDQUFDO1FBQ2xELElBQUksR0FBRyxDQUFDLEtBQUssRUFBRTtZQUNiLE1BQU0sR0FBRyxDQUFDLEtBQUssQ0FBQztTQUNqQjtRQUNELE9BQU8sa0JBQWtCLENBQUMsY0FBYyxFQUFFLEdBQUcsQ0FBQyxNQUFNLENBQUMsQ0FBQztJQUN4RCxDQUFDLENBQUM7SUFFRix5Q0FBeUM7SUFDekMseURBQXlEO0lBQ3pELE9BQU8sSUFBSSxLQUFLLENBQ2QsRUFBRSxFQUNGO1FBQ0UsR0FBRyxFQUNELENBQUMsQ0FBQyxFQUFFLFNBQWlCLEVBQUUsRUFBRSxDQUN6QixDQUFDLEdBQUcsTUFBYSxFQUFFLEVBQUU7WUFDbkIsS0FBSyxDQUFDLDJCQUEyQixFQUFFLE9BQU8sRUFBRSxTQUFTLEVBQUUsSUFBSSxFQUFFLE1BQU0sQ0FBQyxDQUFDO1lBQ3JFLE9BQU8sT0FBTyxDQUFDLFNBQVMsRUFBRSxNQUFNLENBQUMsQ0FBQztRQUNwQyxDQUFDO0tBQ0osQ0FDaUIsQ0FBQztBQUN2QixDQUFDIn0=