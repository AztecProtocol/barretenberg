import debug from 'debug';
let preLogHook;
let postLogHook;
function theFunctionThroughWhichAllLogsPass(logger, ...args) {
    if (!debug.enabled(logger.namespace)) {
        return;
    }
    if (preLogHook) {
        preLogHook(logger.namespace, ...args);
    }
    logger(...args);
    if (postLogHook) {
        postLogHook(logger.namespace, ...args);
    }
}
/**
 * Return a logger, meant to be silent by default and verbose during debugging.
 * @param name - The module name of the logger.
 * @returns A callable log function.
 */
export function createDebugLogger(name) {
    const logger = debug(name);
    return (...args) => theFunctionThroughWhichAllLogsPass(logger, ...args);
}
export function setPreDebugLogHook(fn) {
    preLogHook = fn;
}
export function setPostDebugLogHook(fn) {
    postLogHook = fn;
}
export function enableLogs(str) {
    debug.enable(str);
}
export function isLogEnabled(str) {
    return debug.enabled(str);
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiZGVidWcuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi9zcmMvbG9nL2RlYnVnLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBLE9BQU8sS0FBSyxNQUFNLE9BQU8sQ0FBQztBQUUxQixJQUFJLFVBQWtELENBQUM7QUFDdkQsSUFBSSxXQUFtRCxDQUFDO0FBRXhELFNBQVMsa0NBQWtDLENBQUMsTUFBVyxFQUFFLEdBQUcsSUFBVztJQUNyRSxJQUFJLENBQUMsS0FBSyxDQUFDLE9BQU8sQ0FBQyxNQUFNLENBQUMsU0FBUyxDQUFDLEVBQUU7UUFDcEMsT0FBTztLQUNSO0lBQ0QsSUFBSSxVQUFVLEVBQUU7UUFDZCxVQUFVLENBQUMsTUFBTSxDQUFDLFNBQVMsRUFBRSxHQUFHLElBQUksQ0FBQyxDQUFDO0tBQ3ZDO0lBQ0QsTUFBTSxDQUFDLEdBQUcsSUFBSSxDQUFDLENBQUM7SUFDaEIsSUFBSSxXQUFXLEVBQUU7UUFDZixXQUFXLENBQUMsTUFBTSxDQUFDLFNBQVMsRUFBRSxHQUFHLElBQUksQ0FBQyxDQUFDO0tBQ3hDO0FBQ0gsQ0FBQztBQU9EOzs7O0dBSUc7QUFDSCxNQUFNLFVBQVUsaUJBQWlCLENBQUMsSUFBWTtJQUM1QyxNQUFNLE1BQU0sR0FBRyxLQUFLLENBQUMsSUFBSSxDQUFDLENBQUM7SUFDM0IsT0FBTyxDQUFDLEdBQUcsSUFBVyxFQUFFLEVBQUUsQ0FBQyxrQ0FBa0MsQ0FBQyxNQUFNLEVBQUUsR0FBRyxJQUFJLENBQUMsQ0FBQztBQUNqRixDQUFDO0FBRUQsTUFBTSxVQUFVLGtCQUFrQixDQUFDLEVBQTRCO0lBQzdELFVBQVUsR0FBRyxFQUFFLENBQUM7QUFDbEIsQ0FBQztBQUVELE1BQU0sVUFBVSxtQkFBbUIsQ0FBQyxFQUE0QjtJQUM5RCxXQUFXLEdBQUcsRUFBRSxDQUFDO0FBQ25CLENBQUM7QUFFRCxNQUFNLFVBQVUsVUFBVSxDQUFDLEdBQVc7SUFDcEMsS0FBSyxDQUFDLE1BQU0sQ0FBQyxHQUFHLENBQUMsQ0FBQztBQUNwQixDQUFDO0FBRUQsTUFBTSxVQUFVLFlBQVksQ0FBQyxHQUFXO0lBQ3RDLE9BQU8sS0FBSyxDQUFDLE9BQU8sQ0FBQyxHQUFHLENBQUMsQ0FBQztBQUM1QixDQUFDIn0=