import { createDebugLogger } from '../../log/index.js';
import { ClassConverter } from '../class_converter.js';
import { convertFromJsonObj, convertToJsonObj } from '../convert.js';
import { assert, hasOwnProperty } from '../js_utils.js';
const debug = createDebugLogger('json-rpc:json_proxy');
/**
 * Handles conversion of objects over the write.
 * Delegates to a ClassConverter object.
 */
export class JsonProxy {
    constructor(handler, input) {
        this.handler = handler;
        this.classConverter = new ClassConverter(input);
    }
    /**
     * Call an RPC method.
     * @param methodName - The RPC method.
     * @param jsonParams - The RPG parameters.
     * @returns The remote result.
     */
    async call(methodName, jsonParams = []) {
        // Get access to our class members
        const proto = Object.getPrototypeOf(this.handler);
        assert(hasOwnProperty(proto, methodName), 'JsonProxy: Method not found!');
        assert(Array.isArray(jsonParams), 'JsonProxy: Params not an array!');
        // convert the params from json representation to classes
        const convertedParams = jsonParams.map(param => convertFromJsonObj(this.classConverter, param));
        debug('JsonProxy:call', this.handler, methodName, '<-', convertedParams);
        const rawRet = await this.handler[methodName](...convertedParams);
        const ret = convertToJsonObj(this.classConverter, rawRet);
        debug('JsonProxy:call', this.handler, methodName, '->', ret);
        return ret;
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoianNvbl9wcm94eS5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uL3NyYy9qc29uLXJwYy9zZXJ2ZXIvanNvbl9wcm94eS50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQSxPQUFPLEVBQUUsaUJBQWlCLEVBQUUsTUFBTSxvQkFBb0IsQ0FBQztBQUN2RCxPQUFPLEVBQUUsY0FBYyxFQUF1QixNQUFNLHVCQUF1QixDQUFDO0FBQzVFLE9BQU8sRUFBRSxrQkFBa0IsRUFBRSxnQkFBZ0IsRUFBRSxNQUFNLGVBQWUsQ0FBQztBQUNyRSxPQUFPLEVBQUUsTUFBTSxFQUFFLGNBQWMsRUFBRSxNQUFNLGdCQUFnQixDQUFDO0FBRXhELE1BQU0sS0FBSyxHQUFHLGlCQUFpQixDQUFDLHFCQUFxQixDQUFDLENBQUM7QUFFdkQ7OztHQUdHO0FBQ0gsTUFBTSxPQUFPLFNBQVM7SUFFcEIsWUFBb0IsT0FBZSxFQUFFLEtBQTBCO1FBQTNDLFlBQU8sR0FBUCxPQUFPLENBQVE7UUFDakMsSUFBSSxDQUFDLGNBQWMsR0FBRyxJQUFJLGNBQWMsQ0FBQyxLQUFLLENBQUMsQ0FBQztJQUNsRCxDQUFDO0lBQ0Q7Ozs7O09BS0c7SUFDSSxLQUFLLENBQUMsSUFBSSxDQUFDLFVBQWtCLEVBQUUsYUFBb0IsRUFBRTtRQUMxRCxrQ0FBa0M7UUFDbEMsTUFBTSxLQUFLLEdBQUcsTUFBTSxDQUFDLGNBQWMsQ0FBQyxJQUFJLENBQUMsT0FBTyxDQUFDLENBQUM7UUFDbEQsTUFBTSxDQUFDLGNBQWMsQ0FBQyxLQUFLLEVBQUUsVUFBVSxDQUFDLEVBQUUsOEJBQThCLENBQUMsQ0FBQztRQUMxRSxNQUFNLENBQUMsS0FBSyxDQUFDLE9BQU8sQ0FBQyxVQUFVLENBQUMsRUFBRSxpQ0FBaUMsQ0FBQyxDQUFDO1FBQ3JFLHlEQUF5RDtRQUN6RCxNQUFNLGVBQWUsR0FBRyxVQUFVLENBQUMsR0FBRyxDQUFDLEtBQUssQ0FBQyxFQUFFLENBQUMsa0JBQWtCLENBQUMsSUFBSSxDQUFDLGNBQWMsRUFBRSxLQUFLLENBQUMsQ0FBQyxDQUFDO1FBQ2hHLEtBQUssQ0FBQyxnQkFBZ0IsRUFBRSxJQUFJLENBQUMsT0FBTyxFQUFFLFVBQVUsRUFBRSxJQUFJLEVBQUUsZUFBZSxDQUFDLENBQUM7UUFDekUsTUFBTSxNQUFNLEdBQUcsTUFBTyxJQUFJLENBQUMsT0FBZSxDQUFDLFVBQVUsQ0FBQyxDQUFDLEdBQUcsZUFBZSxDQUFDLENBQUM7UUFDM0UsTUFBTSxHQUFHLEdBQUcsZ0JBQWdCLENBQUMsSUFBSSxDQUFDLGNBQWMsRUFBRSxNQUFNLENBQUMsQ0FBQztRQUMxRCxLQUFLLENBQUMsZ0JBQWdCLEVBQUUsSUFBSSxDQUFDLE9BQU8sRUFBRSxVQUFVLEVBQUUsSUFBSSxFQUFFLEdBQUcsQ0FBQyxDQUFDO1FBQzdELE9BQU8sR0FBRyxDQUFDO0lBQ2IsQ0FBQztDQUNGIn0=