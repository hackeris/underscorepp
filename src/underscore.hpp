//
// Created by hackeris on 2017/3/15.
//

#ifndef UNDERSCOREPP_UNDERSCORE_HPP
#define UNDERSCOREPP_UNDERSCORE_HPP

#include <map>
#include <set>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>

#undef min
#undef max

#ifndef N_THREADS
#define N_THREADS 4
#endif

namespace _ {

    using namespace std;

    template<typename Container, typename Function>
    void each(const Container &container, Function function) {
        for (const auto &item : container) {
            function(item);
        }
    };

    template<typename ResultContainer, typename Container, typename Function>
    ResultContainer map(const Container &container, Function function) {
        ResultContainer result;
        for (const auto &item : container) {
            result.push_back(function(item));
        }
        return std::move(result);
    };

    template<typename Container, typename Function>
    Container filter(const Container &container, Function function) {
        Container result;
        for (const auto &item : container) {
            if (function(item)) {
                result.push_back(item);
            }
        }
        return std::move(result);
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
        return std::move(result);
    };

    template<typename ResultType, typename Container, typename Function>
    ResultType reduce(const Container &container, Function function, ResultType init) {
        ResultType result = init;
        each(container, [&result, &function](const typename Container::value_type &item) {
            result = function(result, item);
        });
        return std::move(result);
    };

    template<typename ContainerOfContainer>
    typename ContainerOfContainer::value_type flatten(ContainerOfContainer &containerOfContainer) {
        typename ContainerOfContainer::value_type result;
        for (const auto &container : containerOfContainer) {
            for (const auto &item : container) {
                result.push_back(item);
            }
        }
        return std::move(result);
    }
}

namespace _ {

    namespace parallel {

        static const int THREADS = N_THREADS;

        template<typename Container>
        struct _peach_selector {
            static void each(const Container &container,
                             std::function<void(size_t tid, size_t idx,
                                                const typename Container::value_type &elem)> function) {
#ifdef _DEBUG
                std::cout << "underscore: parallel with " << N_THREADS << " threads." << std::endl;
#endif

                size_t size = container.size();
                std::thread **threads = new thread *[THREADS];
                for (size_t i = 0; i < THREADS; i++) {
                    threads[i] = new thread([&function, &container, size, i]() {
                        auto start = i * size / THREADS;
                        auto end = std::min((i + 1) * size / THREADS, size);
                        for (auto j = start; j < end; j++) {
                            function(i, j, container[j]);
                        }
                    });
                }
                for (int i = 0; i < THREADS; i++) {
                    threads[i]->join();
                }
                delete[] threads;
            }
        };

        template<typename K, typename V>
        struct _peach_selector<std::map<K, V>> {

            using Container = std::map<K, V>;

            static void each(const Container &container,
                             std::function<void(size_t tid, size_t idx,
                                                const typename Container::value_type &elem)> function) {
#ifdef _DEBUG
                std::cout << "underscore: parallel with " << N_THREADS << " threads." << std::endl;
#endif
                size_t size = container.size();
                size_t idx = 0;
                auto itr = container.begin();
                auto end = container.end();

                std::mutex _mutex;

                std::thread **threads = new thread *[THREADS];
                for (size_t i = 0; i < THREADS; i++) {
                    threads[i] = new thread([&function, &container, &idx, i, &_mutex, &itr, &end]() {

                        while (true) {
                            //  get itr first
                            _mutex.lock();
                            if (itr == end) {
                                _mutex.unlock();
                                break;
                            } else {
                                itr++;
                                idx++;
                                auto local_itr = itr;
                                _mutex.unlock();
                                function(i, idx, *local_itr);
                            }
                        }
                    });
                }
                for (int i = 0; i < THREADS; i++) {
                    threads[i]->join();
                }
                delete[] threads;
            }
        };

        template<typename Container>
        void _peach(const Container &container,
                    std::function<void(size_t tid, size_t idx, const typename Container::value_type &elem)> function) {
            _peach_selector<Container>::each(container, function);
        };

        template<typename Container, typename Function>
        void each(const Container &container, Function function) {
            _peach<Container>(container,
                              [&container, &function](size_t i, size_t j, const typename Container::value_type &elem) {
                                  function(elem);
                              });
        };

        template<typename ResultContainer, typename Container, typename Function>
        ResultContainer map(const Container &container, Function function) {
            ResultContainer result(container.size());
            _peach(container, [&result, &function, &container](size_t tid, size_t idx,
                                                               const typename Container::value_type &elem) {
                result[idx] = function(elem);
            });
            return std::move(result);
        };

        template<typename Container, typename Function>
        Container filter(const Container &container, Function function) {
            Container result;
            std::mutex _mutex;
            _peach(container, [&result, &_mutex, &container, &function](size_t tid, size_t idx,
                                                                        const typename Container::value_type &elem) {
                if (function(elem)) {
                    _mutex.lock();
                    result.push_back(elem);
                    _mutex.unlock();
                }
            });
            return std::move(result);
        };

