#pragma once

namespace proof_system {
// This will go away, but we move it here has a temporary measure to make proof_system not depend on honk or plonk.
enum ComposerType {
    STANDARD,
    TURBO,
    PLOOKUP,
    STANDARD_HONK, // Todo(Arijit): We should replace STANDARD_HONK outside plonk or the namespace should be
                   // proof_system
};
} // namespace proof_system