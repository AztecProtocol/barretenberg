#include "barretenberg/ecc/curves/bn254/fq12.hpp"
#include "barretenberg/ecc/curves/bn254/pairing.hpp"
#include "io.hpp"
#include "barretenberg/common/mem.hpp"
#include <gtest/gtest.h>
#include "msgpack.hpp"
#include "msgpack-impl.hpp"

using namespace barretenberg;

TEST(io, read_transcript_loads_well_formed_srs)
{
    size_t degree = 100000;
    g1::affine_element* monomials = (g1::affine_element*)(aligned_alloc(32, sizeof(g1::affine_element) * (degree + 2)));
    g2::affine_element g2_x;
    io::read_transcript(monomials, g2_x, degree, "../srs_db/ignition");

    EXPECT_EQ(g1::affine_one, monomials[0]);

    g1::affine_element P[2];
    g2::affine_element Q[2];
    P[0] = monomials[1];
    P[1] = g1::affine_one;
    P[0].y.self_neg();
    Q[0] = g2::affine_one;
    Q[1] = g2_x;
    fq12 res = pairing::reduced_ate_pairing_batch(P, Q, 2);

    EXPECT_EQ(res, fq12::one());
    for (size_t i = 0; i < degree; ++i) {
        EXPECT_EQ(monomials[i].on_curve(), true);
    }
    aligned_free(monomials);
}

struct MyExampleMap {
    int a;
    std::string b;

    auto serialize_map(const auto& ar) { return ar(NVP(a), NVP(b)); }
};

struct MyExampleFlat {
    int a;
    std::string b;

    auto serialize(const auto& ar) { return ar(a, b); }
};

TEST(io, myexample)
{

    { // pack, unpack
        MyExampleMap my{ 42, "Hello" };
        std::stringstream ss;
        msgpack::pack(ss, my);

        std::string const& str = ss.str();
        msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
        msgpack::object obj = oh.get();
        std::cout << obj << std::endl;
        //        assert(obj.as<MyExampleMap>() == my);
    }
    { // create object with zone
        MyExampleMap my{ 42, "Hello" };
        msgpack::zone z;
        msgpack::object obj(my, z);
        std::cout << obj << std::endl;
        //        assert(obj.as<MyExampleMap>() == my);
    }
}