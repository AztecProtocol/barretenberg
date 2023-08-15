#include "../check_sound_circ.hpp"

cvc5::Term func(std::vector<cvc5::Term>& inputs, cvc5::Solver& s, cvc5::Sort& fp)
{
    size_t coeffs_len = inputs.size() - 2;
    cvc5::Term point = inputs[coeffs_len];
    cvc5::Term result = inputs[coeffs_len + 1];
    cvc5::Term ev = s.mkFiniteFieldElem("0", fp);

    for (size_t i = 0; i < coeffs_len; i++) {
        cvc5::Term tmp = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { ev, point });
        ev = s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { tmp, inputs[i] });
    }

    cvc5::Term ret = s.mkTerm(cvc5::Kind::EQUAL, { ev, result });
    // ret = s.mkTerm(cvc5::Kind::EQUAL, {ret, s.mkBoolean(0)});
    s.assertFormula(ret);
    return ev;
}

// cvc5::Term func_big(std::vector<cvc5::Term>& inputs, cvc5::Solver& s, cvc5::Sort& fp){
//
//}