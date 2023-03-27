/**
 * Represents a class compatible with our class conversion system.
 * E.g. PublicKey here satisfies 'IOClass'.
 * ```
 *    class PublicKey {
 *      toString() {
 *        return '...';
 *      }
 *      static fromString(str) {
 *        return new PublicKey(...);
 *      }
 *    }
 * ```
 */
interface IOClass {
    new (...args: any): any;
    /**
     * Creates an IOClass from a given string.
     */
    fromString: (str: string) => any;
}
/**
 * Registered classes available for conversion.
 */
export interface ClassConverterInput {
    [className: string]: IOClass;
}
/**
 * Represents a class in a JSON-friendly encoding.
 */
export interface JsonEncodedClass {
    /**
     * The class type.
     */
    type: string;
    /**
     * The class data string.
     */
    data: string;
}
/**
 * Handles mapping of classes to names, and calling toString and fromString to convert to and from JSON-friendly formats.
 * Takes a class map as input.
 */
export declare class ClassConverter {
    private toClass;
    private toName;
    /**
     * Create a class converter from a table of classes.
     * @param input - The class table.
     */
    constructor(input: ClassConverterInput);
    /**
     * Register a class with a certain name.
     * This name is used for conversion from and to this class.
     * @param type - The class name to use for serialization.
     * @param class_ - The class object.
     */
    register(type: string, class_: IOClass): void;
    /**
     * Does this type name have a registered class?
     * @param type - The type name.
     * @returns If there's a registered class.
     */
    isRegisteredClassName(type: string): boolean;
    /**
     * Is this class object registered?
     * @param obj - The class object.
     * @returns If it is a registered class.
     */
    isRegisteredClass(obj: any): boolean;
    /**
     * Convert a JSON-like object to a class object.
     * @param jsonObj - An object encoding a class.
     * @returns The class object.
     */
    toClassObj(jsonObj: JsonEncodedClass): any;
    /**
     * Convert a JSON-like object to a class object.
     * @param classObj - A JSON encoding a class.
     * @returns The class object.
     */
    toJsonObj(classObj: any): JsonEncodedClass;
}
export {};
//# sourceMappingURL=class_converter.d.ts.map