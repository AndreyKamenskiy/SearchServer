#include "string_processing.h"
#include <vector>
#include <string>
#include <string_view>

std::vector<std::string_view> SplitIntoWords(const std::string_view text) {
    std::vector<std::string_view> words;
    size_t word_begin = 0;
    int wordLength = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == ' ') {
            word_begin = i + 1;
            if (wordLength > 0) {
                words.push_back(std::string_view(&text[word_begin], wordLength));
                wordLength = 0;
            }
        } else {
            ++wordLength;
        }
    }
    if (wordLength > 0) {
        words.push_back(std::string_view(&text[word_begin], wordLength));
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