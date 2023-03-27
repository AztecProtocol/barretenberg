import { ClassConverterInput } from '../class_converter.js';
import Koa from 'koa';
import { JsonProxy } from './json_proxy.js';
/**
 * JsonRpcServer.
 * Minimal, dev-friendly mechanism to create a server from an object.
 */
export declare class JsonRpcServer {
    private handler;
    proxy: JsonProxy;
    constructor(handler: object, input: ClassConverterInput);
    /**
     * Get an express app object.
     * @param prefix - Our server prefix.
     * @returns The app object.
     */
    getApp(prefix?: string): Koa<Koa.DefaultState, Koa.DefaultContext>;
    /**
     * Get a router object wrapping our RPC class.
     * @param prefix - The server prefix.
     * @returns The router object.
     */
    private getRouter;
    /**
     * Start this server with koa.
     * @param port - Port number.
     * @param prefix - Prefix string.
     */
    start(port: number, prefix?: string): void;
}
//# sourceMappingURL=json_rpc_server.d.ts.map