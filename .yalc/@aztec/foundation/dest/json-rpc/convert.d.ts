import { ClassConverter } from './class_converter.js';
/**
 * Convert a JSON-friendly object, which may encode a class object.
 * @param cc - The class converter.
 * @param obj - The encoded object.
 * @returns The decoded object.
 */
export declare function convertFromJsonObj(cc: ClassConverter, obj: any): any;
/**
 * Convert objects or classes to a JSON-friendly object.
 * @param cc - The class converter.
 * @param obj - The object.
 * @returns The encoded object.
 */
export declare function convertToJsonObj(cc: ClassConverter, obj: any): any;
//# sourceMappingURL=convert.d.ts.map