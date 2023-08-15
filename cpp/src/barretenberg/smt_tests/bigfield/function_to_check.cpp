#include "../check_sound_circ.hpp"

cvc5::Term func(std::vector<cvc5::Term>& inputs, cvc5::Solver& s, cvc5::Sort& fp)
{
    cvc5::Term lmb = s.mkFiniteFieldElem("100000000000000000", fp, 16);
    cvc5::Term lmb2 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { lmb, lmb });
    cvc5::Term lmb3 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { lmb2, lmb });

    cvc5::Term tmp_al1 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { inputs[1], lmb });
    cvc5::Term tmp_al2 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { inputs[2], lmb2 });
    cvc5::Term tmp_al3 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { inputs[3], lmb3 });
    cvc5::Term a_r = s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { inputs[0], tmp_al1, tmp_al2, tmp_al3 });

    cvc5::Term tmp_bl1 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { inputs[5], lmb });
    cvc5::Term tmp_bl2 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { inputs[6], lmb2 });
    cvc5::Term tmp_bl3 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { inputs[7], lmb3 });
    cvc5::Term b_r = s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { inputs[4], tmp_bl1, tmp_bl2, tmp_bl3 });

    cvc5::Term tmp_cl1 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { inputs[9], lmb });
    cvc5::Term tmp_cl2 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { inputs[10], lmb2 });
    cvc5::Term tmp_cl3 = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { inputs[11], lmb3 });
    cvc5::Term c_r = s.mkTerm(cvc5::Kind::FINITE_FIELD_ADD, { inputs[8], tmp_cl1, tmp_cl2, tmp_cl3 });

    cvc5::Term mul = s.mkTerm(cvc5::Kind::FINITE_FIELD_MULT, { a_r, b_r });

    cvc5::Term ret = s.mkTerm(cvc5::Kind::EQUAL, { c_r, mul });
    ret = s.mkTerm(cvc5::Kind::EQUAL, { ret, s.mkBoolean(0) });
    s.assertFormula(ret);
    return mul;
}