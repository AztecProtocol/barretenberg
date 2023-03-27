import { assert, hasOwnProperty } from './js_utils.js';
/**
 * Handles mapping of classes to names, and calling toString and fromString to convert to and from JSON-friendly formats.
 * Takes a class map as input.
 */
export class ClassConverter {
    /**
     * Create a class converter from a table of classes.
     * @param input - The class table.
     */
    constructor(input) {
        this.toClass = new Map();
        this.toName = new Map();
        for (const key of Object.keys(input)) {
            this.register(key, input[key]);
        }
    }
    /**
     * Register a class with a certain name.
     * This name is used for conversion from and to this class.
     * @param type - The class name to use for serialization.
     * @param class_ - The class object.
     */
    register(type, class_) {
        assert(type !== 'Buffer', "'Buffer' handling is hardcoded. Cannot use as name.");
        assert(hasOwnProperty(class_.prototype, 'toString'), `Class ${type} must define a toString() method.`);
        assert(class_['fromString'], `Class ${type} must define a fromString() static method.`);
        this.toName.set(class_, type);
        this.toClass.set(type, class_);
    }
    /**
     * Does this type name have a registered class?
     * @param type - The type name.
     * @returns If there's a registered class.
     */
    isRegisteredClassName(type) {
        return this.toClass.has(type);
    }
    /**
     * Is this class object registered?
     * @param obj - The class object.
     * @returns If it is a registered class.
     */
    isRegisteredClass(obj) {
        return this.toName.has(obj);
    }
    /**
     * Convert a JSON-like object to a class object.
     * @param jsonObj - An object encoding a class.
     * @returns The class object.
     */
    toClassObj(jsonObj) {
        const class_ = this.toClass.get(jsonObj.type);
        assert(class_, `Could not find type in lookup.`);
        return class_.fromString(jsonObj.data);
    }
    /**
     * Convert a JSON-like object to a class object.
     * @param classObj - A JSON encoding a class.
     * @returns The class object.
     */
    toJsonObj(classObj) {
        const type = this.toName.get(classObj.constructor);
        assert(type, `Could not find class in lookup.`);
        return { type: type, data: classObj.toString() };
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiY2xhc3NfY29udmVydGVyLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vc3JjL2pzb24tcnBjL2NsYXNzX2NvbnZlcnRlci50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQSxPQUFPLEVBQUUsTUFBTSxFQUFFLGNBQWMsRUFBRSxNQUFNLGVBQWUsQ0FBQztBQTZDdkQ7OztHQUdHO0FBQ0gsTUFBTSxPQUFPLGNBQWM7SUFJekI7OztPQUdHO0lBQ0gsWUFBWSxLQUEwQjtRQVA5QixZQUFPLEdBQUcsSUFBSSxHQUFHLEVBQW1CLENBQUM7UUFDckMsV0FBTSxHQUFHLElBQUksR0FBRyxFQUFtQixDQUFDO1FBTzFDLEtBQUssTUFBTSxHQUFHLElBQUksTUFBTSxDQUFDLElBQUksQ0FBQyxLQUFLLENBQUMsRUFBRTtZQUNwQyxJQUFJLENBQUMsUUFBUSxDQUFDLEdBQUcsRUFBRSxLQUFLLENBQUMsR0FBRyxDQUFDLENBQUMsQ0FBQztTQUNoQztJQUNILENBQUM7SUFDRDs7Ozs7T0FLRztJQUNILFFBQVEsQ0FBQyxJQUFZLEVBQUUsTUFBZTtRQUNwQyxNQUFNLENBQUMsSUFBSSxLQUFLLFFBQVEsRUFBRSxxREFBcUQsQ0FBQyxDQUFDO1FBQ2pGLE1BQU0sQ0FBQyxjQUFjLENBQUMsTUFBTSxDQUFDLFNBQVMsRUFBRSxVQUFVLENBQUMsRUFBRSxTQUFTLElBQUksbUNBQW1DLENBQUMsQ0FBQztRQUN2RyxNQUFNLENBQUMsTUFBTSxDQUFDLFlBQVksQ0FBQyxFQUFFLFNBQVMsSUFBSSw0Q0FBNEMsQ0FBQyxDQUFDO1FBQ3hGLElBQUksQ0FBQyxNQUFNLENBQUMsR0FBRyxDQUFDLE1BQU0sRUFBRSxJQUFJLENBQUMsQ0FBQztRQUM5QixJQUFJLENBQUMsT0FBTyxDQUFDLEdBQUcsQ0FBQyxJQUFJLEVBQUUsTUFBTSxDQUFDLENBQUM7SUFDakMsQ0FBQztJQUVEOzs7O09BSUc7SUFDSCxxQkFBcUIsQ0FBQyxJQUFZO1FBQ2hDLE9BQU8sSUFBSSxDQUFDLE9BQU8sQ0FBQyxHQUFHLENBQUMsSUFBSSxDQUFDLENBQUM7SUFDaEMsQ0FBQztJQUNEOzs7O09BSUc7SUFDSCxpQkFBaUIsQ0FBQyxHQUFRO1FBQ3hCLE9BQU8sSUFBSSxDQUFDLE1BQU0sQ0FBQyxHQUFHLENBQUMsR0FBRyxDQUFDLENBQUM7SUFDOUIsQ0FBQztJQUNEOzs7O09BSUc7SUFDSCxVQUFVLENBQUMsT0FBeUI7UUFDbEMsTUFBTSxNQUFNLEdBQUcsSUFBSSxDQUFDLE9BQU8sQ0FBQyxHQUFHLENBQUMsT0FBTyxDQUFDLElBQUksQ0FBQyxDQUFDO1FBQzlDLE1BQU0sQ0FBQyxNQUFNLEVBQUUsZ0NBQWdDLENBQUMsQ0FBQztRQUNqRCxPQUFPLE1BQU8sQ0FBQyxVQUFVLENBQUMsT0FBTyxDQUFDLElBQUksQ0FBQyxDQUFDO0lBQzFDLENBQUM7SUFDRDs7OztPQUlHO0lBQ0gsU0FBUyxDQUFDLFFBQWE7UUFDckIsTUFBTSxJQUFJLEdBQUcsSUFBSSxDQUFDLE1BQU0sQ0FBQyxHQUFHLENBQUMsUUFBUSxDQUFDLFdBQVcsQ0FBQyxDQUFDO1FBQ25ELE1BQU0sQ0FBQyxJQUFJLEVBQUUsaUNBQWlDLENBQUMsQ0FBQztRQUNoRCxPQUFPLEVBQUUsSUFBSSxFQUFFLElBQUssRUFBRSxJQUFJLEVBQUUsUUFBUSxDQUFDLFFBQVEsRUFBRSxFQUFFLENBQUM7SUFDcEQsQ0FBQztDQUNGIn0=