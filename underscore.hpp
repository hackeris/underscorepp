//
// Created by hackeris on 2017/3/15.
//

#ifndef UNDERSCOREPP_UNDERSCORE_HPP
#define UNDERSCOREPP_UNDERSCORE_HPP

#include <map>
#include <thread>

namespace _ {

    using namespace std;

    template<typename Container, typename Function>
    void each(const Container &container, Function function) {
        for (const auto &item: container) {
            function(item);
        }
    };

    template<typename ResultContainer, typename Container, typename Function>
    ResultContainer map(const Container &container, Function function) {
        ResultContainer result;
        for (const auto &item : container) {
            result.push_back(function(item));
        }
        return result;
    };

    template<typename Container, typename Function>
    Container filter(const Container &container, Function function) {
        Container result;
        for (const auto &item: container) {
            if (function(item)) {
                result.push_back(item);
            }
        }
        return result;
    };

    template<typename GroupKey, typename Container, typename Function>
    std::map<GroupKey, Container> group(const Container &container, Function function) {
        std::map<GroupKey, Container> result;
        _::each(container, [&result, &function](const typename Container::value_type &item) {
            GroupKey key = function(item);
            if (result.find(key) == result.end()) {
                Container tmpC;
                tmpC.push_back(item);
                result[key] = tmpC;
            } else {
                result[key].push_back(item);
            }
        });
        return result;
    };

    template<typename ResultType, typename Container, typename Function>
    ResultType reduce(const Container &container, Function function, ResultType init) {
        ResultType result = init;
        each(container, [&result, &function](const typename Container::value_type &item) {
            result = function(result, item);
        });
        return result;
    };

    template<typename ResultType, typename Container, typename Function>
    ResultType reduceRight(const Container &container, Function function, ResultType init) {
        ResultType result = init;
        each(container, [&result, &function](const typename Container::value_type &item) {
            result = function(item, result);
        });
        return result;
    };

}

namespace _ {

    namespace parallel {

        static const int THREADS = 4;

        template<typename Container>
        void _peach(const Container &container, std::function<void(int tid, int idx)> function) {
            size_t size = container.size();
            std::thread **threads = new thread *[THREADS];
            for (int i = 0; i < THREADS; i++) {
                threads[i] = new thread([&function, &container, size, i]() {
                    auto start = i * size / THREADS;
                    auto end = std::min((i + 1) * size / THREADS, size);
                    for (auto j = start; j < end; j++) {
                        function(i, j);
                    }
                });
            }
            for (int i = 0; i < THREADS; i++) {
                threads[i]->join();
            }
            delete[] threads;
        };

        template<typename Container, typename Function>
        void each(const Container &container, Function function) {
            _peach<Container>(container, [&container, &function](int i, int j) {
                function(container[j]);
            });
        };

        template<typename ResultContainer, typename Container, typename Function>
        ResultContainer map(const Container &container, Function function) {
            ResultContainer result(container.size());
            _peach(container, [&result, &function, &container](int tid, int idx) {
                result[idx] = function(container[idx]);
            });
            return result;
        };

        template<typename Container, typename Function>
        Container filter(const Container &container, Function function) {
            Container result;
            std::mutex _mutex;
            _peach(container, [&result, &_mutex, &container, &function](int tid, int idx) {
                if (function(container[idx])) {
                    _mutex.lock();
                    result.push_back(container[idx]);
                    _mutex.unlock();
                }
            });
            return result;
        };
    }

}


#endif //UNDERSCOREPP_UNDERSCORE_HPP