        template<typename KeyType, typename ValueType>
        std::map<KeyType, std::vector<ValueType>>
        merge(const std::vector<std::map<KeyType, std::vector<ValueType>>> &temp) {

            using ValueContainer = std::vector<ValueType>;
            using ResultType =std::map<KeyType, ValueContainer>;
            using KeysType = std::vector<KeyType>;

            //  get all the grouped keys, put them into a vector
            std::set<KeyType> tempKeys;
            _::each(temp, [&tempKeys](const ResultType &item) {
                for (const auto &pair : item) {
                    tempKeys.insert(pair.first);
                }
            });
            ResultType result;
            auto keys = _::map < KeysType > (tempKeys,
                    [&result](const KeyType &key) -> KeyType {
                        result[key] = ValueContainer();
                        return key;
                    });

            //  parallel reduce temp maps into single. paralleled by keys
            each(keys, [&result, &temp](const KeyType &key) {
                _::each(temp, [&key, &result](const ResultType &itemp) {
                    auto &keySet = result[key];
                    if (itemp.find(key) != itemp.end()) {
                        auto &tset = const_cast<ResultType &>(itemp)[key];
                        for (const auto &item : tset) {
                            keySet.push_back(item);
                        }
                    }
                });
            });
            return std::move(result);
        };

        template<typename GroupKey, typename Container, typename Function>
        std::map<GroupKey, Container> group(const Container &container, Function function) {

            using result_type = std::map<GroupKey, Container>;

            //  parallel group data to many temp maps.
            std::vector<result_type> temp(THREADS);
            _peach(container, [&temp, &function](size_t tid, size_t idx, const typename Container::value_type &elem) {
                const auto &item = elem;
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

            return merge(temp);
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
            _peach(ranges, [&containerOfContainer, &ranges, &result](size_t tid, size_t idx, const range_type &range) {
                size_t cid = std::get<0>(range);
                size_t start = std::get<1>(range);
                size_t end = std::get<2>(range);
                for (size_t i = start; i < end; i++) {
                    result[i] = (containerOfContainer[cid][i - start]);
                }
            });
            return std::move(result);
        }
    }
}

namespace _ {

    namespace strategy {

        struct Serial {

            template<typename Container, typename Function>
            static void each(const Container &container, Function function) {
                _::each(container, function);
            };

            template<typename ResultContainer, typename Container, typename Function>
            static ResultContainer map(const Container &container, Function function) {
                return _::map < ResultContainer > (container, function);
            };

            template<typename Container, typename Function>
            static Container filter(const Container &container, Function function) {
                return _::filter(container, function);
            };

            template<typename GroupKey, typename Container, typename Function>
            static std::map<GroupKey, Container> group(const Container &container, Function function) {

                return _::group(container, function);
            };

            template<typename ResultType, typename Container, typename Function>
            static ResultType reduce(const Container &container, Function function, ResultType init) {
                return _::reduce(container, function, init);
            };

            template<typename ContainerOfContainer>
            static typename ContainerOfContainer::value_type flatten(ContainerOfContainer &containerOfContainer) {

                return _::flatten(containerOfContainer);
            }
        };

        struct Parallel {

            template<typename Container, typename Function>
            static void each(const Container &container, Function function) {
                _::parallel::each(container, function);
            };

            template<typename ResultContainer, typename Container, typename Function>
            static ResultContainer map(const Container &container, Function function) {
                return _::parallel::map<ResultContainer>(container, function);
            };

            template<typename Container, typename Function>
            static Container filter(const Container &container, Function function) {
                return _::parallel::filter(container, function);
            };

            template<typename GroupKey, typename Container, typename Function>
            static std::map<GroupKey, Container> group(const Container &container, Function function) {

                return _::parallel::group(container, function);
            };

            template<typename ResultType, typename Container, typename Function>
            static ResultType reduce(const Container &container, Function function, ResultType init) {
                return _::reduce(container, function, init);
            };

            template<typename ContainerOfContainer>
            static typename ContainerOfContainer::value_type flatten(ContainerOfContainer &containerOfContainer) {
                return _::parallel::flatten(containerOfContainer);
            }
        };
    }

    using Serial = _::strategy::Serial;
    using Parallel = _::strategy::Parallel;

    template<typename Container, typename StrategyType>
    class Wrapper {

        using WrapperType = Wrapper<Container, StrategyType>;

    public:
        Wrapper(const Container &container) : container(container) {}

        Container value() const {
            return container;
        }

        template<typename Strategy=StrategyType, typename Function>
        void each(Function function) {
            Strategy::each(container, function);
        }

        template<typename ResultContainer, typename Strategy=StrategyType, typename Function>
        WrapperType map(Function function) {
            return WrapperType(Strategy::template map<ResultContainer>(container, function));
        };

        template<typename Strategy=StrategyType, typename Function>
        WrapperType filter(Function function) {
            return WrapperType(Strategy::filter(container, function));
        }

        template<typename GroupKey, typename Strategy=StrategyType, typename Function>
        WrapperType group(Function function) {
            return WrapperType(Strategy::group(container, function));
        };

        template<typename ResultType, typename Strategy=StrategyType, typename Function>
        Wrapper<ResultType, StrategyType> reduce(Function function, ResultType init) {
            return Wrapper<ResultType, StrategyType>(Strategy::reduce(container, function, init));
        };

        template<typename ResultType, typename Strategy=StrategyType>
        Wrapper<ResultType, StrategyType> flatten() {
            return Wrapper<ResultType, StrategyType>(Strategy::template flatten<Container>(container));
        }

    private:
        Container container;
    };

    template<typename StrategyType=Serial, typename Container>
    Wrapper<Container, StrategyType> chain(const Container &container) {
        return Wrapper<Container, StrategyType>(container);
    }
}

#endif //UNDERSCOREPP_UNDERSCORE_HPP