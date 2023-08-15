#include "barretenberg/serialize/cbind.hpp"
#include "barretenberg/serialize/msgpack.hpp"

#include <fstream>

#include <gtest/gtest.h>
#include <unordered_map>

struct GoodExample {
    barretenberg::fr a;
    barretenberg::fr b;
    MSGPACK_FIELDS(a, b);
} good_example;

struct ComplicatedSchema {
    std::vector<std::array<barretenberg::fr, 20>> array;
    std::optional<GoodExample> good_or_not;
    barretenberg::fr bare;
    std::variant<barretenberg::fr, GoodExample> huh;
    MSGPACK_FIELDS(array, good_or_not, bare, huh);
} complicated_schema;

TEST(msgpack_tests, msgpack_schema_sanity)
{
    info(msgpack_schema_to_string(good_example));
    info(msgpack_schema_to_string(complicated_schema));
}

TEST(msgpack_tests, serialize_struct)
{
    GoodExample obj;
    obj.a = barretenberg::fr::random_element();
    obj.b = barretenberg::fr::random_element();
    std::cout << obj.a << std::endl;
    std::cout << obj.b << std::endl;

    // Create a buffer to store the encoded data
    msgpack::sbuffer buffer;
    msgpack::pack(buffer, obj);

    uint8_t* output = (uint8_t*)aligned_alloc(64, buffer.size());
    memcpy(output, buffer.data(), buffer.size());
    // Convert the buffer data to a string and return it
    std::cout << output << std::endl;

    GoodExample obj1;
    msgpack::unpack((const char*)output, buffer.size()).get().convert(obj1);
    std::cout << obj1.a << std::endl;
    std::cout << obj1.b << std::endl;
}

struct CircuitSchema {
    std::vector<uint32_t> public_inps;
    std::unordered_map<uint32_t, std::string> vars_of_interest;
    std::vector<barretenberg::fr> variables;
    std::vector<std::vector<barretenberg::fr>> selectors;
    std::vector<std::vector<uint32_t>> wits;
    MSGPACK_FIELDS(public_inps, vars_of_interest, variables, selectors, wits);
};

#include <limits>
CircuitSchema unpack(const std::string& filename)
{
    std::ifstream fin;
    fin.open(filename, std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        throw std::invalid_argument("file not found");
    }
    if (fin.tellg() == -1) {
        throw std::invalid_argument("something went wrong");
    }

    fin.ignore(std::numeric_limits<std::streamsize>::max()); // ohboy
    std::streamsize fsize = fin.gcount();
    fin.clear();
    fin.seekg(0, std::ios_base::beg);
    info("File size: ", fsize);

    CircuitSchema cir;
    char* encoded_data = (char*)aligned_alloc(64, static_cast<size_t>(fsize));
    fin.read(encoded_data, fsize);
    msgpack::unpack((const char*)encoded_data, static_cast<size_t>(fsize)).get().convert(cir);

    return cir;
}

TEST(msgpack_tests, unpack)
{
    CircuitSchema cirtest;
    cirtest.public_inps = { 1, 2, 3 };
    cirtest.vars_of_interest.insert({ 1, "aboba" });
    cirtest.vars_of_interest.insert({ 2, "check" });
    cirtest.variables = { barretenberg::fr::one() };
    cirtest.selectors.push_back({ barretenberg::fr::one(), barretenberg::fr::zero() });
    cirtest.wits.push_back({ 1, 2, 3 });

    std::fstream myfile;

    myfile.open("strict.pack", std::ios::out | std::ios::trunc | std::ios::binary);
    msgpack::sbuffer buffer;
    msgpack::pack(buffer, cirtest);
    info("Buffer size: ", buffer.size());
    myfile.write(buffer.data(), static_cast<long>(buffer.size()));
    myfile.close();

    CircuitSchema cirunpack = unpack("strict.pack");
    ASSERT_EQ(cirtest.public_inps, cirunpack.public_inps);
    ASSERT_EQ(cirtest.vars_of_interest, cirunpack.vars_of_interest);
    ASSERT_EQ(cirtest.variables, cirunpack.variables);
    ASSERT_EQ(cirtest.selectors, cirunpack.selectors);
    ASSERT_EQ(cirtest.wits, cirunpack.wits);
}