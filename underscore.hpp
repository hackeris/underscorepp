//
// Created by hackeris on 2017/3/15.
//

#ifndef UNDERSCOREPP_UNDERSCORE_HPP
#define UNDERSCOREPP_UNDERSCORE_HPP

#include <vector>
#include <map>

namespace _ {

    using namespace std;

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

    template<typename Container, typename Function>
    void each(const Container &container, Function function) {
        for (const auto &item: container) {
            function(item);
        }
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

}


#endif //UNDERSCOREPP_UNDERSCORE_HPP
