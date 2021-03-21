#include "string_processing.h"
#include <vector>
#include <string>

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

uint64_t getStringHash(const std::string& word)
{
    const uint64_t k = 127;
    uint64_t m = 1;
    uint64_t hash = 0;
    for (char ch : word) {
        hash += m * std::abs(ch);
        m = m * k;
    }
    return hash;
}