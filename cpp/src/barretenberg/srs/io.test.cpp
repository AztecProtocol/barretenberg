#include "barretenberg/ecc/curves/bn254/fq12.hpp"
#include "barretenberg/ecc/curves/bn254/pairing.hpp"
#include "io.hpp"
#include "barretenberg/common/mem.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include "msgpack.hpp"
#include "msgpack-impl.hpp"
#include "barretenberg/crypto/aes128/aes128.hpp"

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

struct MyExampleFlat {
    int a;
    std::string b;

    void msgpack_flat(auto ar) { ar(a, b); }
};

struct MyExampleMap {
    int a;
    std::string b;
    MyExampleFlat flat;

    void msgpack(auto ar) { ar(NVP(a), NVP(b), NVP(flat)); }
};

namespace cbinds {
struct aes__decrypt_buffer_cbc {
    std::vector<uint8_t> in;
    std::vector<uint8_t> iv;
    std::vector<uint8_t> key;
    size_t length;
    void msgpack_flat(auto ar) { ar(in, iv, key, length); }
    auto operator()()
    {
        crypto::aes128::decrypt_buffer_cbc(in.data(), iv.data(), key.data(), length);
        return in;
    }
};
} // namespace cbinds

// auto aes__decrypt_buffer_cbc(uint8_t* in, uint8_t* iv, const uint8_t* key, const size_t length, uint8_t* r) {}
// auto cbind_example() {}

void pretty_print(const auto& obj)
{
    std::stringstream output;
    msgpack::pack(output, obj);
    std::string output_str = output.str();
    msgpack::object_handle oh = msgpack::unpack(output_str.data(), output_str.size());
    std::cout << oh.get() << std::endl;
}

TEST(io, myexample)
{

    { // pack, unpack
        MyExampleMap my{ 1, "2", { 3, "4" } };
        MsgPackSchema<MyExampleMap> my_schema;
        pretty_print(my);
        pretty_print(my_schema);
        std::stringstream ss;
        msgpack::pack(ss, my);

        std::string const& str = ss.str();
        // Write the packed data to a file
        std::ofstream ofs("output.msgpack", std::ios::binary);
        if (ofs) {
            ofs.write(str.data(), (std::streamsize)str.size());
            ofs.close();
            std::cout << "Binary string written to output.msgpack" << std::endl;
        } else {
            std::cerr << "Error: Unable to open output.msgpack" << std::endl;
        }
        msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
        msgpack::object obj = oh.get();
        std::cout << obj << std::endl;
        MyExampleMap map = obj.convert();
        std::cout << map.b << std::endl;
    }
}