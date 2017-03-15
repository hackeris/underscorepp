#include <iostream>
#include "underscore.hpp"

void test_each() {

    std::cout << "Testing each..." << std::endl;

    std::vector<int> a{1, 2, 3, 4, 5, 6};
    _::each(a, [](const int &item) {
        std::cout << item << " ";
    });
    std::cout << std::endl;

    std::cout << "OK." << std::endl;
}

void test_map() {

    std::cout << "Testing map..." << std::endl;

    std::vector<int> a{1, 2, 3, 4, 5, 6};

    using result_type = std::vector<int>;
    auto a2 = _::map < result_type > (a, [](int item) -> int {
        return item * 2;
    });

    _::each(a2, [](const int &item) {
        std::cout << item << " ";
    });
    std::cout << std::endl;

    std::cout << "OK." << std::endl;
}

void test_filter() {

    std::cout << "Testing filter..." << std::endl;

    std::vector<int> a{1, 2, 3, 4, 5, 6};
    auto od = _::filter(a, [](int item) -> bool {
        return item % 2 == 0;
    });

    _::each(od, [](const int &item) {
        std::cout << item << " ";
    });
    std::cout << std::endl;

    std::cout << "OK." << std::endl;
}

void test_group() {

    std::cout << "Testing group..." << std::endl;

    std::vector<int> a{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    auto result = _::group<int>(a, [](const int &item) -> int {
        return item % 3;
    });

    std::cout << "OK." << std::endl;
}

int main() {

    test_each();
    test_map();
    test_filter();
    test_group();

    return 0;
}
