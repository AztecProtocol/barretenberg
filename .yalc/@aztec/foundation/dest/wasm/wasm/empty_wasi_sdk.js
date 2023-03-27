import { createDebugLogger } from '@aztec/foundation';
/**
 * Dummy implementation of a necessary part of the wasi api:
 * https://github.com/WebAssembly/WASI/blob/main/phases/snapshot/docs.md
 * We don't use these functions, but the environment expects them.
 * TODO find a way to update off of wasi 12.
 */
/* eslint-disable camelcase */
/* eslint-disable jsdoc/require-jsdoc */
export const getEmptyWasiSdk = (debug = createDebugLogger('wasm:empty_wasi_sdk')) => ({
    clock_time_get() {
        debug('clock_time_get');
    },
    environ_get() {
        debug('environ_get');
    },
    environ_sizes_get() {
        debug('environ_sizes_get');
    },
    fd_close() {
        debug('fd_close');
    },
    fd_read() {
        debug('fd_read');
    },
    fd_write() {
        debug('fd_write');
    },
    fd_seek() {
        debug('fd_seek');
    },
    fd_fdstat_get() {
        debug('fd_fdstat_get');
    },
    fd_fdstat_set_flags() {
        debug('fd_fdstat_set_flags');
    },
    fd_prestat_get() {
        debug('fd_prestat_get');
        return 8;
    },
    fd_prestat_dir_name() {
        debug('fd_prestat_dir_name');
        return 28;
    },
    path_open() {
        debug('path_open');
    },
    path_filestat_get() {
        debug('path_filestat_get');
    },
    proc_exit() {
        debug('proc_exit');
        return 52;
    },
    random_get() {
        debug('random_get');
        return 1;
    },
});
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiZW1wdHlfd2FzaV9zZGsuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi9zcmMvd2FzbS93YXNtL2VtcHR5X3dhc2lfc2RrLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBLE9BQU8sRUFBRSxpQkFBaUIsRUFBRSxNQUFNLG1CQUFtQixDQUFDO0FBRXREOzs7OztHQUtHO0FBQ0gsOEJBQThCO0FBQzlCLHdDQUF3QztBQUN4QyxNQUFNLENBQUMsTUFBTSxlQUFlLEdBQUcsQ0FBQyxLQUFLLEdBQUcsaUJBQWlCLENBQUMscUJBQXFCLENBQUMsRUFBRSxFQUFFLENBQUMsQ0FBQztJQUNwRixjQUFjO1FBQ1osS0FBSyxDQUFDLGdCQUFnQixDQUFDLENBQUM7SUFDMUIsQ0FBQztJQUNELFdBQVc7UUFDVCxLQUFLLENBQUMsYUFBYSxDQUFDLENBQUM7SUFDdkIsQ0FBQztJQUNELGlCQUFpQjtRQUNmLEtBQUssQ0FBQyxtQkFBbUIsQ0FBQyxDQUFDO0lBQzdCLENBQUM7SUFDRCxRQUFRO1FBQ04sS0FBSyxDQUFDLFVBQVUsQ0FBQyxDQUFDO0lBQ3BCLENBQUM7SUFDRCxPQUFPO1FBQ0wsS0FBSyxDQUFDLFNBQVMsQ0FBQyxDQUFDO0lBQ25CLENBQUM7SUFDRCxRQUFRO1FBQ04sS0FBSyxDQUFDLFVBQVUsQ0FBQyxDQUFDO0lBQ3BCLENBQUM7SUFDRCxPQUFPO1FBQ0wsS0FBSyxDQUFDLFNBQVMsQ0FBQyxDQUFDO0lBQ25CLENBQUM7SUFDRCxhQUFhO1FBQ1gsS0FBSyxDQUFDLGVBQWUsQ0FBQyxDQUFDO0lBQ3pCLENBQUM7SUFDRCxtQkFBbUI7UUFDakIsS0FBSyxDQUFDLHFCQUFxQixDQUFDLENBQUM7SUFDL0IsQ0FBQztJQUNELGNBQWM7UUFDWixLQUFLLENBQUMsZ0JBQWdCLENBQUMsQ0FBQztRQUN4QixPQUFPLENBQUMsQ0FBQztJQUNYLENBQUM7SUFDRCxtQkFBbUI7UUFDakIsS0FBSyxDQUFDLHFCQUFxQixDQUFDLENBQUM7UUFDN0IsT0FBTyxFQUFFLENBQUM7SUFDWixDQUFDO0lBQ0QsU0FBUztRQUNQLEtBQUssQ0FBQyxXQUFXLENBQUMsQ0FBQztJQUNyQixDQUFDO0lBQ0QsaUJBQWlCO1FBQ2YsS0FBSyxDQUFDLG1CQUFtQixDQUFDLENBQUM7SUFDN0IsQ0FBQztJQUNELFNBQVM7UUFDUCxLQUFLLENBQUMsV0FBVyxDQUFDLENBQUM7UUFDbkIsT0FBTyxFQUFFLENBQUM7SUFDWixDQUFDO0lBQ0QsVUFBVTtRQUNSLEtBQUssQ0FBQyxZQUFZLENBQUMsQ0FBQztRQUNwQixPQUFPLENBQUMsQ0FBQztJQUNYLENBQUM7Q0FDRixDQUFDLENBQUMifQ==