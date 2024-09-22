#include "adder/sample.hpp"

#include "private_sample.hpp"

namespace autoshinyhunthgss {

int add_positive_numbers(int a, int b)
{
    bool is_a_positive = is_integer_positive(a);
    bool is_b_positive = is_integer_positive(b);

    if (is_a_positive && is_b_positive)
    {
        return a + b;
    }

    return -1; // error
}

} // namespace autoshinyhunthgss