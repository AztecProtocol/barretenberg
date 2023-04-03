#pragma once

namespace proof_system {
// This will go away, but we move it here has a temporary measure to make proof_system not depend on honk or plonk.
enum ComposerType { STANDARD, TURBO, PLOOKUP, STANDARD_HONK };
} // namespace proof_system