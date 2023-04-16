#include "barretenberg/crypto/sha256/sha256.hpp"
#include "barretenberg/crypto/pedersen_commitment/pedersen.hpp"
#include "barretenberg/crypto/pedersen_commitment/pedersen_lookup.hpp"
#include "barretenberg/polynomials/evaluation_domain.hpp"
#include "verification_key.hpp"
#include "barretenberg/plonk/proof_system/constants.hpp"

namespace proof_system::plonk {

/**
 * @brief Hashes the evaluation domain to match the 'circuit' approach taken in
 * stdlib/recursion/verification_key/verification_key.hpp.
 * @note: in that reference file, the circuit-equivalent of this function is a _method_ of the `evaluation_domain'
 * struct. But we cannot do that with the native `barretenberg::evaluation_domain` type unfortunately, because it's
 * defined in polynomials/evaluation_domain.hpp, and `polynomial` is a bberg library which does not depend on `crypto`
 * in its CMakeLists.txt file. (We'd need `crypto` to be able to call native pedersen functions).
 *
 * @param domain to compress
 * @param composer_type to use when choosing pedersen compression function
 * @return barretenberg::fr compression of the evaluation domain as a field
 */
barretenberg::fr compress_native_evaluation_domain(barretenberg::evaluation_domain const& domain,
                                                   proof_system::ComposerType composer_type)
{
    barretenberg::fr out;
    if (composer_type == proof_system::ComposerType::PLOOKUP) {
        out = crypto::pedersen_commitment::lookup::compress_native({
            domain.root,
            domain.domain,
            domain.generator,
        });
    } else {
        out = crypto::pedersen_commitment::compress_native({
            domain.root,
            domain.domain,
            domain.generator,
        });
    }
    return out;
}

/**
 * @brief Compress the verification key data.
 *
 * @details Native pedersen compression of VK data that is truly core to a VK.
 * Omits recursion proof flag and recursion input indices as they are not really
 * core to the VK itself.
 *
 * @param hash_index generator index to use during pedersen compression
 * @returns a field containing the compression
 */
barretenberg::fr verification_key_data::compress_native(const size_t hash_index)
{
    barretenberg::evaluation_domain domain = evaluation_domain(circuit_size);
    barretenberg::fr compressed_domain =
        compress_native_evaluation_domain(domain, proof_system::ComposerType(composer_type));

    constexpr size_t num_limb_bits = plonk::NUM_LIMB_BITS_IN_FIELD_SIMULATION;

    const auto split_bigfield_limbs = [](const uint256_t& element) {
        std::vector<barretenberg::fr> limbs;
        limbs.push_back(element.slice(0, num_limb_bits));
        limbs.push_back(element.slice(num_limb_bits, num_limb_bits * 2));
        limbs.push_back(element.slice(num_limb_bits * 2, num_limb_bits * 3));
        limbs.push_back(element.slice(num_limb_bits * 3, num_limb_bits * 4));
        return limbs;
    };

    std::vector<barretenberg::fr> preimage_data;
    preimage_data.emplace_back(composer_type);
    preimage_data.emplace_back(compressed_domain);
    preimage_data.emplace_back(num_public_inputs);
    for (const auto& [tag, selector] : commitments) {
        const auto x_limbs = split_bigfield_limbs(selector.x);
        const auto y_limbs = split_bigfield_limbs(selector.y);

        preimage_data.push_back(x_limbs[0]);
        preimage_data.push_back(x_limbs[1]);
        preimage_data.push_back(x_limbs[2]);
        preimage_data.push_back(x_limbs[3]);

        preimage_data.push_back(y_limbs[0]);
        preimage_data.push_back(y_limbs[1]);
        preimage_data.push_back(y_limbs[2]);
        preimage_data.push_back(y_limbs[3]);
    }

    barretenberg::fr compressed_key;
    if (proof_system::ComposerType(composer_type) == proof_system::ComposerType::PLOOKUP) {
        compressed_key = crypto::pedersen_commitment::lookup::compress_native(preimage_data, hash_index);
    } else {
        compressed_key = crypto::pedersen_commitment::compress_native(preimage_data, hash_index);
    }
    return compressed_key;
}

verification_key::verification_key(const size_t num_gates,
                                   const size_t num_inputs,
                                   std::shared_ptr<VerifierReferenceString> const& crs,
                                   uint32_t composer_type_)
    : composer_type(composer_type_)
    , circuit_size(num_gates)
    , log_circuit_size(numeric::get_msb(num_gates))
    , num_public_inputs(num_inputs)
    , domain(circuit_size)
    , reference_string(crs)
    , polynomial_manifest(composer_type)
{}

verification_key::verification_key(const std::vector<barretenberg::fr>& key_as_fields,
                                   std::shared_ptr<VerifierReferenceString> const& crs,
                                   uint32_t composer_type_)
{
    composer_type = composer_type_;
    polynomial_manifest = PolynomialManifest(composer_type);
    reference_string = crs;
    // std::cout << "key as field = " << key_as_fields[3] << std::endl;
    // for (const auto& f : key_as_fields) {
    //     std::cout << "f : " << f << std::endl;
    // }
    circuit_size = static_cast<size_t>(uint256_t(key_as_fields[3]));

    std::cout << "circuit size = " << circuit_size << std::endl;

    log_circuit_size = numeric::get_msb(circuit_size);
    num_public_inputs = static_cast<size_t>(uint256_t(key_as_fields[4]));
    contains_recursive_proof = static_cast<size_t>(uint256_t(key_as_fields[5]));
    domain = evaluation_domain(circuit_size);

    // ADD THE COMITMENTS DERP DERP DERP
    // AS WELL AS RECURSIVE PROOF PUBLIC INPUT INDICES?
    //
}

verification_key::verification_key(verification_key_data&& data, std::shared_ptr<VerifierReferenceString> const& crs)
    : composer_type(data.composer_type)
    , circuit_size(data.circuit_size)
    , log_circuit_size(numeric::get_msb(data.circuit_size))
    , num_public_inputs(data.num_public_inputs)
    , domain(circuit_size)
    , reference_string(crs)
    , commitments(std::move(data.commitments))
    , polynomial_manifest(data.composer_type)
    , contains_recursive_proof(data.contains_recursive_proof)
    , recursive_proof_public_input_indices(std::move(data.recursive_proof_public_input_indices))
{}

verification_key::verification_key(const verification_key& other)
    : composer_type(other.composer_type)
    , circuit_size(other.circuit_size)
    , log_circuit_size(numeric::get_msb(other.circuit_size))
    , num_public_inputs(other.num_public_inputs)
    , domain(other.domain)
    , reference_string(other.reference_string)
    , commitments(other.commitments)
    , polynomial_manifest(other.polynomial_manifest)
    , contains_recursive_proof(other.contains_recursive_proof)
    , recursive_proof_public_input_indices(other.recursive_proof_public_input_indices)
{}

verification_key::verification_key(verification_key&& other)
    : composer_type(other.composer_type)
    , circuit_size(other.circuit_size)
    , log_circuit_size(numeric::get_msb(other.circuit_size))
    , num_public_inputs(other.num_public_inputs)
    , domain(other.domain)
    , reference_string(other.reference_string)
    , commitments(other.commitments)
    , polynomial_manifest(other.polynomial_manifest)
    , contains_recursive_proof(other.contains_recursive_proof)
    , recursive_proof_public_input_indices(other.recursive_proof_public_input_indices)
{}

verification_key& verification_key::operator=(verification_key&& other)
{
    composer_type = other.composer_type;
    circuit_size = other.circuit_size;
    log_circuit_size = numeric::get_msb(other.circuit_size);
    num_public_inputs = other.num_public_inputs;
    reference_string = std::move(other.reference_string);
    commitments = std::move(other.commitments);
    polynomial_manifest = std::move(other.polynomial_manifest);
    domain = std::move(other.domain);
    contains_recursive_proof = (other.contains_recursive_proof);
    recursive_proof_public_input_indices = std::move(other.recursive_proof_public_input_indices);
    return *this;
}

sha256::hash verification_key::sha256_hash()
{
    std::vector<uint256_t> vk_data;
    vk_data.emplace_back(composer_type);
    vk_data.emplace_back(circuit_size);
    vk_data.emplace_back(num_public_inputs);
    for (auto& commitment_entry : commitments) {
        vk_data.emplace_back(commitment_entry.second.x);
        vk_data.emplace_back(commitment_entry.second.y);
    }
    vk_data.emplace_back(contains_recursive_proof);
    for (auto& index : recursive_proof_public_input_indices) {
        vk_data.emplace_back(index);
    }
    return sha256::sha256(to_buffer(vk_data));
}

std::vector<barretenberg::fr> verification_key::export_transcript_in_recursion_format()
{
    std::vector<fr> output;
    output.emplace_back(domain.root);
    output.emplace_back(domain.domain);
    output.emplace_back(domain.generator);
    output.emplace_back(circuit_size);
    output.emplace_back(num_public_inputs);
    output.emplace_back(contains_recursive_proof);

    for (auto& commitment_entry : commitments) {
        std::cout << "key = " << commitment_entry.first << std::endl;
        std::cout << "value = " << commitment_entry.second << std::endl;
    }
    for (const auto& descriptor : polynomial_manifest.get()) {
        if (descriptor.source == PolynomialSource::SELECTOR || descriptor.source == PolynomialSource::PERMUTATION) {
            std::cout << "key =  " << descriptor.commitment_label << std::endl;
            const auto element = commitments.at(std::string(descriptor.commitment_label));
            std::cout << "read" << std::endl;
            const uint256_t x = element.x;
            const uint256_t y = element.y;
            const barretenberg::fr x_lo = x.slice(0, 136);
            const barretenberg::fr x_hi = x.slice(136, 272);
            const barretenberg::fr y_lo = y.slice(0, 136);
            const barretenberg::fr y_hi = y.slice(136, 272);
            output.emplace_back(x_lo);
            output.emplace_back(x_hi);
            output.emplace_back(y_lo);
            output.emplace_back(y_hi);
        }
    }

    return output;
}

} // namespace proof_system::plonk
