#pragma once
#include "barretenberg/smt_verification/solver/solver.hpp"

namespace smt_terms {
using namespace smt_solver;

class FFTerm;
class Bool;

class FFTerm {
  private:
    Solver* solver;
    cvc5::Term term;
    bool isconst;

  public:
    FFTerm()
        : solver(nullptr)
        , term(cvc5::Term())
        , isconst(false){};
    explicit FFTerm(const std::string& t, Solver* slv, bool isconst = false, uint32_t base = 16);
    FFTerm(cvc5::Term& term, Solver* s)
        : solver(s)
        , term(term){};
    FFTerm(const FFTerm& other) = default;
    FFTerm(FFTerm&& other) = default;

    FFTerm& operator=(const FFTerm& right) = default;
    FFTerm& operator=(FFTerm&& right) = default;

    FFTerm operator+(const FFTerm& other) const;
    void operator+=(const FFTerm& other);
    FFTerm operator-(const FFTerm& other) const;
    void operator-=(const FFTerm& other);
    FFTerm operator*(const FFTerm& other) const;
    void operator*=(const FFTerm& other);
    FFTerm operator/(const FFTerm& other) const;
    void operator/=(const FFTerm& other);

    void operator==(const FFTerm& other) const;
    void operator!=(const FFTerm& other) const;

    // TODO(alex): Maybe do the same thing with +, - but I don't see a point
    // and also properlythink on how to implement this sh
    // void operator==(const std::pair<std::string, uint32_t>& other) const;
    // void operator!=(const std::pair<std::string, uint32_t>& other) const;

    operator std::string() const { return isconst ? term.getFiniteFieldValue() : term.toString(); };
    operator cvc5::Term() const { return term; };

    ~FFTerm() = default;
    friend std::ostream& operator<<(std::ostream& out, const FFTerm& k) { return out << k.term; }

    friend FFTerm batch_add(const std::vector<FFTerm>& children)
    {
        Solver* slv = children[0].solver;
        std::vector<cvc5::Term> terms(children.begin(), children.end());
        cvc5::Term res = slv->s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, terms);
        return { res, slv };
    }

    friend FFTerm batch_mul(const std::vector<FFTerm>& children)
    {
        Solver* slv = children[0].solver;
        std::vector<cvc5::Term> terms(children.begin(), children.end());
        cvc5::Term res = slv->s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, terms);
        return { res, slv };
    }

    friend class Bool;
};

FFTerm Var(const std::string& name, Solver* slv);
FFTerm Const(const std::string& val, Solver* slv, uint32_t base = 16);

class Bool { // TODO(alex) separate them into two files idkh
  private:
    cvc5::Solver* solver;
    cvc5::Term term;
    bool asserted = false;

  public:
    explicit Bool(const cvc5::Term& t, Solver& slv)
        : solver(&slv.s)
        , term(t){};
    explicit Bool(const FFTerm& t)
        : solver(&t.solver->s)
        , term(t.term){};

    explicit Bool(bool t, Solver& slv)
        : solver(&slv.s)
    {
        term = solver->mkBoolean(t);
    }
    Bool(const cvc5::Term& term, cvc5::Solver* s)
        : solver(s)
        , term(term){};
    Bool(const Bool& other) = default;
    Bool(Bool&& other) = default;

    Bool& operator=(const Bool& right) = default;
    Bool& operator=(Bool&& right) = default;

    void assert_term()
    {
        if (!asserted) {
            solver->assertFormula(term);
            asserted = true;
        }
    }

    Bool operator|(const Bool& other) const;
    void operator|=(const Bool& other);
    Bool operator|(const bool& other) const;
    void operator|=(const bool& other) const;

    Bool operator&(const Bool& other) const;
    void operator&=(const Bool& other);
    Bool operator&(const bool& other) const;
    void operator&=(const bool& other);

    Bool operator==(const Bool& other) const;
    Bool operator!=(const Bool& other) const;

    operator std::string() const { return term.toString(); };
    operator cvc5::Term() const { return term; };

    friend Bool batch_or(const std::vector<Bool>& children)
    {
        cvc5::Solver* s = children[0].solver;
        std::vector<cvc5::Term> terms(children.begin(), children.end());
        cvc5::Term res = s->mkTerm(cvc5::Kind::OR, terms);
        return { res, s };
    }

    friend Bool batch_and(const std::vector<Bool>& children)
    {
        cvc5::Solver* s = children[0].solver;
        std::vector<cvc5::Term> terms(children.begin(), children.end());
        cvc5::Term res = s->mkTerm(cvc5::Kind::AND, terms);
        return { res, s };
    }

    ~Bool() = default;
};
}; // namespace smt_terms