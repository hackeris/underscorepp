# underscorepp

[underscore.js ](http://underscorejs.org/) like library for C++.

## Features

* each
* map
* filter
* group
* reduce
* flatten

### Paralleled

* each
* map
* filter
* group
* flatten

### Chain

* chain (with serial and parallel strategy)

## Example

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
    
    // ---------- test parallel chain --------------
    std::vector<std::vector<int>> a{
                    {1, 2, 3, 4, 5, 6, 7, 8},
                    {2, 3, 4, 5, 6, 7, 8, 9},
                    {3, 4, 5, 6, 7, 8, 9, 10}
                };
    auto result = _::chain<_::Parallel>(a)
            .flatten<std::vector<int>>()
            .reduce([](int memo, int item) -> int { return memo + item; }, 0)
            .value();
    std::cout << result << std::endl;
    std::cout << "OK." << std::endl;
    
    return 0;
}

```
