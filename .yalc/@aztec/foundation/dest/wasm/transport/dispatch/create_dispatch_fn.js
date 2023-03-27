/**
 *
 */
export function createDispatchFn(targetFn, debug = console.error) {
    return async ({ fn, args }) => {
        const target = targetFn();
        debug(`dispatching to ${target}: ${fn}`, args);
        return await target[fn](...args);
    };
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiY3JlYXRlX2Rpc3BhdGNoX2ZuLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vLi4vc3JjL3dhc20vdHJhbnNwb3J0L2Rpc3BhdGNoL2NyZWF0ZV9kaXNwYXRjaF9mbi50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFjQTs7R0FFRztBQUNILE1BQU0sVUFBVSxnQkFBZ0IsQ0FBQyxRQUFtQixFQUFFLEtBQUssR0FBRyxPQUFPLENBQUMsS0FBSztJQUN6RSxPQUFPLEtBQUssRUFBRSxFQUFFLEVBQUUsRUFBRSxJQUFJLEVBQWUsRUFBRSxFQUFFO1FBQ3pDLE1BQU0sTUFBTSxHQUFHLFFBQVEsRUFBRSxDQUFDO1FBQzFCLEtBQUssQ0FBQyxrQkFBa0IsTUFBTSxLQUFLLEVBQUUsRUFBRSxFQUFFLElBQUksQ0FBQyxDQUFDO1FBQy9DLE9BQU8sTUFBTSxNQUFNLENBQUMsRUFBRSxDQUFDLENBQUMsR0FBRyxJQUFJLENBQUMsQ0FBQztJQUNuQyxDQUFDLENBQUM7QUFDSixDQUFDIn0=