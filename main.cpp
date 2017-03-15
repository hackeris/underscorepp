#include <iostream>
#include <vector>
#include <cassert>
#include "underscore.hpp"

namespace test {
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

        using item_type = decltype(result)::value_type;
        _::each(result, [](const item_type &item) {
            std::cout << item.first << ": ";
            _::each(item.second, [](const int &elem) {
                std::cout << elem << " ";
            });
            std::cout << std::endl;
        });

        std::cout << "OK." << std::endl;
    }

    void test_reduce() {

        std::cout << "Testing reduce..." << std::endl;

        std::vector<int> a{1, 2, 3, 4, 5, 6, 7, 8};
        auto result = _::reduce<int>(a, [](const int &memo, const int &item) -> int {
            return memo + item;
        }, 0);

        std::cout << result << std::endl;

        std::cout << "OK." << std::endl;
    }

    void test_reduce_right() {

        std::cout << "Testing reduce right..." << std::endl;

        std::vector<int> a{1, 2, 3, 4, 5, 6, 7, 8};
        auto result = _::reduceRight<int>(a, [](const int &item, const int &memo) -> int {
            return memo + item;
        }, 0);

        std::cout << result << std::endl;

        std::cout << "OK." << std::endl;
    }

    void test_underscore() {

        test_each();
        test_map();
        test_filter();
        test_group();
        test_reduce();
        test_reduce_right();
    }
}

namespace test {
    namespace parallel {

        void test_each() {

            std::cout << "Testing parallel each..." << std::endl;

            int n = 100000;
            std::vector<int> a;
            for (int i = 1; i <= n; i++) {
                a.push_back(i);
            }
            std::atomic<int> sum{0};

            _::parallel::each(a, [&sum](const int &item) {
                sum.fetch_add(item);
            });
            assert(sum == n * (n + 1) / 2);

            std::cout << "OK..." << std::endl;
        }

        void test_map() {
            std::cout << "Testing parallel map..." << std::endl;

            int n = 100000;
            std::vector<int> a;
            for (int i = 1; i <= n; i++) {
                a.push_back(i);
            }

            using result_type = std::vector<int>;
            auto a2 = _::parallel::map<result_type>(a, [](const int &item) -> int {
                return item * 2;
            });
            for (size_t i = 0; i < a2.size(); i++) {
                assert(a2[i] == a[i] * 2);
            }

            std::cout << "OK..." << std::endl;
        }

        void test_filter() {

        }

        void test_parallel_underscore() {
            test_each();
            test_map();
        }

    }
}

int main() {

    test::test_underscore();
    test::parallel::test_parallel_underscore();

    return 0;
}