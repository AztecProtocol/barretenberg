#include "polynomial_store.hpp"
#include "barretenberg/env/data_store.hpp"

namespace bonk {

using namespace barretenberg;

// Set to 1 to enable logging.
#if 0
template <typename... Args> inline void debug(Args... args)
{
    info("PolynomialStoreWasm: ", args...);
}
#else
template <typename... Args> inline void debug(Args...) {}
#endif

void PolynomialStoreWasm::put(std::string const& key, polynomial const& poly)
{
    set_data(key.c_str(), poly.get_coefficients(), poly.size() * sizeof(fr));
}

polynomial PolynomialStoreWasm::get(std::string const& key) const
{
    size_t length_out;
    void* buf = get_data(key.c_str(), &length_out);
    return length_out ? polynomial((fr*)buf, length_out / sizeof(fr)) : polynomial();
}

} // namespace bonk
