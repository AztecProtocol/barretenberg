// TODO should come from a dependency
/**
 * A simple fifo queue. It can grow unbounded. It can have multiple producers and consumers.
 * Putting an item onto the queue always succeeds, unless either end() or cancel() has been called in which case
 * the item being pushed is simply discarded.
 */
export class MemoryFifo {
    constructor() {
        this.waiting = [];
        this.items = [];
        this.flushing = false;
    }
    /**
     * Length of queue.
     * @returns integer.
     */
    length() {
        return this.items.length;
    }
    /**
     * Returns next item within the queue, or blocks until and item has been put into the queue.
     * If given a timeout, the promise will reject if no item is received after `timeout` seconds.
     * If the queue is flushing, `null` is returned.
     * @param timeout - In seconds.
     * @returns Promise of result.
     */
    get(timeout) {
        if (this.items.length) {
            return Promise.resolve(this.items.shift());
        }
        if (this.items.length === 0 && this.flushing) {
            return Promise.resolve(null);
        }
        return new Promise((resolve, reject) => {
            this.waiting.push(resolve);
            if (timeout) {
                setTimeout(() => {
                    const index = this.waiting.findIndex(r => r === resolve);
                    if (index > -1) {
                        this.waiting.splice(index, 1);
                        const err = new Error('Timeout getting item from queue.');
                        reject(err);
                    }
                }, timeout * 1000);
            }
        });
    }
    /**
     * Put an item onto back of the queue.
     * @param item - The item to enqueue.
     */
    put(item) {
        if (this.flushing) {
            return;
        }
        else if (this.waiting.length) {
            this.waiting.shift()(item);
        }
        else {
            this.items.push(item);
        }
    }
    /**
     * Once ended, no further items are added to queue. Consumers will consume remaining items within the queue.
     * The queue is not reusable after calling `end()`.
     * Any consumers waiting for an item receive null.
     */
    end() {
        this.flushing = true;
        this.waiting.forEach(resolve => resolve(null));
    }
    /**
     * Once cancelled, all items are discarded from the queue, and no further items are added to the queue.
     * The queue is not reusable after calling `cancel()`.
     * Any consumers waiting for an item receive null.
     */
    cancel() {
        this.flushing = true;
        this.items = [];
        this.waiting.forEach(resolve => resolve(null));
    }
    /**
     * Helper method that can be used to continously consume and process items on the queue.
     * @param handler - The item handler function.
     */
    async process(handler) {
        try {
            while (true) {
                const item = await this.get();
                if (item === null) {
                    break;
                }
                await handler(item);
            }
        }
        catch (err) {
            console.error('Queue handler exception:', err);
        }
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoibWVtb3J5X2ZpZm8uanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi9zcmMvd2FzbS9tZW1vcnlfZmlmby50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQSxxQ0FBcUM7QUFDckM7Ozs7R0FJRztBQUNILE1BQU0sT0FBTyxVQUFVO0lBQXZCO1FBQ1UsWUFBTyxHQUFpQyxFQUFFLENBQUM7UUFDM0MsVUFBSyxHQUFRLEVBQUUsQ0FBQztRQUNoQixhQUFRLEdBQUcsS0FBSyxDQUFDO0lBOEYzQixDQUFDO0lBNUZDOzs7T0FHRztJQUNJLE1BQU07UUFDWCxPQUFPLElBQUksQ0FBQyxLQUFLLENBQUMsTUFBTSxDQUFDO0lBQzNCLENBQUM7SUFFRDs7Ozs7O09BTUc7SUFDSSxHQUFHLENBQUMsT0FBZ0I7UUFDekIsSUFBSSxJQUFJLENBQUMsS0FBSyxDQUFDLE1BQU0sRUFBRTtZQUNyQixPQUFPLE9BQU8sQ0FBQyxPQUFPLENBQUMsSUFBSSxDQUFDLEtBQUssQ0FBQyxLQUFLLEVBQUcsQ0FBQyxDQUFDO1NBQzdDO1FBRUQsSUFBSSxJQUFJLENBQUMsS0FBSyxDQUFDLE1BQU0sS0FBSyxDQUFDLElBQUksSUFBSSxDQUFDLFFBQVEsRUFBRTtZQUM1QyxPQUFPLE9BQU8sQ0FBQyxPQUFPLENBQUMsSUFBSSxDQUFDLENBQUM7U0FDOUI7UUFFRCxPQUFPLElBQUksT0FBTyxDQUFXLENBQUMsT0FBTyxFQUFFLE1BQU0sRUFBRSxFQUFFO1lBQy9DLElBQUksQ0FBQyxPQUFPLENBQUMsSUFBSSxDQUFDLE9BQU8sQ0FBQyxDQUFDO1lBRTNCLElBQUksT0FBTyxFQUFFO2dCQUNYLFVBQVUsQ0FBQyxHQUFHLEVBQUU7b0JBQ2QsTUFBTSxLQUFLLEdBQUcsSUFBSSxDQUFDLE9BQU8sQ0FBQyxTQUFTLENBQUMsQ0FBQyxDQUFDLEVBQUUsQ0FBQyxDQUFDLEtBQUssT0FBTyxDQUFDLENBQUM7b0JBQ3pELElBQUksS0FBSyxHQUFHLENBQUMsQ0FBQyxFQUFFO3dCQUNkLElBQUksQ0FBQyxPQUFPLENBQUMsTUFBTSxDQUFDLEtBQUssRUFBRSxDQUFDLENBQUMsQ0FBQzt3QkFDOUIsTUFBTSxHQUFHLEdBQUcsSUFBSSxLQUFLLENBQUMsa0NBQWtDLENBQUMsQ0FBQzt3QkFDMUQsTUFBTSxDQUFDLEdBQUcsQ0FBQyxDQUFDO3FCQUNiO2dCQUNILENBQUMsRUFBRSxPQUFPLEdBQUcsSUFBSSxDQUFDLENBQUM7YUFDcEI7UUFDSCxDQUFDLENBQUMsQ0FBQztJQUNMLENBQUM7SUFFRDs7O09BR0c7SUFDSSxHQUFHLENBQUMsSUFBTztRQUNoQixJQUFJLElBQUksQ0FBQyxRQUFRLEVBQUU7WUFDakIsT0FBTztTQUNSO2FBQU0sSUFBSSxJQUFJLENBQUMsT0FBTyxDQUFDLE1BQU0sRUFBRTtZQUM5QixJQUFJLENBQUMsT0FBTyxDQUFDLEtBQUssRUFBRyxDQUFDLElBQUksQ0FBQyxDQUFDO1NBQzdCO2FBQU07WUFDTCxJQUFJLENBQUMsS0FBSyxDQUFDLElBQUksQ0FBQyxJQUFJLENBQUMsQ0FBQztTQUN2QjtJQUNILENBQUM7SUFFRDs7OztPQUlHO0lBQ0ksR0FBRztRQUNSLElBQUksQ0FBQyxRQUFRLEdBQUcsSUFBSSxDQUFDO1FBQ3JCLElBQUksQ0FBQyxPQUFPLENBQUMsT0FBTyxDQUFDLE9BQU8sQ0FBQyxFQUFFLENBQUMsT0FBTyxDQUFDLElBQUksQ0FBQyxDQUFDLENBQUM7SUFDakQsQ0FBQztJQUVEOzs7O09BSUc7SUFDSSxNQUFNO1FBQ1gsSUFBSSxDQUFDLFFBQVEsR0FBRyxJQUFJLENBQUM7UUFDckIsSUFBSSxDQUFDLEtBQUssR0FBRyxFQUFFLENBQUM7UUFDaEIsSUFBSSxDQUFDLE9BQU8sQ0FBQyxPQUFPLENBQUMsT0FBTyxDQUFDLEVBQUUsQ0FBQyxPQUFPLENBQUMsSUFBSSxDQUFDLENBQUMsQ0FBQztJQUNqRCxDQUFDO0lBRUQ7OztPQUdHO0lBQ0ksS0FBSyxDQUFDLE9BQU8sQ0FBQyxPQUFtQztRQUN0RCxJQUFJO1lBQ0YsT0FBTyxJQUFJLEVBQUU7Z0JBQ1gsTUFBTSxJQUFJLEdBQUcsTUFBTSxJQUFJLENBQUMsR0FBRyxFQUFFLENBQUM7Z0JBQzlCLElBQUksSUFBSSxLQUFLLElBQUksRUFBRTtvQkFDakIsTUFBTTtpQkFDUDtnQkFDRCxNQUFNLE9BQU8sQ0FBQyxJQUFJLENBQUMsQ0FBQzthQUNyQjtTQUNGO1FBQUMsT0FBTyxHQUFHLEVBQUU7WUFDWixPQUFPLENBQUMsS0FBSyxDQUFDLDBCQUEwQixFQUFFLEdBQUcsQ0FBQyxDQUFDO1NBQ2hEO0lBQ0gsQ0FBQztDQUNGIn0=