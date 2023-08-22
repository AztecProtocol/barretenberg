#include "ffterm.hpp"

namespace smt_terms {

FFTerm Var(const std::string& name, Solver* slv)
{
    return FFTerm(name, slv);
};
FFTerm Const(const std::string& val, Solver* slv, uint32_t base)
{
    return FFTerm(val, slv, true, base);
};

FFTerm::FFTerm(const std::string& t, Solver* slv, bool isconst, uint32_t base)
    : solver(slv)
    , isconst(isconst)
{
    if (!isconst) {
        this->term = slv->s.mkConst(slv->fp, t);
    } else {
        this->term = slv->s.mkFiniteFieldElem(t, slv->fp, base);
    }
}

FFTerm FFTerm::operator+(const FFTerm& other) const
{
    cvc5::Term res = this->solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { this->term, other.term });
    ;
    return { res, this->solver };
}

void FFTerm::operator+=(const FFTerm& other)
{
    this->term = this->solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { this->term, other.term });
}

FFTerm FFTerm::operator-(const FFTerm& other) const
{
    cvc5::Term res = this->solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_NEG, { other.term });
    res = solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { this->term, res });
    return { res, this->solver };
}

void FFTerm::operator-=(const FFTerm& other)
{
    cvc5::Term tmp_term = this->solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_NEG, { other.term });
    this->term = this->solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { this->term, tmp_term });
}

FFTerm FFTerm::operator*(const FFTerm& other) const
{
    cvc5::Term res = solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { this->term, other.term });
    return { res, this->solver };
}

void FFTerm::operator*=(const FFTerm& other)
{
    this->term = this->solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { this->term, other.term });
}

FFTerm FFTerm::operator/(const FFTerm& other) const
{
    cvc5::Term res = this->solver->s.mkConst(this->solver->fp,
                                             "fe0f65a52067384116dc1137d798e0ca00a7ed46950e4eab7db51e08481535f2_div_" +
                                                 std::string(*this) + "_" + std::string(other));
    cvc5::Term div = this->solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { res, other.term });
    cvc5::Term eq = this->solver->s.mkTerm(cvc5::Kind::EQUAL, { this->term, div });
    this->solver->s.assertFormula(eq);
    return { res, this->solver };
}

void FFTerm::operator/=(const FFTerm& other)
{
    cvc5::Term res = this->solver->s.mkConst(this->solver->fp,
                                             "fe0f65a52067384116dc1137d798e0ca00a7ed46950e4eab7db51e08481535f2_div_" +
                                                 std::string(*this) + "__" + std::string(other));
    cvc5::Term div = this->solver->s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { res, other.term });
    cvc5::Term eq = this->solver->s.mkTerm(cvc5::Kind::EQUAL, { this->term, div });
    this->solver->s.assertFormula(eq);
    this->term = res;
}

void FFTerm::operator==(const FFTerm& other) const
{
    cvc5::Term eq = this->solver->s.mkTerm(cvc5::Kind::EQUAL, { this->term, other.term });
    this->solver->s.assertFormula(eq);
}

void FFTerm::operator!=(const FFTerm& other) const
{
    cvc5::Term eq = this->solver->s.mkTerm(cvc5::Kind::EQUAL, { this->term, other.term });
    eq = this->solver->s.mkTerm(cvc5::Kind::EQUAL, { eq, this->solver->s.mkBoolean(false) });
    this->solver->s.assertFormula(eq);
}

Bool Bool::operator|(const Bool& other) const
{
    cvc5::Term res = solver->mkTerm(cvc5::Kind::OR, { this->term, other.term });
    ;
    return { res, this->solver };
}

void Bool::operator|=(const Bool& other)
{
    this->term = this->solver->mkTerm(cvc5::Kind::OR, { this->term, other.term });
}

Bool Bool::operator&(const Bool& other) const
{
    cvc5::Term res = solver->mkTerm(cvc5::Kind::AND, { this->term, other.term });
    return { res, this->solver };
}

void Bool::operator&=(const Bool& other)
{
    this->term = this->solver->mkTerm(cvc5::Kind::AND, { this->term, other.term });
}

Bool Bool::operator==(const Bool& other) const
{
    cvc5::Term res = solver->mkTerm(cvc5::Kind::EQUAL, { this->term, other.term });
    return { res, this->solver };
}

Bool Bool::operator!=(const Bool& other) const
{
    cvc5::Term res = solver->mkTerm(cvc5::Kind::EQUAL, { this->term, other.term });
    res = solver->mkTerm(cvc5::Kind::EQUAL, { res, this->solver->mkBoolean(false) });
    return { res, this->solver };
}
}; // namespace smt_terms