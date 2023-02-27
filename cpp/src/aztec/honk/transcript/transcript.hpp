#pragma once

#include "common/serialize.hpp"
#include "crypto/pedersen/pedersen.hpp"
#include "crypto/blake3s/blake3s.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace honk {

/**
 * @brief Common transcript functionality for both parties. Stores the data for the current round, as well as the
 * manifest.
 *
 * @tparam FF Field from which we sample challenges.
 */
template <typename FF> class BaseTranscript {
    // TODO(Adrian): Make these tweakable
    static constexpr size_t HASH_OUTPUT_SIZE = 32;
    static constexpr size_t MIN_BYTES_PER_CHALLENGE = 128 / 8; // 128 bit challenges

    size_t round_number = 0;
    std::array<uint8_t, HASH_OUTPUT_SIZE> previous_challenge_buffer{};
    std::vector<uint8_t> current_round_data;

    /**
     * @brief Compute c_next = H( Compress(c_prev || round_buffer) )
     *
     * @return std::array<uint8_t, HASH_OUTPUT_SIZE>
     */
    [[nodiscard]] std::array<uint8_t, HASH_OUTPUT_SIZE> get_next_challenge_buffer() const
    {
        // Prevent challenge generation if nothing was sent by the prover.
        ASSERT(!current_round_data.empty());

        // concatenate the hash of the previous round (if not the first round) with the current round data.
        // TODO(Adrian): Do we want to use a domain separator as the initial challenge buffer?
        // We could be cheeky and use the hash of the manifest as domain separator, which would prevent us from having
        // to domain separate all the data. (See https://safe-hash.dev)
        std::vector<uint8_t> full_buffer;
        if (round_number > 0) {
            full_buffer.insert(full_buffer.end(), previous_challenge_buffer.begin(), previous_challenge_buffer.end());
        }
        full_buffer.insert(full_buffer.end(), current_round_data.begin(), current_round_data.end());

        // Optionally pre-hash the full buffer to minimize the amount of data passed to the cryptographic hash function.
        // Only a collision-resistant hash-function like Pedersen is required for this step.
        std::vector<uint8_t> compressed_buffer = to_buffer(crypto::pedersen::compress_native(full_buffer));

        // Use a strong hash function to derive the new challenge_buffer.
        auto base_hash = blake3::blake3s(compressed_buffer);

        std::array<uint8_t, HASH_OUTPUT_SIZE> new_challenge_buffer;
        std::copy_n(base_hash.begin(), HASH_OUTPUT_SIZE, new_challenge_buffer.begin());

        return new_challenge_buffer;
    };

  protected:
    /**
     * @brief Adds challenge elements to the current_round_buffer and updates the manifest.
     *
     * @param label of the element sent
     * @param element_bytes serialized
     */
    void consume_prover_element_bytes(const std::string& label, std::span<const uint8_t> element_bytes)
    {
        (void)label;

        current_round_data.insert(current_round_data.end(), element_bytes.begin(), element_bytes.end());
    }

  public:
    /**
     * @brief After all the prover messages have been sent, finalize the round by hashing all the data, create the field
     * elements and reset the state in preparation for the next round.
     *
     * @param labels human-readable names for the challenges for the manifest
     * @return std::array<FF, num_challenges> challenges for this round.
     */
    template <typename... Strings>
    std::array<FF, sizeof...(Strings)> get_challenges(const Strings&... labels)
    {
        constexpr size_t num_challenges = sizeof...(Strings);
        constexpr size_t bytes_per_challenge = HASH_OUTPUT_SIZE / num_challenges;

        // Ensure we have enough entropy from the hash function to construct each challenge.
        static_assert(bytes_per_challenge >= MIN_BYTES_PER_CHALLENGE, "requested too many challenges in this round");

        // TODO(Adrian): Add the challenge names to the map.
        ((void)labels, ...);

        // Compute the new challenge buffer from which we derive the challenges.
        auto next_challenge_buffer = get_next_challenge_buffer();

        // Create challenges from bytes.
        std::array<FF, num_challenges> challenges{};
        for (size_t i = 0; i < num_challenges; ++i) {
            // Initialize the buffer for the i-th challenge with 0s.
            std::array<uint8_t, sizeof(FF)> field_element_buffer{};
            // Copy the i-th chunk of size `bytes_per_challenge` to the start of `field_element_buffer`
            // The last bytes will be 0,
            std::copy_n(next_challenge_buffer.begin() + i * bytes_per_challenge,
                        bytes_per_challenge,
                        field_element_buffer.begin());

            // Create a FF element from a slice of bytes of next_challenge_buffer.
            challenges[i] = from_buffer<FF>(field_element_buffer);
        }

        // Prepare for next round.
        ++round_number;
        current_round_data.clear();
        previous_challenge_buffer = next_challenge_buffer;

        return challenges;
    }

    FF get_challenge(const std::string& label) { return get_challenges(label)[0]; }
};

template <typename FF> class ProverTranscript : public BaseTranscript<FF> {

  public:
    /// Contains the raw data sent by the prover.
    std::vector<uint8_t> proof_data;

    /**
     * @brief Adds a prover message to the transcript.
     *
     * @details Serializes the provided object into `proof_data`, and updates the current round state.
     *
     * @param label Description/name of the object being added.
     * @param element Serializable object that will be added to the transcript
     *
     * @todo Use a concept to only allow certain types to be passed. Requirements are that the object should be
     * serializable.
     *
     */
    template <class T> void send_to_verifier(const std::string& label, const T& element)
    {
        using serialize::write;
        // DANGER: When serializing an affine_element, we write the x and y coordinates
        // but this is annowing to deal with right now.
        auto element_bytes = to_buffer(element);
        proof_data.insert(proof_data.end(), element_bytes.begin(), element_bytes.end());

        BaseTranscript<FF>::consume_prover_element_bytes(label, element_bytes);
    }

    static ProverTranscript init_empty()
    {
        ProverTranscript<FF> transcript;
        constexpr uint32_t init{ 42 };
        transcript.send_to_verifier("Init", init);
        return transcript;
    };
};

template <class FF> class VerifierTranscript : public BaseTranscript<FF> {

    /// Contains the raw data sent by the prover.
    const std::vector<uint8_t> proof_data_;
    typename std::vector<uint8_t>::const_iterator read_iterator_;

  public:
    explicit VerifierTranscript(const std::vector<uint8_t>& proof_data)
        : proof_data_(proof_data.begin(), proof_data.end())
        , read_iterator_(proof_data_.cbegin())
    {}

    static VerifierTranscript init_empty(const ProverTranscript<FF>& transcript)
    {
        VerifierTranscript<FF> verifier_transcript{ transcript.proof_data };
        [[maybe_unused]] auto _ = verifier_transcript.template receive_from_prover<uint32_t>("Init");
        return verifier_transcript;
    };

    /**
     * @brief Reads the next element of type `T` from the transcript, with a predefined label.
     *
     * @details
     *
     * @param label Human readable name for the challenge.
     * @return deserialized element of type T
     */
    template <class T> T receive_from_prover(const std::string& label)
    {
        constexpr size_t element_size = sizeof(T);
        ASSERT(read_iterator_ + element_size <= proof_data_.end());

        std::span<const uint8_t> element_bytes{ read_iterator_, element_size };
        read_iterator_ += element_size;

        BaseTranscript<FF>::consume_prover_element_bytes(label, element_bytes);

        T element = from_buffer<T>(element_bytes);

        return element;
    }
};
} // namespace honk