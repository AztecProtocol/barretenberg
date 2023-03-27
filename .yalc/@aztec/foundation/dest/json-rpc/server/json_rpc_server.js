import http from 'http';
import Router from 'koa-router';
import cors from '@koa/cors';
import compress from 'koa-compress';
import Koa from 'koa';
import bodyParser from 'koa-bodyparser';
import { JsonProxy } from './json_proxy.js';
import { createDebugLogger } from '../../log/index.js';
const debug = createDebugLogger('json-rpc:json_rpc_server');
/**
 * JsonRpcServer.
 * Minimal, dev-friendly mechanism to create a server from an object.
 */
export class JsonRpcServer {
    constructor(handler, input) {
        this.handler = handler;
        this.proxy = new JsonProxy(handler, input);
    }
    /**
     * Get an express app object.
     * @param prefix - Our server prefix.
     * @returns The app object.
     */
    getApp(prefix = '') {
        const router = this.getRouter(prefix);
        const exceptionHandler = async (ctx, next) => {
            try {
                await next();
            }
            catch (err) {
                console.log(err);
                ctx.status = 400;
                ctx.body = { error: err.message };
            }
        };
        const app = new Koa();
        app.on('error', error => {
            console.log(`KOA app-level error. ${JSON.stringify({ error })}`);
        });
        app.use(compress({ br: false }));
        app.use(bodyParser());
        app.use(cors());
        app.use(exceptionHandler);
        app.use(router.routes());
        app.use(router.allowedMethods());
        return app;
    }
    /**
     * Get a router object wrapping our RPC class.
     * @param prefix - The server prefix.
     * @returns The router object.
     */
    getRouter(prefix) {
        const router = new Router({ prefix });
        const proto = Object.getPrototypeOf(this.handler);
        // Find all our endpoints from the handler methods
        for (const method of Object.getOwnPropertyNames(proto)) {
            // Ignore if not a function
            if (method === 'constructor' || typeof proto[method] !== 'function') {
                continue;
            }
            router.post(`/${method}`, async (ctx) => {
                const { params = [], jsonrpc, id } = ctx.request.body;
                debug('JsonRpcServer:getRouter', method, '<-', params);
                const result = await this.proxy.call(method, params);
                ctx.body = { jsonrpc, id, result };
                ctx.status = 200;
            });
        }
        return router;
    }
    /**
     * Start this server with koa.
     * @param port - Port number.
     * @param prefix - Prefix string.
     */
    start(port, prefix = '') {
        const httpServer = http.createServer(this.getApp(prefix).callback());
        httpServer.listen(port);
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoianNvbl9ycGNfc2VydmVyLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vc3JjL2pzb24tcnBjL3NlcnZlci9qc29uX3JwY19zZXJ2ZXIudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEsT0FBTyxJQUFJLE1BQU0sTUFBTSxDQUFDO0FBQ3hCLE9BQU8sTUFBTSxNQUFNLFlBQVksQ0FBQztBQUNoQyxPQUFPLElBQUksTUFBTSxXQUFXLENBQUM7QUFDN0IsT0FBTyxRQUFRLE1BQU0sY0FBYyxDQUFDO0FBRXBDLE9BQU8sR0FBRyxNQUFNLEtBQUssQ0FBQztBQUN0QixPQUFPLFVBQVUsTUFBTSxnQkFBZ0IsQ0FBQztBQUN4QyxPQUFPLEVBQUUsU0FBUyxFQUFFLE1BQU0saUJBQWlCLENBQUM7QUFDNUMsT0FBTyxFQUFFLGlCQUFpQixFQUFFLE1BQU0sb0JBQW9CLENBQUM7QUFFdkQsTUFBTSxLQUFLLEdBQUcsaUJBQWlCLENBQUMsMEJBQTBCLENBQUMsQ0FBQztBQUU1RDs7O0dBR0c7QUFDSCxNQUFNLE9BQU8sYUFBYTtJQUV4QixZQUFvQixPQUFlLEVBQUUsS0FBMEI7UUFBM0MsWUFBTyxHQUFQLE9BQU8sQ0FBUTtRQUNqQyxJQUFJLENBQUMsS0FBSyxHQUFHLElBQUksU0FBUyxDQUFDLE9BQU8sRUFBRSxLQUFLLENBQUMsQ0FBQztJQUM3QyxDQUFDO0lBRUQ7Ozs7T0FJRztJQUNJLE1BQU0sQ0FBQyxNQUFNLEdBQUcsRUFBRTtRQUN2QixNQUFNLE1BQU0sR0FBRyxJQUFJLENBQUMsU0FBUyxDQUFDLE1BQU0sQ0FBQyxDQUFDO1FBQ3RDLE1BQU0sZ0JBQWdCLEdBQUcsS0FBSyxFQUFFLEdBQWdCLEVBQUUsSUFBeUIsRUFBRSxFQUFFO1lBQzdFLElBQUk7Z0JBQ0YsTUFBTSxJQUFJLEVBQUUsQ0FBQzthQUNkO1lBQUMsT0FBTyxHQUFRLEVBQUU7Z0JBQ2pCLE9BQU8sQ0FBQyxHQUFHLENBQUMsR0FBRyxDQUFDLENBQUM7Z0JBQ2pC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