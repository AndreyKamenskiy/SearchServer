#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <set>
#include <stdexcept>
#include <algorithm>

std::vector<std::string_view> SplitIntoWords(const std::string_view text);

template <typename StringWithIterators>
bool HasSpecialSymbols(const StringWithIterators word) {
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

template <typename StringViewContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringViewContainer& views) {
    std::set<std::string> non_empty_strings;
    for (const std::string_view view : views) {
        if (HasSpecialSymbols(view)) {
            using namespace std;
            throw invalid_argument("Stop words has illegal symbols in word: "s + view);
        }
        if (!view.empty()) {
            non_empty_strings.insert(string(view.begin(), view.end()));
        }
    }
    return non_empty_strings;
}

uint64_t getStringHash(const std::string& word);

