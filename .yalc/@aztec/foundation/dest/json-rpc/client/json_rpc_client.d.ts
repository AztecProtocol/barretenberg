import { RemoteObject } from 'comlink';
import { ClassConverterInput } from '../class_converter.js';
/**
 * A normal fetch function that does not retry.
 * Alternatives are a fetch function with retries, or a mocked fetch.
 * @param host - The host URL.
 * @param method - The RPC method name.
 * @param body - The RPC payload.
 * @returns The parsed JSON response, or throws an error.
 */
export declare function defaultFetch(host: string, rpcMethod: string, body: any): Promise<any>;
/**
 * A fetch function with retries.
 */
export declare function mustSucceedFetch(host: string, rpcMethod: string, body: any): Promise<any>;
/**
 * Creates a Proxy object that delegates over RPC and satisfies RemoteObject<T>.
 * The server should have ran new JsonRpcServer().
 */
export declare function createJsonRpcClient<T extends object>(host: string, classMap: ClassConverterInput, fetch?: typeof defaultFetch): RemoteObject<T>;
//# sourceMappingURL=json_rpc_client.d.ts.map