// AUTOGENERATED FILE
#pragma once

#include <string_view>

#include "barretenberg/relations/relation_parameters.hpp"
#include "barretenberg/relations/relation_types.hpp"

namespace bb::avm2 {

template <typename FF_> class address_derivationImpl {
  public:
    using FF = FF_;

    static constexpr std::array<size_t, 6> SUBRELATION_PARTIAL_LENGTHS = { 3, 3, 3, 3, 3, 3 };

    template <typename AllEntities> inline static bool skip(const AllEntities& in)
    {
        const auto& new_term = in;
        return (new_term.address_derivation_sel).is_zero();
    }

    template <typename ContainerOverSubrelations, typename AllEntities>
    void static accumulate(ContainerOverSubrelations& evals,
                           const AllEntities& new_term,
                           [[maybe_unused]] const RelationParameters<FF>&,
                           [[maybe_unused]] const FF& scaling_factor)
    {
        const auto constants_GRUMPKIN_ONE_X = FF(1);
        const auto constants_GRUMPKIN_ONE_Y =
            FF(uint256_t{ 9457493854555940652UL, 3253583849847263892UL, 14921373847124204899UL, 2UL });
        const auto constants_GENERATOR_INDEX__CONTRACT_ADDRESS_V1 = FF(15);
        const auto constants_GENERATOR_INDEX__PARTIAL_ADDRESS = FF(27);
        const auto constants_GENERATOR_INDEX__PUBLIC_KEYS_HASH = FF(52);

        {
            using Accumulator = typename std::tuple_element_t<0, ContainerOverSubrelations>;
            auto tmp = new_term.address_derivation_sel * (FF(1) - new_term.address_derivation_sel);
            tmp *= scaling_factor;
            std::get<0>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<1, ContainerOverSubrelations>;
            auto tmp = new_term.address_derivation_sel * (new_term.address_derivation_partial_address_domain_separator -
                                                          constants_GENERATOR_INDEX__PARTIAL_ADDRESS);
            tmp *= scaling_factor;
            std::get<1>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<2, ContainerOverSubrelations>;
            auto tmp =
                new_term.address_derivation_sel * (new_term.address_derivation_public_keys_hash_domain_separator -
                                                   constants_GENERATOR_INDEX__PUBLIC_KEYS_HASH);
            tmp *= scaling_factor;
            std::get<2>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<3, ContainerOverSubrelations>;
            auto tmp = new_term.address_derivation_sel * (new_term.address_derivation_preaddress_domain_separator -
                                                          constants_GENERATOR_INDEX__CONTRACT_ADDRESS_V1);
            tmp *= scaling_factor;
            std::get<3>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<4, ContainerOverSubrelations>;
            auto tmp = new_term.address_derivation_sel * (new_term.address_derivation_g1_x - constants_GRUMPKIN_ONE_X);
            tmp *= scaling_factor;
            std::get<4>(evals) += typename Accumulator::View(tmp);
        }
        {
            using Accumulator = typename std::tuple_element_t<5, ContainerOverSubrelations>;
            auto tmp = new_term.address_derivation_sel * (new_term.address_derivation_g1_y - constants_GRUMPKIN_ONE_Y);
            tmp *= scaling_factor;
            std::get<5>(evals) += typename Accumulator::View(tmp);
        }
    }
};

template <typename FF> class address_derivation : public Relation<address_derivationImpl<FF>> {
  public:
    static constexpr const std::string_view NAME = "address_derivation";

    static std::string get_subrelation_label(size_t index)
    {
        switch (index) {}
        return std::to_string(index);
    }
};

} // namespace bb::avm2
