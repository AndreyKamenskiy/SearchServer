#pragma once
#include <vector>
#include <string>
#include <set>
#include <stdexcept>

std::vector<std::string> SplitIntoWords(const std::string& text);

bool HasSpecialSymbols(const std::string& text);

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string> non_empty_strings;
    for (const std::string& str : strings) {
        if (HasSpecialSymbols(str)) {
            using namespace std;
            throw invalid_argument("Stop words has illegal symbols in word: "s + str);
        }

        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

uint64_t getStringHash(const std::string& word);

