#pragma once
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/stdlib/types/turbo.hpp"

namespace examples::simple {

using namespace proof_system::plonk;
using namespace stdlib::types;

Composer* create_composer(std::shared_ptr<proof_system::ReferenceStringFactory> const& crs_factory);

proof create_proof(Composer* composer);

bool verify_proof(Composer* composer, proof_system::plonk::proof const& proof);

void delete_composer(Composer* composer);

} // namespace examples::simple
