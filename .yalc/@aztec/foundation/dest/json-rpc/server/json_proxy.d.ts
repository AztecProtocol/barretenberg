import { ClassConverter, ClassConverterInput } from '../class_converter.js';
/**
 * Handles conversion of objects over the write.
 * Delegates to a ClassConverter object.
 */
export declare class JsonProxy {
    private handler;
    classConverter: ClassConverter;
    constructor(handler: object, input: ClassConverterInput);
    /**
     * Call an RPC method.
     * @param methodName - The RPC method.
     * @param jsonParams - The RPG parameters.
     * @returns The remote result.
     */
    call(methodName: string, jsonParams?: any[]): Promise<any>;
}
//# sourceMappingURL=json_proxy.d.ts.map