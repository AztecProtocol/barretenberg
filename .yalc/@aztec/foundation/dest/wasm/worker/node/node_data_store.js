import levelup from 'levelup';
import leveldown from 'leveldown';
import memdown from 'memdown';
/**
 * Cache for data used by wasm module.
 */
export class NodeDataStore {
    // eslint-disable-next-line
    constructor(path) {
        // Hack: Cast as any to work around packages "broken" with node16 resolution
        // See https://github.com/microsoft/TypeScript/issues/49160
        this.db = levelup(path ? leveldown(path) : memdown());
    }
    /**
     * Get a value from our DB.
     * @param key - The key to look up.
     * @returns The value.
     */
    async get(key) {
        return await this.db.get(key).catch(() => { });
    }
    /**
     * Set a value in our DB.
     * @param key - The key to update.
     * @param value - The value to set.
     */
    async set(key, value) {
        await this.db.put(key, value);
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoibm9kZV9kYXRhX3N0b3JlLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vLi4vc3JjL3dhc20vd29ya2VyL25vZGUvbm9kZV9kYXRhX3N0b3JlLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUNBLE9BQU8sT0FBb0IsTUFBTSxTQUFTLENBQUM7QUFDM0MsT0FBTyxTQUFTLE1BQU0sV0FBVyxDQUFDO0FBQ2xDLE9BQU8sT0FBTyxNQUFNLFNBQVMsQ0FBQztBQUU5Qjs7R0FFRztBQUNILE1BQU0sT0FBTyxhQUFhO0lBR3hCLDJCQUEyQjtJQUMzQixZQUFZLElBQWE7UUFDdkIsNEVBQTRFO1FBQzVFLDJEQUEyRDtRQUMzRCxJQUFJLENBQUMsRUFBRSxHQUFHLE9BQU8sQ0FBQyxJQUFJLENBQUMsQ0FBQyxDQUFFLFNBQWlCLENBQUMsSUFBSSxDQUFDLENBQUMsQ0FBQyxDQUFFLE9BQWUsRUFBRSxDQUFDLENBQUM7SUFDMUUsQ0FBQztJQUVEOzs7O09BSUc7SUFDSCxLQUFLLENBQUMsR0FBRyxDQUFDLEdBQVc7UUFDbkIsT0FBTyxNQUFNLElBQUksQ0FBQyxFQUFFLENBQUMsR0FBRyxDQUFDLEdBQUcsQ0FBQyxDQUFDLEtBQUssQ0FBQyxHQUFHLEVBQUUsR0FBRSxDQUFDLENBQUMsQ0FBQztJQUNoRCxDQUFDO0lBRUQ7Ozs7T0FJRztJQUNILEtBQUssQ0FBQyxHQUFHLENBQUMsR0FBVyxFQUFFLEtBQWE7UUFDbEMsTUFBTSxJQUFJLENBQUMsRUFBRSxDQUFDLEdBQUcsQ0FBQyxHQUFHLEVBQUUsS0FBSyxDQUFDLENBQUM7SUFDaEMsQ0FBQztDQUNGIn0=