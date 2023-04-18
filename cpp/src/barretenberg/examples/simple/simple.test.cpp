#include "simple.hpp"
#include <barretenberg/srs/reference_string/pippenger_reference_string.hpp>
#include <barretenberg/common/test.hpp>
#include <filesystem>

namespace examples::simple {

TEST(examples_simple, create_proof)
{
    auto srs_path = std::filesystem::absolute("./srs_db/ignition");
    auto crs_factory = std::make_shared<proof_system::FileReferenceStringFactory>(srs_path);
    auto* composer = create_composer(crs_factory);
    auto proof = create_proof(composer);
    bool valid = verify_proof(composer, proof);
    delete_composer(composer);
    EXPECT_TRUE(valid);
}

} // namespace examples::simple
