import { createDebugLogger } from '@aztec/foundation';
const debug = createDebugLogger('bb:worker_pool');
/**
 * Allocates a pool of WasmWorker's.
 * Worker 0 is allocated MAX_PAGES memory pages. This is because worker 0 will need to hold the proving key
 * (i.e. Has state), whereas the others are pure compute (they hold a little crs state).
 */
export class WorkerPool {
    constructor() {
        /**
         * The workers in the pool.
         */
        this.workers = [];
    }
    /**
     * Create an instance and initialize the workers.
     * @param createWorker - Worker factory.
     * @param poolSize - Pool size.
     * @returns An initialized WorkerPool.
     */
    static async new(createWorker, poolSize) {
        const pool = new WorkerPool();
        await pool.init(createWorker, poolSize);
        return pool;
    }
    /**
     * Initialize the workers.
     * @param createWorker - Worker factory().
     * @param poolSize - Pool size.
     * @param maxMem - Max memory pages.
     */
    async init(createWorker, poolSize, maxMem = WorkerPool.MAX_PAGES) {
        debug(`creating ${poolSize} workers...`);
        const start = new Date().getTime();
        this.workers = await Promise.all(Array(poolSize)
            .fill(0)
            .map((_, i) => createWorker(`${i}`, i === 0 ? Math.min(WorkerPool.MAX_PAGES, maxMem) : 768, maxMem)));
        debug(`created workers: ${new Date().getTime() - start}ms`);
    }
    /**
     * Tell all workers in the pool to stop processing.
     */
    async destroy() {
        await Promise.all(this.workers.map(w => w.destroyWorker()));
    }
}
// TODO(AD): Revisit what this means in aztec 3 context
// --
// Introduction of low mem prover work (polynomial cache) may actually increase mem usage when the backing store isn't
// enabled. We were seeing intermittent failings related to memory in production for some users when limiting to
// 6660 (416MB). It would be nice to understand why this is (the non determinism and/or the increased mem usage).
// For now, increasing mem usage to 512MB. This maybe preferable to backing out the low mem work, but
// ironically may break the chance of us using it in mobile.
// We *could* enable the low memory backing store, but this needs a little bit of work to actually
// read/write from indexeddb, performance testing, and actual further memory load testing.
// At this point it's hard to know what our memory savings would be relative to just fully reverting the LMP.
// public static MAX_PAGES = 6660;
/**
 * The maximum number of memory pages to be used by the webassembly.
 */
WorkerPool.MAX_PAGES = 8192;
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoid29ya2VyX3Bvb2wuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi9zcmMvd2FzbS93b3JrZXIvd29ya2VyX3Bvb2wudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEsT0FBTyxFQUFFLGlCQUFpQixFQUFFLE1BQU0sbUJBQW1CLENBQUM7QUFHdEQsTUFBTSxLQUFLLEdBQUcsaUJBQWlCLENBQUMsZ0JBQWdCLENBQUMsQ0FBQztBQU9sRDs7OztHQUlHO0FBQ0gsTUFBTSxPQUFPLFVBQVU7SUFBdkI7UUFnQkU7O1dBRUc7UUFDSyxZQUFPLEdBQWlCLEVBQUUsQ0FBQztJQXNDckMsQ0FBQztJQXBDQzs7Ozs7T0FLRztJQUNILE1BQU0sQ0FBQyxLQUFLLENBQUMsR0FBRyxDQUFDLFlBQTBCLEVBQUUsUUFBZ0I7UUFDM0QsTUFBTSxJQUFJLEdBQUcsSUFBSSxVQUFVLEVBQUUsQ0FBQztRQUM5QixNQUFNLElBQUksQ0FBQyxJQUFJLENBQUMsWUFBWSxFQUFFLFFBQVEsQ0FBQyxDQUFDO1FBQ3hDLE9BQU8sSUFBSSxDQUFDO0lBQ2QsQ0FBQztJQUVEOzs7OztPQUtHO0lBQ0ksS0FBSyxDQUFDLElBQUksQ0FBQyxZQUEwQixFQUFFLFFBQWdCLEVBQUUsTUFBTSxHQUFHLFVBQVUsQ0FBQyxTQUFTO1FBQzNGLEtBQUssQ0FBQyxZQUFZLFFBQVEsYUFBYSxDQUFDLENBQUM7UUFDekMsTUFBTSxLQUFLLEdBQUcsSUFBSSxJQUFJLEVBQUUsQ0FBQyxPQUFPLEVBQUUsQ0FBQztRQUNuQyxJQUFJLENBQUMsT0FBTyxHQUFHLE1BQU0sT0FBTyxDQUFDLEdBQUcsQ0FDOUIsS0FBSyxDQUFDLFFBQVEsQ0FBQzthQUNaLElBQUksQ0FBQyxDQUFDLENBQUM7YUFDUCxHQUFHLENBQUMsQ0FBQyxDQUFDLEVBQUUsQ0FBQyxFQUFFLEVBQUUsQ0FBQyxZQUFZLENBQUMsR0FBRyxDQUFDLEVBQUUsRUFBRSxDQUFDLEtBQUssQ0FBQyxDQUFDLENBQUMsQ0FBQyxJQUFJLENBQUMsR0FBRyxDQUFDLFVBQVUsQ0FBQyxTQUFTLEVBQUUsTUFBTSxDQUFDLENBQUMsQ0FBQyxDQUFDLEdBQUcsRUFBRSxNQUFNLENBQUMsQ0FBQyxDQUN2RyxDQUFDO1FBRUYsS0FBSyxDQUFDLG9CQUFvQixJQUFJLElBQUksRUFBRSxDQUFDLE9BQU8sRUFBRSxHQUFHLEtBQUssSUFBSSxDQUFDLENBQUM7SUFDOUQsQ0FBQztJQUVEOztPQUVHO0lBQ0ksS0FBSyxDQUFDLE9BQU87UUFDbEIsTUFBTSxPQUFPLENBQUMsR0FBRyxDQUFDLElBQUksQ0FBQyxPQUFPLENBQUMsR0FBRyxDQUFDLENBQUMsQ0FBQyxFQUFFLENBQUMsQ0FBQyxDQUFDLGFBQWEsRUFBRSxDQUFDLENBQUMsQ0FBQztJQUM5RCxDQUFDOztBQXZERCx1REFBdUQ7QUFDdkQsS0FBSztBQUNMLHNIQUFzSDtBQUN0SCxnSEFBZ0g7QUFDaEgsaUhBQWlIO0FBQ2pILHFHQUFxRztBQUNyRyw0REFBNEQ7QUFDNUQsa0dBQWtHO0FBQ2xHLDBGQUEwRjtBQUMxRiw2R0FBNkc7QUFDN0csa0NBQWtDO0FBQ2xDOztHQUVHO0FBQ1csb0JBQVMsR0FBRyxJQUFJLENBQUMifQ==