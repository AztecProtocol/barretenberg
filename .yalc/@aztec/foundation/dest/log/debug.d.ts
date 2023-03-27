/**
 * A debug logger.
 */
export type DebugLogger = (...args: any[]) => void;
/**
 * Return a logger, meant to be silent by default and verbose during debugging.
 * @param name - The module name of the logger.
 * @returns A callable log function.
 */
export declare function createDebugLogger(name: string): DebugLogger;
export declare function setPreDebugLogHook(fn: (...args: any[]) => void): void;
export declare function setPostDebugLogHook(fn: (...args: any[]) => void): void;
export declare function enableLogs(str: string): void;
export declare function isLogEnabled(str: string): boolean;
//# sourceMappingURL=debug.d.ts.map