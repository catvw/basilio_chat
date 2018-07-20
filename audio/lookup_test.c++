#include "type_value_lookup.t++"
#include <iostream>

template<typename T, auto V>
using tvp = vanwestco::type_value_pair<T, V>;

template<typename T>
using lookup = vanwestco::lookup<
    T, 
    tvp<float,     1>,
    tvp<short int, 2>,
    tvp<void*,     3>,
    tvp<char,      4>,
    tvp<double,    5>,
    tvp<char*,     6>,
    tvp<float*,    7>,
    tvp<short int, 8>,
    tvp<void,      9>,
    tvp<char*,    10>,
    tvp<double,   11>,
    tvp<char,     12>,
    tvp<int,      13>, /* right here */
    tvp<char**,   14>,
    tvp<char***,  15>
>;

int main() {
    int i = lookup<decltype(i)>::value;
    std::cout << i << std::endl;
    return 0;
}
