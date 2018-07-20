#include "type_value_lookup.t++"
#include <iostream>

template<typename T, auto V>
using tvp = vanwestco::type_value_pair<T, V>;

int main() {
    int i = vanwestco::lookup<decltype(i), tvp<float, 1>,
                                   tvp<short int, 2>,
                                   tvp<void*, 3>,
                                   tvp<char*, 4>,
                                   tvp<double, 5>,
                                   tvp<char, 6>,
                                   tvp<float, 1>,
                                   tvp<short int, 2>,
                                   tvp<void*, 3>,
                                   tvp<char*, 4>,
                                   tvp<double, 5>,
                                   tvp<char, 6>,
                                   tvp<int, 7>>::value;
    std::cout << i << std::endl;
    return 0;
}
