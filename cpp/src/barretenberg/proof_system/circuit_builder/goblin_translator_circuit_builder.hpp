#pragma once
/**
 * @file goblin_translator_builder.hpp
 * @author @Rumata888
 * @brief Circuit Logic generation for Goblin Plonk translator (checks equivalence of Queues/Transcripts for ECCVM and
 * Recursive Circuits)
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "barretenberg/proof_system/arithmetization/arithmetization.hpp"
#include "circuit_builder_base.hpp"
namespace proof_system {
class GoblinTranslatorCircuitBuilder : CircuitBuilderBase<arithmetization::GoblinTranslator> {
  public:
    static constexpr std::string_view NAME_STRING = "GoblinTranslatorArithmetization";
    // TODO(kesha): fix size hints
    GoblinTranslatorCircuitBuilder()
        : CircuitBuilderBase({}, 0){};
    GoblinTranslatorCircuitBuilder(const GoblinTranslatorCircuitBuilder& other) = delete;
    GoblinTranslatorCircuitBuilder(GoblinTranslatorCircuitBuilder&& other) noexcept
        : CircuitBuilderBase(std::move(other)){};
    GoblinTranslatorCircuitBuilder& operator=(const GoblinTranslatorCircuitBuilder& other) = delete;
    GoblinTranslatorCircuitBuilder& operator=(GoblinTranslatorCircuitBuilder&& other) noexcept
    {
        CircuitBuilderBase::operator=(std::move(other));
        return *this;
    };
    ~GoblinTranslatorCircuitBuilder() override = default;
};
} // namespace proof_system