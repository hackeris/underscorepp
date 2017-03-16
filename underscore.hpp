//
// Created by hackeris on 2017/3/15.
//

#ifndef UNDERSCOREPP_UNDERSCORE_HPP
#define UNDERSCOREPP_UNDERSCORE_HPP

#include <map>
#include <set>
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

    template<typename Container, typename Function>
    typename Container::iterator findFirst(Container &container, Function function) {
        for (auto itr = container.begin();
             itr != container.end();
             itr++) {
            if (function(*itr)) {
                return itr;
            }
        }
        return container.end();
    };

    template<typename ContainerOfContainer>
    typename ContainerOfContainer::value_type flatten(ContainerOfContainer &containerOfContainer) {
        typename ContainerOfContainer::value_type result;
        for (const auto &container: containerOfContainer) {
            for (const auto &item: container) {
                result.push_back(item);
            }
        }
        return result;
    }
}

namespace _ {

    namespace parallel {

        static const int THREADS = 4;

        template<typename Container>
        void _peach(const Container &container, std::function<void(size_t tid, size_t idx)> function) {
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
            _peach<Container>(container, [&container, &function](size_t i, size_t j) {
                function(container[j]);
            });
        };

        template<typename ResultContainer, typename Container, typename Function>
        ResultContainer map(const Container &container, Function function) {
            ResultContainer result(container.size());
            _peach(container, [&result, &function, &container](size_t tid, size_t idx) {
                result[idx] = function(container[idx]);
            });
            return result;
        };

        template<typename Container, typename Function>
        Container filter(const Container &container, Function function) {
            Container result;
            std::mutex _mutex;
            _peach(container, [&result, &_mutex, &container, &function](size_t tid, size_t idx) {
                if (function(container[idx])) {
                    _mutex.lock();
                    result.push_back(container[idx]);
                    _mutex.unlock();
                }
            });
            return result;
        };

        template<typename GroupKey, typename Container, typename Function>
        std::map<GroupKey, Container> group(const Container &container, Function function) {

            using result_type =std::map<GroupKey, Container>;

            std::vector<result_type> temp(THREADS);
            _peach(container, [&container, &temp, &function](size_t tid, size_t idx) {
                const auto &item = container[idx];
                auto &ttemp = temp[tid];
                GroupKey key = function(item);
                if (ttemp.find(key) == ttemp.end()) {
                    Container tmpC;
                    tmpC.push_back(item);
                    ttemp[key] = tmpC;
                } else {
                    ttemp[key].push_back(item);
                }
            });

            std::set<GroupKey> tempKeys;
            _::each(temp, [&tempKeys](const result_type &item) {
                for (const auto &pair: item) {
                    tempKeys.insert(pair.first);
                }
            });
            using KeysType = std::vector<GroupKey>;
            result_type result;
            auto keys = _::map < KeysType > (tempKeys,
                    [&result](const GroupKey &key) -> GroupKey {
                        result[key] = Container();
                        return key;
                    });

            each(keys, [&result, &temp](const GroupKey &key) {
                _::each(temp, [&key, &result](const result_type &itemp) {
                    auto &keySet = result[key];
                    if (itemp.find(key) != itemp.end()) {
                        auto &tset = const_cast<result_type &>(itemp)[key];
                        for (const auto &item: tset) {
                            keySet.push_back(item);
                        }
                    }
                });
            });
            return result;
        };

        template<typename ContainerOfContainer>
        typename ContainerOfContainer::value_type flatten(ContainerOfContainer &containerOfContainer) {

            //  get ranges of each container in result
            typedef std::tuple<size_t, size_t, size_t> range_type;
            size_t total_size = 0;
            std::vector<range_type> ranges(containerOfContainer.size());
            for (size_t i = 0; i < containerOfContainer.size(); i++) {
                ranges[i] = make_tuple(i, total_size, total_size + containerOfContainer[i].size());
                total_size += containerOfContainer[i].size();
            }

            //  perform parallel flatten
            typename ContainerOfContainer::value_type result(total_size);
            _peach(ranges, [&containerOfContainer, &ranges, &result](size_t tid, size_t idx) {
                range_type range = ranges[idx];
                size_t cid = std::get<0>(range);
                size_t start = std::get<1>(range);
                size_t end = std::get<2>(range);
                for (size_t i = start; i < end; i++) {
                    result[i] = (containerOfContainer[cid][i - start]);
                }
            });
            return result;
        }
    }

}


#endif //UNDERSCOREPP_UNDERSCORE_HPP
