const $transferable = Symbol('thread.transferable');
/**
 *
 */
function isTransferable(thing) {
    if (!thing || typeof thing !== 'object')
        return false;
    // Don't check too thoroughly, since the list of transferable things in JS might grow over time
    return true;
}
/**
 *
 */
export function isTransferDescriptor(thing) {
    return thing && typeof thing === 'object' && thing[$transferable];
}
/**
 * Create a transfer descriptor, marking these as transferable.
 * @param payload - The payload.
 * @param transferables - The transferable objects.
 * @returns The descriptor.
 */
export function Transfer(payload, transferables) {
    if (!transferables) {
        if (!isTransferable(payload))
            throw Error();
        transferables = [payload];
    }
    return {
        [$transferable]: true,
        send: payload,
        transferables,
    };
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoidHJhbnNmZXJhYmxlLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vLi4vc3JjL3dhc20vdHJhbnNwb3J0L2ludGVyZmFjZS90cmFuc2ZlcmFibGUudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEsTUFBTSxhQUFhLEdBQUcsTUFBTSxDQUFDLHFCQUFxQixDQUFDLENBQUM7QUFxQnBEOztHQUVHO0FBQ0gsU0FBUyxjQUFjLENBQUMsS0FBVTtJQUNoQyxJQUFJLENBQUMsS0FBSyxJQUFJLE9BQU8sS0FBSyxLQUFLLFFBQVE7UUFBRSxPQUFPLEtBQUssQ0FBQztJQUN0RCwrRkFBK0Y7SUFDL0YsT0FBTyxJQUFJLENBQUM7QUFDZCxDQUFDO0FBRUQ7O0dBRUc7QUFDSCxNQUFNLFVBQVUsb0JBQW9CLENBQUMsS0FBVTtJQUM3QyxPQUFPLEtBQUssSUFBSSxPQUFPLEtBQUssS0FBSyxRQUFRLElBQUksS0FBSyxDQUFDLGFBQWEsQ0FBQyxDQUFDO0FBQ3BFLENBQUM7QUF1Q0Q7Ozs7O0dBS0c7QUFDSCxNQUFNLFVBQVUsUUFBUSxDQUFJLE9BQVUsRUFBRSxhQUE4QjtJQUNwRSxJQUFJLENBQUMsYUFBYSxFQUFFO1FBQ2xCLElBQUksQ0FBQyxjQUFjLENBQUMsT0FBTyxDQUFDO1lBQUUsTUFBTSxLQUFLLEVBQUUsQ0FBQztRQUM1QyxhQUFhLEdBQUcsQ0FBQyxPQUFPLENBQUMsQ0FBQztLQUMzQjtJQUVELE9BQU87UUFDTCxDQUFDLGFBQWEsQ0FBQyxFQUFFLElBQUk7UUFDckIsSUFBSSxFQUFFLE9BQU87UUFDYixhQUFhO0tBQ2QsQ0FBQztBQUNKLENBQUMifQ==