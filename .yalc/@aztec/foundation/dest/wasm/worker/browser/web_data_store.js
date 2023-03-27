import levelup from 'levelup';
import memdown from 'memdown';
/**
 * Cache for data used by wasm module.
 * Stores in a LevelUp database.
 */
export class WebDataStore {
    constructor() {
        // TODO: The whole point of this is to reduce memory load in the browser.
        // Replace with leveljs so the data is stored in indexeddb and not in memory.
        // Hack: Cast as any to work around package "broken" with node16 resolution
        // See https://github.com/microsoft/TypeScript/issues/49160
        this.db = levelup(memdown());
    }
    /**
     * Lookup a key.
     * @param key - Key to lookup.
     * @returns The buffer.
     */
    async get(key) {
        return await this.db.get(key).catch(() => { });
    }
    /**
     * Alter a key.
     * @param key - Key to alter.
     * @param value - Buffer to store.
     */
    async set(key, value) {
        await this.db.put(key, value);
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoid2ViX2RhdGFfc3RvcmUuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi8uLi9zcmMvd2FzbS93b3JrZXIvYnJvd3Nlci93ZWJfZGF0YV9zdG9yZS50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFDQSxPQUFPLE9BQW9CLE1BQU0sU0FBUyxDQUFDO0FBQzNDLE9BQU8sT0FBTyxNQUFNLFNBQVMsQ0FBQztBQUU5Qjs7O0dBR0c7QUFDSCxNQUFNLE9BQU8sWUFBWTtJQUd2QjtRQUNFLHlFQUF5RTtRQUN6RSw2RUFBNkU7UUFDN0UsMkVBQTJFO1FBQzNFLDJEQUEyRDtRQUMzRCxJQUFJLENBQUMsRUFBRSxHQUFHLE9BQU8sQ0FBRSxPQUFlLEVBQUUsQ0FBQyxDQUFDO0lBQ3hDLENBQUM7SUFFRDs7OztPQUlHO0lBQ0gsS0FBSyxDQUFDLEdBQUcsQ0FBQyxHQUFXO1FBQ25CLE9BQU8sTUFBTSxJQUFJLENBQUMsRUFBRSxDQUFDLEdBQUcsQ0FBQyxHQUFHLENBQUMsQ0FBQyxLQUFLLENBQUMsR0FBRyxFQUFFLEdBQUUsQ0FBQyxDQUFDLENBQUM7SUFDaEQsQ0FBQztJQUVEOzs7O09BSUc7SUFDSCxLQUFLLENBQUMsR0FBRyxDQUFDLEdBQVcsRUFBRSxLQUFhO1FBQ2xDLE1BQU0sSUFBSSxDQUFDLEVBQUUsQ0FBQyxHQUFHLENBQUMsR0FBRyxFQUFFLEtBQUssQ0FBQyxDQUFDO0lBQ2hDLENBQUM7Q0FDRiJ9