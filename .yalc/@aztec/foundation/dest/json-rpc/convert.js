import { Buffer } from 'buffer';
/**
 * Convert a JSON-friendly object, which may encode a class object.
 * @param cc - The class converter.
 * @param obj - The encoded object.
 * @returns The decoded object.
 */
export function convertFromJsonObj(cc, obj) {
    if (!obj) {
        return obj; // Primitive type
    }
    // Is this a serialized Node buffer?
    if (obj.type === 'Buffer' && typeof obj.data === 'string') {
        return Buffer.from(obj.data, 'base64');
    }
    // Is this a convertible type?
    if (typeof obj.type === 'string' && cc.isRegisteredClassName(obj.type)) {
        return cc.toClassObj(obj);
    }
    // Is this an array?
    if (Array.isArray(obj)) {
        return obj.map((x) => convertFromJsonObj(cc, x));
    }
    // Is this a dictionary?
    if (obj.constructor === Object) {
        const newObj = {};
        for (const key of Object.keys(obj)) {
            newObj[key] = convertFromJsonObj(cc, obj[key]);
        }
        return newObj;
    }
    // Leave alone, assume JSON primitive
    return obj;
}
/**
 * Convert objects or classes to a JSON-friendly object.
 * @param cc - The class converter.
 * @param obj - The object.
 * @returns The encoded object.
 */
