#pragma once
#include "../../transcript/transcript_wrappers.hpp"
namespace waffle {
class settings_base {
  public:
    static constexpr bool requires_shifted_wire(const uint64_t wire_shift_settings, const uint64_t wire_index)
    {
        return (((wire_shift_settings >> (wire_index)) & 1UL) == 1UL);
    }
};

class standard_settings : public settings_base {
  public:
    static constexpr size_t num_challenge_bytes = 32;
    static constexpr transcript::HashType hash_type = transcript::HashType::Keccak256;
    static constexpr size_t program_width = 3;
    static constexpr size_t num_shifted_wire_evaluations = 1;
    static constexpr uint64_t wire_shift_settings = 0b0100;
    static constexpr uint32_t permutation_shift = 30;
    static constexpr uint32_t permutation_mask = 0xC0000000;
    static constexpr bool use_linearisation = true;
    static constexpr size_t num_roots_cut_out_of_vanishing_polynomial = 4;
};

class unrolled_standard_settings : public settings_base {
  public:
    static constexpr size_t num_challenge_bytes = 16;
    static constexpr transcript::HashType hash_type = transcript::HashType::PedersenBlake2s;
    static constexpr size_t program_width = 3;
    static constexpr size_t num_shifted_wire_evaluations = 1;
    static constexpr uint64_t wire_shift_settings = 0b0100;
    static constexpr uint32_t permutation_shift = 30;
    static constexpr uint32_t permutation_mask = 0xC0000000;
    static constexpr bool use_linearisation = false;
    static constexpr size_t num_roots_cut_out_of_vanishing_polynomial = 4;
};

class turbo_settings : public settings_base {
  public:
    static constexpr size_t num_challenge_bytes = 32;
    static constexpr transcript::HashType hash_type = transcript::HashType::Keccak256;
    static constexpr size_t program_width = 4;
    static constexpr size_t num_shifted_wire_evaluations = 4;
    static constexpr uint64_t wire_shift_settings = 0b1111;
    static constexpr uint32_t permutation_shift = 30;
    static constexpr uint32_t permutation_mask = 0xC0000000;
    static constexpr bool use_linearisation = true;
    static constexpr size_t num_roots_cut_out_of_vanishing_polynomial = 4;
};

class plookup_settings : public settings_base {
  public:
    static constexpr size_t num_challenge_bytes = 32;
    static constexpr transcript::HashType hash_type = transcript::HashType::Keccak256;
    static constexpr size_t program_width = 4;
    static constexpr size_t num_shifted_wire_evaluations = 4;
    static constexpr uint64_t wire_shift_settings = 0b1111;
    static constexpr uint32_t permutation_shift = 30;
    static constexpr uint32_t permutation_mask = 0xC0000000;
    static constexpr bool use_linearisation = true;
    static constexpr size_t num_roots_cut_out_of_vanishing_polynomial = 4;
};

class unrolled_plookup_settings : public settings_base {
  public:
    static constexpr size_t num_challenge_bytes = 16;
    static constexpr transcript::HashType hash_type = transcript::HashType::PedersenBlake2s;
    static constexpr size_t program_width = 4;
    static constexpr size_t num_shifted_wire_evaluations = 4;
    static constexpr uint64_t wire_shift_settings = 0b1111;
    static constexpr uint32_t permutation_shift = 30;
    static constexpr uint32_t permutation_mask = 0xC0000000;
    static constexpr bool use_linearisation = false;
    static constexpr size_t num_roots_cut_out_of_vanishing_polynomial = 4;
};

class unrolled_turbo_settings : public settings_base {
  public:
    static constexpr size_t num_challenge_bytes = 16;
    static constexpr transcript::HashType hash_type = transcript::HashType::PedersenBlake2s;
    static constexpr size_t program_width = 4;
    static constexpr size_t num_shifted_wire_evaluations = 4;
    static constexpr uint64_t wire_shift_settings = 0b1111;
    static constexpr uint32_t permutation_shift = 30;
    static constexpr uint32_t permutation_mask = 0xC0000000;
    static constexpr bool use_linearisation = false;
    static constexpr size_t num_roots_cut_out_of_vanishing_polynomial = 4;
};
} // namespace waffle