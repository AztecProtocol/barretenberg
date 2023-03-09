#include <cstddef>
#include <gtest/gtest.h>

#include "polynomial_container.hpp"
#include "barretenberg/polynomials/polynomial.hpp"

namespace bonk {

using namespace barretenberg;
using Fr = barretenberg::fr;
using Polynomial = barretenberg::Polynomial<Fr>;

// Test basic put and get functionality
TEST(PolynomialContainer, PutThenGet)
{
    PolynomialContainer<Fr> polynomial_container;

    // Instantiate a polynomial with random coefficients
    Polynomial poly(1024);
    for (auto& coeff : poly) {
        coeff = Fr::random_element();
    }

    // Make a copy for comparison after original is moved into container
    Polynomial poly_copy(poly);

    // Move the poly into the container
    polynomial_container.put("id", std::move(poly));

    // Confirm equality of the copy and the original poly that now resides in the container
    EXPECT_EQ(poly_copy, polynomial_container.get("id"));
}

// Ensure that attempt to access non-existent key throws out_of_range exception
TEST(PolynomialContainer, NonexistentKey)
{
    PolynomialContainer<Fr> polynomial_container;

    polynomial_container.put("id_1", Polynomial(100));

    polynomial_container.get("id_1"); // no problem!

    EXPECT_THROW(polynomial_container.get("id_2"), std::out_of_range);
}

// Ensure correct calculation of volume in bytes
TEST(PolynomialContainer, Volume)
{
    PolynomialContainer<Fr> polynomial_container;
    size_t size1 = 100;
    size_t size2 = 10;
    size_t size3 = 5000;

    Polynomial poly1(size1);
    Polynomial poly2(size2);
    Polynomial poly3(size3);

    polynomial_container.put("id_1", std::move(poly1));
    polynomial_container.put("id_2", std::move(poly2));
    polynomial_container.put("id_3", std::move(poly3));

    // polynomial_container.print();

    size_t bytes_expected = sizeof(Fr) * (size1 + size2 + size3);

    EXPECT_EQ(polynomial_container.get_size_in_bytes(), bytes_expected);
}

// Ensure that the remove method erases entry and reduces memory
TEST(PolynomialContainer, Remove)
{
    PolynomialContainer<Fr> polynomial_container;
    size_t size1 = 100;
    size_t size2 = 500;
    Polynomial poly1(size1);
    Polynomial poly2(size2);

    polynomial_container.put("id_1", std::move(poly1));
    polynomial_container.put("id_2", std::move(poly2));

    size_t bytes_expected = sizeof(Fr) * (size1 + size2);

    EXPECT_EQ(polynomial_container.get_size_in_bytes(), bytes_expected);

    polynomial_container.remove("id_1");

    bytes_expected -= sizeof(Fr) * size1;

    EXPECT_THROW(polynomial_container.get("id_1"), std::out_of_range);
    EXPECT_EQ(polynomial_container.get_size_in_bytes(), bytes_expected);
}

} // namespace bonk
