using namespace std;

#include <stdexcept>

#include "search_server.h"
#include "string_processing.h"
#include "document.h"
#include "log_duration.h"
#include <iostream>
#include <execution>
#include <string_view>

    SearchServer::SearchServer(const string_view stop_words_view)
        : SearchServer(SplitIntoWords(stop_words_view))
    {
    }

    SearchServer::SearchServer(const string stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))
    {
    }

    void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        SearchServer::AddDocument(document_id, std::string_view(document), status, ratings);
    }

    void SearchServer::AddDocument(int document_id, const string_view& document_view, DocumentStatus status, const vector<int>& ratings) {
        if ((document_id < 0) || (documents_.count(document_id) > 0)) {
            throw invalid_argument("Invalid document_id"s);
        }
        const auto words = SearchServer::saveUniqueWords(SplitIntoWordsNoStop(document_view));
        const double inv_word_count = 1.0 / words.size();
        for (const string_view& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
            document_to_word_freqs_[document_id][word] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
        document_ids_.insert(document_id);
    }


    vector<Document> SearchServer::FindTopDocuments(const string_view& raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
    }

    vector<Document> SearchServer::FindTopDocuments(const string_view& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    int SearchServer::GetDocumentCount() const {
        return documents_.size();
    }


    tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const string_view& raw_query, int document_id) const {
        return SearchServer::MatchDocument(std::execution::seq, raw_query, document_id);
    }

    /*tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
        return SearchServer::MatchDocument(std::execution::seq, static_cast<string_view>(raw_query), document_id);
    }*/


    const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const
    {
        if (documents_.count(document_id) == 0) {
           //if a document with given id does not exist
           return emty_map_;
        }
        return document_to_word_freqs_.at(document_id);
    }

    bool SearchServer::IsStopWord(const string_view& word) const {
        return stop_words_.count(word) > 0;
    }

    bool SearchServer::IsValidWord(const string_view& word) {
        // A valid word must not contain special characters
        return HasSpecialSymbols(word);
    }

    vector<string_view> SearchServer::SplitIntoWordsNoStop(const string_view& text) const {
        vector<string_view> words;
        for (string_view& word : SplitIntoWords(text)) {
            if (!IsValidWord(word)) {
                throw invalid_argument("Word "s + static_cast<string>(word) + " is invalid"s);
            }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    SearchServer::QueryWord SearchServer::ParseQueryWord(const string_view& text) const {
        if (text.empty()) {
            throw invalid_argument("Query word is empty"s);
        }
        string_view word = text;
        bool is_minus = false;
        if (word[0] == '-') {
            is_minus = true;
            word = word.substr(1);
        }
        if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
            throw invalid_argument("Query word "s + static_cast<string>(text) + " is invalid");
        }

        return {word, is_minus, IsStopWord(static_cast<string>(word))};
    }

    SearchServer::Query SearchServer::ParseQuery(const string_view& text) const {
        SearchServer::Query result;
        for (const string_view& word : SplitIntoWords(text)) {
            const auto query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    result.minus_words.insert(query_word.data);
                } else {
                    result.plus_words.insert(query_word.data);
                }
            }
        }
        return result;
    }


    double SearchServer::ComputeWordInverseDocumentFreq(const string_view& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

