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

    void test_flatten() {

        std::cout << "Testing flatten..." << std::endl;

        std::vector<std::vector<int>> a{
                {1, 2, 3, 4, 5, 6, 7, 8},
                {1, 2, 3, 4, 5, 6, 7, 8},
                {1, 2, 3, 4, 5, 6, 7, 8}
        };
        auto result = _::flatten(a);

        for (const auto &item:result) {
            std::cout << item << " ";
        }
        std::cout << std::endl;

        std::cout << "OK." << std::endl;
    }

    void test_chain() {

        std::cout << "Testing chain..." << std::endl;

        std::vector<std::vector<int>> a{
                {1, 2, 3, 4, 5, 6, 7, 8},
                {2, 3, 4, 5, 6, 7, 8, 9},
                {3, 4, 5, 6, 7, 8, 9, 10}
        };
        int result = _::chain(a)
                .flatten<std::vector<int>>()
                .reduce([](const int &memo, const int &item) -> int { return memo + item; }, 0)
                .value();
        std::cout << result << std::endl;

        std::cout << "OK." << std::endl;
    }

    void test_underscore() {

        test_each();
        test_map();
        test_filter();
        test_group();
        test_reduce();
        test_flatten();
        test_chain();
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
            std::cout << "Testing parallel filter..." << std::endl;

            int n = 100000;
            std::vector<int> a;
            for (int i = 1; i <= n; i++) {
                a.push_back(i);
            }

            using result_type = std::vector<int>;
            auto a2 = _::parallel::filter(a, [](const int &item) -> bool {
                return item % 1000 == 0;
            });
            for (size_t i = 0; i < a2.size(); i++) {
                assert(a2[i] % 1000 == 0);
            }
            assert(a2.size() == n / 1000);

            std::cout << "OK..." << std::endl;
        }

        void test_group() {

            std::cout << "Testing parallel group..." << std::endl;

            int n = 100000;
            std::vector<int> a;
            for (int i = 1; i <= n; i++) {
                a.push_back(i);
            }

            auto result = _::parallel::group<int>(a, [](const int &item) -> int {
                return item % 10;
            });

            using item_type = decltype(result)::value_type;
            _::each(result, [&n](const item_type &item) {
                int count = _::reduce(item.second, [](int memo, int ele) -> int {
                    return memo + 1;
                }, 0);
                std::cout << count << " ";
            });

            std::cout << "OK." << std::endl;
        }

        void test_flatten() {
            std::cout << "Testing parallel flatten..." << std::endl;

            std::vector<std::vector<int>> a{
                    {1, 2, 3, 4, 5, 6, 7, 8},
                    {1, 2, 3, 4, 5, 6, 7, 8},
                    {1, 2, 3, 4, 5, 6, 7, 8},
                    {1, 2, 3, 4, 5, 6, 7}
            };
            auto result = _::parallel::flatten(a);

            for (const auto &item:result) {
                std::cout << item << " ";
            }
            std::cout << std::endl;

            std::cout << "OK." << std::endl;
        }

        void test_chain() {

            std::cout << "Testing parallel chain..." << std::endl;

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
        }

        void test_parallel_each_for_map() {
            std::cout << "Testing parallel each for map..." << std::endl;

            std::map<int, int> mp;
            const int n = 10000000;
            for (int i = 0; i < n; i++) {
                mp[i] = i;
            }
            std::vector<int> v(n);
            _::parallel::each(mp, [&v](const std::pair<int, int> &p) {
                v[p.first] = p.second;
            });

            for (int i = 0; i < n; i++) {
                assert(v[i] == i);
            }

            std::cout << "OK." << std::endl;
        }

        void test_parallel_underscore() {
            test_each();
            test_map();
            test_filter();
            test_group();
            test_flatten();
            test_chain();
            test_parallel_each_for_map();
        }

    }
}

int main() {

    test::test_underscore();
    test::parallel::test_parallel_underscore();

    return 0;
}