#include <cstdint>
#include <barretenberg/common/wasm_export.hpp>

WASM_EXPORT void srs_init_srs(uint8_t const* points_buf, uint32_t const* num_points, uint8_t const* g2_point_buf);
WASM_EXPORT void srs_init_from_path(char const* crs_path);