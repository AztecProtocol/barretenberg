#pragma once

#include "./transcript.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/ecc/curves/bn254/g1.hpp"
#include <unordered_map>

namespace transcript {
/**
 * Transcript extended with functions for easy
 * field element setting/getting
 */
class StandardTranscript : public Transcript {
  public:
    /**
     * Create a new standard transcript for Prover based on the manifest.
     *
     * @param input_manifest The manifest with round descriptions.
     * @param hash_type The hash to use for Fiat-Shamir.
     * @param challenge_bytes The number of bytes per challenge to generate.
     *
     * */
    StandardTranscript(const Manifest input_manifest,
                       const HashType hash_type = HashType::Keccak256,
                       const size_t challenge_bytes = 32)
        : Transcript(input_manifest, hash_type, challenge_bytes)
    {}
    /**
     * Parse a serialized version of an input_transcript into a deserialized
     * one based on the manifest.
     *
     * @param input_transcript Serialized transcript.
     * @param input_manifest The manifest which governs the parsing.
     * @param hash_type The hash used for Fiat-Shamir
     * @param challenge_bytes The number of bytes per challenge to generate.
     *
     * */
    StandardTranscript(const std::vector<uint8_t>& input_transcript,
                       const Manifest input_manifest,
                       const HashType hash_type = HashType::Keccak256,
                       const size_t challenge_bytes = 32)
        : Transcript(input_transcript, input_manifest, hash_type, challenge_bytes){};

    void add_field_element(const std::string& element_name, const barretenberg::fr& element);

    barretenberg::fr get_field_element(const std::string& element_name) const;
    barretenberg::g1::affine_element get_group_element(const std::string& element_name) const;

    std::vector<barretenberg::fr> get_field_element_vector(const std::string& element_name) const;

    barretenberg::fr get_challenge_field_element(const std::string& challenge_name, const size_t idx = 0) const;
    barretenberg::fr get_challenge_field_element_from_map(const std::string& challenge_name,
                                                          const std::string& challenge_map_name) const;

    std::vector<uint8_t> export_transcript() const { return Transcript::export_transcript(); }

    // TODO(luke): temporary function for debugging
    barretenberg::fr get_mock_challenge() { return barretenberg::fr::random_element(); };

    /**
     * @brief Returns transcript represented as a vector of barretenberg::fr.
     *        Used to represent recursive proofs (i.e. proof represented as circuit-native field elements)
     *
     * @return std::vector<barretenberg::fr>
     */
    std::vector<barretenberg::fr> export_transcript_in_recursion_format()
    {
        std::vector<barretenberg::fr> fields;
        const auto num_rounds = get_manifest().get_num_rounds();
        for (size_t i = 0; i < num_rounds; ++i) {
            for (const auto& manifest_element : get_manifest().get_round_manifest(i).elements) {
                if (!manifest_element.derived_by_verifier) {
                    if (manifest_element.num_bytes == 32 && manifest_element.name != "public_inputs") {
                        fields.emplace_back(get_field_element(manifest_element.name));
                    } else if (manifest_element.num_bytes == 64 && manifest_element.name != "public_inputs") {
                        const auto group_element = get_group_element(manifest_element.name);
                        const uint256_t x = group_element.x;
                        const uint256_t y = group_element.y;
                        const barretenberg::fr x_lo = x.slice(0, 136);
                        const barretenberg::fr x_hi = x.slice(136, 272);
                        const barretenberg::fr y_lo = y.slice(0, 136);
                        const barretenberg::fr y_hi = y.slice(136, 272);
                        fields.emplace_back(x_lo);
                        fields.emplace_back(x_hi);
                        fields.emplace_back(y_lo);
                        fields.emplace_back(y_hi);
                    } else {
                        ASSERT(manifest_element.name == "public_inputs");
                        const auto public_inputs_vector = get_field_element_vector(manifest_element.name);
                        for (const auto& ele : public_inputs_vector) {
                            fields.emplace_back(ele);
                        }
                    }
                }
            }
        }
        return fields;
    }

    /**
     * @brief Get a dummy fake proof for recursion. All elliptic curve group elements are still valid points to prevent
     * errors being thrown.
     *
     * @param manifest
     * @return std::vector<barretenberg::fr>
     */
    static std::vector<barretenberg::fr> export_dummy_transcript_in_recursion_format(const Manifest& manifest)
    {
        std::vector<barretenberg::fr> fields;
        const auto num_rounds = manifest.get_num_rounds();
        for (size_t i = 0; i < num_rounds; ++i) {
            for (const auto& manifest_element : manifest.get_round_manifest(i).elements) {
                if (!manifest_element.derived_by_verifier) {
                    if (manifest_element.num_bytes == 32 && manifest_element.name != "public_inputs") {
                        fields.emplace_back(0);
                    } else if (manifest_element.num_bytes == 64 && manifest_element.name != "public_inputs") {
                        const auto group_element = barretenberg::g1::affine_one;
                        const uint256_t x = group_element.x;
                        const uint256_t y = group_element.y;
                        const barretenberg::fr x_lo = x.slice(0, 136);
                        const barretenberg::fr x_hi = x.slice(136, 272);
                        const barretenberg::fr y_lo = y.slice(0, 136);
                        const barretenberg::fr y_hi = y.slice(136, 272);
                        fields.emplace_back(x_lo);
                        fields.emplace_back(x_hi);
                        fields.emplace_back(y_lo);
                        fields.emplace_back(y_hi);
                    } else {
                        ASSERT(manifest_element.name == "public_inputs");
                        const size_t num_public_inputs = manifest_element.num_bytes / 32;
                        for (size_t j = 0; j < num_public_inputs; ++j) {
                            fields.emplace_back(0);
                        }
                    }
                }
            }
        }
        return fields;
    }
};

} // namespace transcript