export function convertToJsonObj(cc, obj) {
    if (!obj) {
        return obj; // Primitive type
    }
    // Is this a Node buffer?
    if (obj instanceof Buffer) {
        return { type: 'Buffer', data: obj.toString('base64') };
    }
    // Is this a convertible type?
    if (cc.isRegisteredClass(obj.constructor)) {
        return cc.toJsonObj(obj);
    }
    // Is this an array?
    if (Array.isArray(obj)) {
        return obj.map((x) => convertToJsonObj(cc, x));
    }
    // Is this a dictionary?
    if (obj.constructor === Object) {
        const newObj = {};
        for (const key of Object.keys(obj)) {
            newObj[key] = convertToJsonObj(cc, obj[key]);
        }
        return newObj;
    }
    // Leave alone, assume JSON primitive
    return obj;
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiY29udmVydC5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uL3NyYy9qc29uLXJwYy9jb252ZXJ0LnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUNBLE9BQU8sRUFBRSxNQUFNLEVBQUUsTUFBTSxRQUFRLENBQUM7QUFFaEM7Ozs7O0dBS0c7QUFDSCxNQUFNLFVBQVUsa0JBQWtCLENBQUMsRUFBa0IsRUFBRSxHQUFRO0lBQzdELElBQUksQ0FBQyxHQUFHLEVBQUU7UUFDUixPQUFPLEdBQUcsQ0FBQyxDQUFDLGlCQUFpQjtLQUM5QjtJQUNELG9DQUFvQztJQUNwQyxJQUFJLEdBQUcsQ0FBQyxJQUFJLEtBQUssUUFBUSxJQUFJLE9BQU8sR0FBRyxDQUFDLElBQUksS0FBSyxRQUFRLEVBQUU7UUFDekQsT0FBTyxNQUFNLENBQUMsSUFBSSxDQUFDLEdBQUcsQ0FBQyxJQUFJLEVBQUUsUUFBUSxDQUFDLENBQUM7S0FDeEM7SUFDRCw4QkFBOEI7SUFDOUIsSUFBSSxPQUFPLEdBQUcsQ0FBQyxJQUFJLEtBQUssUUFBUSxJQUFJLEVBQUUsQ0FBQyxxQkFBcUIsQ0FBQyxHQUFHLENBQUMsSUFBSSxDQUFDLEVBQUU7UUFDdEUsT0FBTyxFQUFFLENBQUMsVUFBVSxDQUFDLEdBQUcsQ0FBQyxDQUFDO0tBQzNCO0lBRUQsb0JBQW9CO0lBQ3BCLElBQUksS0FBSyxDQUFDLE9BQU8sQ0FBQyxHQUFHLENBQUMsRUFBRTtRQUN0QixPQUFPLEdBQUcsQ0FBQyxHQUFHLENBQUMsQ0FBQyxDQUFNLEVBQUUsRUFBRSxDQUFDLGtCQUFrQixDQUFDLEVBQUUsRUFBRSxDQUFDLENBQUMsQ0FBQyxDQUFDO0tBQ3ZEO0lBQ0Qsd0JBQXdCO0lBQ3hCLElBQUksR0FBRyxDQUFDLFdBQVcsS0FBSyxNQUFNLEVBQUU7UUFDOUIsTUFBTSxNQUFNLEdBQVEsRUFBRSxDQUFDO1FBQ3ZCLEtBQUssTUFBTSxHQUFHLElBQUksTUFBTSxDQUFDLElBQUksQ0FBQyxHQUFHLENBQUMsRUFBRTtZQUNsQyxNQUFNLENBQUMsR0FBRyxDQUFDLEdBQUcsa0JBQWtCLENBQUMsRUFBRSxFQUFFLEdBQUcsQ0FBQyxHQUFHLENBQUMsQ0FBQyxDQUFDO1NBQ2hEO1FBQ0QsT0FBTyxNQUFNLENBQUM7S0FDZjtJQUVELHFDQUFxQztJQUNyQyxPQUFPLEdBQUcsQ0FBQztBQUNiLENBQUM7QUFFRDs7Ozs7R0FLRztBQUNILE1BQU0sVUFBVSxnQkFBZ0IsQ0FBQyxFQUFrQixFQUFFLEdBQVE7SUFDM0QsSUFBSSxDQUFDLEdBQUcsRUFBRTtRQUNSLE9BQU8sR0FBRyxDQUFDLENBQUMsaUJBQWlCO0tBQzlCO0lBQ0QseUJBQXlCO0lBQ3pCLElBQUksR0FBRyxZQUFZLE1BQU0sRUFBRTtRQUN6QixPQUFPLEVBQUUsSUFBSSxFQUFFLFFBQVEsRUFBRSxJQUFJLEVBQUUsR0FBRyxDQUFDLFFBQVEsQ0FBQyxRQUFRLENBQUMsRUFBRSxDQUFDO0tBQ3pEO0lBQ0QsOEJBQThCO0lBQzlCLElBQUksRUFBRSxDQUFDLGlCQUFpQixDQUFDLEdBQUcsQ0FBQyxXQUFXLENBQUMsRUFBRTtRQUN6QyxPQUFPLEVBQUUsQ0FBQyxTQUFTLENBQUMsR0FBRyxDQUFDLENBQUM7S0FDMUI7SUFDRCxvQkFBb0I7SUFDcEIsSUFBSSxLQUFLLENBQUMsT0FBTyxDQUFDLEdBQUcsQ0FBQyxFQUFFO1FBQ3RCLE9BQU8sR0FBRyxDQUFDLEdBQUcsQ0FBQyxDQUFDLENBQU0sRUFBRSxFQUFFLENBQUMsZ0JBQWdCLENBQUMsRUFBRSxFQUFFLENBQUMsQ0FBQyxDQUFDLENBQUM7S0FDckQ7SUFDRCx3QkFBd0I7SUFDeEIsSUFBSSxHQUFHLENBQUMsV0FBVyxLQUFLLE1BQU0sRUFBRTtRQUM5QixNQUFNLE1BQU0sR0FBUSxFQUFFLENBQUM7UUFDdkIsS0FBSyxNQUFNLEdBQUcsSUFBSSxNQUFNLENBQUMsSUFBSSxDQUFDLEdBQUcsQ0FBQyxFQUFFO1lBQ2xDLE1BQU0sQ0FBQyxHQUFHLENBQUMsR0FBRyxnQkFBZ0IsQ0FBQyxFQUFFLEVBQUUsR0FBRyxDQUFDLEdBQUcsQ0FBQyxDQUFDLENBQUM7U0FDOUM7UUFDRCxPQUFPLE1BQU0sQ0FBQztLQUNmO0lBRUQscUNBQXFDO0lBQ3JDLE9BQU8sR0FBRyxDQUFDO0FBQ2IsQ0FBQyJ9