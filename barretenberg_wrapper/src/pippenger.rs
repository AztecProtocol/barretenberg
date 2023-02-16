use crate::*;

pub fn new(crs_data: &[u8]) -> *mut std::os::raw::c_void {
    let num_points = (crs_data.len() / 64) as u64;
    let result: *mut std::os::raw::c_void;
    // TODO: this is a problem in the C++ code, it should not
    // TODO: need a mutable pointer
    let mut crs_data = crs_data.to_vec();

    unsafe {
        result = new_pippenger(crs_data.as_mut_ptr(), num_points);
    }
    result
}
