#include <iostream>
#include <cassert>
#include "video/color.hpp"

// Helper function to compare floats with a small tolerance
bool approx_equal(float a, float b, float epsilon = 0.01f) {
    return std::fabs(a - b) < epsilon;
}

// Test Case 1: Check default constructor
void test_default_constructor() {
    Color c;
    assert(approx_equal(c.red, 0.0f));
    assert(approx_equal(c.green, 0.0f));
    assert(approx_equal(c.blue, 0.0f));
    assert(approx_equal(c.alpha, 1.0f));
}

// Test Case 2: Check parameterized constructor
void test_parameterized_constructor() {
    Color c(0.5f, 0.5f, 0.5f, 0.5f);
    assert(approx_equal(c.red, 0.5f));
    assert(approx_equal(c.green, 0.5f));
    assert(approx_equal(c.blue, 0.5f));
    assert(approx_equal(c.alpha, 0.5f));
}

// Test Case 3: Check equality operator
void test_equality_operator() {
    Color c1(0.1f, 0.2f, 0.3f, 1.0f);
    Color c2(0.1f, 0.2f, 0.3f, 1.0f);
    Color c3(0.3f, 0.2f, 0.1f, 1.0f);
    assert(c1 == c2);
    assert(!(c1 == c3));
}



// Test Case 4: Check color clamping in from_oklch
void test_from_oklch_clamping() {
    ColorOKLCh lch = {1.5f, 0.5f, 3.0f};  // Should clamp to [0,1]
    Color c = Color::from_oklch(lch);
    assert(c.red <= 1.0f && c.red >= 0.0f);
    assert(c.green <= 1.0f && c.green >= 0.0f);
    assert(c.blue <= 1.0f && c.blue >= 0.0f);
    assert(approx_equal(c.alpha, 1.0f));
}

// Test Case 5: Test hex serialization/deserialization
void test_serialize_deserialize_hex() {
    Color c(0.5f, 0.4f, 0.3f, 1.0f);
    std::string hex = Color::serialize_to_hex(c);
    auto parsed = Color::deserialize_from_hex(hex);
    assert(parsed.has_value());
    assert(approx_equal(parsed->red, c.red));
    assert(approx_equal(parsed->green, c.green));
    assert(approx_equal(parsed->blue, c.blue));
    assert(approx_equal(parsed->alpha, c.alpha));
}

int main() {
    // Run tests
    test_default_constructor();
    test_parameterized_constructor();
    test_equality_operator();
    test_from_oklch_clamping();
    test_serialize_deserialize_hex();

    std::cout << "All tests passed!\n";
    return 0;
}

/* EOF */
