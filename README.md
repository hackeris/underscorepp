# underscorepp

underscore.js like library for C++.

## features

* each
* map
* filter
* group
* reduce
* findFirst
* flatten

###paralleled

* each
* map
* filter
* group
* flatten

## example

```c++

#include "underscore.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <cassert>

int main() {

    int n = 100000000;
    std::vector<int> a;
    for (int i = 1; i <= n; i++) {
        a.push_back(i);
    }
    // ---------- test map --------------
    std::cout << "Test map" << std::endl;

    using result_type = std::vector<int>;
    auto a2 = _::map < result_type > (a, [](int item) -> int {
        return item * 2;
    });

    std::cout << "Validating..." << std::endl;

    for (size_t i = 0; i < a.size(); i++) {
        assert(a2[i] == a[i] * 2);
    }
    std::cout << "OK" << std::endl;

    // ---------- test parallel map --------------
    std::cout << "Testing parallel map..." << std::endl;

    using result_type = std::vector<int>;
    auto a4 = _::parallel::map<result_type>(a, [](const int &item) -> int {
        return item * 2;
    });
    std::cout << "Validating..." << std::endl;
    for (size_t i = 0; i < a4.size(); i++) {
        assert(a4[i] == a[i] * 2);
    }
    std::cout << "OK..." << std::endl;

    return 0;
}

```
