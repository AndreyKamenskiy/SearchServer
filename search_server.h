#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cmath>
#include <execution>
#include <string_view>

#include "document.h"
#include "string_processing.h"
#include "log_duration.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
    {
    }

    explicit SearchServer(const std::string_view stop_words_text);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
        LOG_DURATION_STREAM("Operation time", std::cerr);
        using namespace std;
        const auto query = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
            if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    int GetDocumentCount() const;

    const std::map<std::string, double>& GetWordFrequencies(int document_id) const;


    template<class ExecutionPolicy>
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const std::string& raw_query, int document_id) const {
        using namespace std;
        if ((document_id < 0) || (documents_.count(document_id) == 0)) {
            throw invalid_argument("Invalid document_id"s);
        }
        //LOG_DURATION_STREAM("Operation time", std::cerr);
        const auto query = ParseQuery(raw_query);
        vector<string> matched_words;
        bool findMinus = false;
        auto& word_to_document_freqs = word_to_document_freqs_;

        {
            std::vector<int> temp(query.minus_words.size());
            std::transform(policy,
                query.minus_words.begin(), query.minus_words.end(),
                temp.begin(),
                [&findMinus, document_id, &word_to_document_freqs](auto& minusWord) {
                    if (!findMinus
                        && word_to_document_freqs.count(minusWord)
                        && word_to_document_freqs.at(minusWord).count(document_id)) {
                        findMinus = true;
                    }
                    return 0;
                }
            );
        }
        if (findMinus) {
            return { matched_words, documents_.at(document_id).status };
        }
        {
            using namespace std;
            const std::string emptyString = ""s;
            std::vector<const std::string*> temp(query.plus_words.size());
            std::transform(policy,
                query.plus_words.begin(), query.plus_words.end(),
                temp.begin(),
                [&emptyString, document_id, &word_to_document_freqs](const std::string& plusWord) {
                    if (word_to_document_freqs.count(plusWord)
                        && word_to_document_freqs.at(plusWord).count(document_id)) {
                        return &plusWord;
                    }
                    return &emptyString;
                }
            );
            for (const string* curr : temp) {
                if (curr != &emptyString) {
                    matched_words.push_back(*curr);
                }
            }
        }
        return { matched_words, documents_.at(document_id).status };
    }


    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;


    template<class ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id) {
        if (documents_.count(document_id) == 0) {
            //if the document with given id does not exist
            return;
        }

        //delete from documents_;
        documents_.erase(document_id);
        //delete from documnet_ids_
        document_ids_.erase(document_id);

        auto& doc_words = document_to_word_freqs_.at(document_id);
        auto& wtdf = word_to_document_freqs_;

        std::vector<int> temp(doc_words.size());
        std::transform(policy,
            doc_words.begin(), doc_words.end(),
            temp.begin(),
            [&wtdf, document_id](auto& pairWordFreq) {
                wtdf.at(pairWordFreq.first).erase(document_id);
                return 0;
            }
        );

        document_to_word_freqs_.erase(document_id);
    }

    void RemoveDocument(int document_id) {
        RemoveDocument(std::execution::seq, document_id);
    }

    auto begin() {
        return document_ids_.begin();
    };

    auto end() {
        return document_ids_.end();
    };

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    const std::set<std::string> stop_words_;

    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string, double>> document_to_word_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;

    const std::map<std::string, double> emty_map_;


    bool IsStopWord(const std::string& word) const;

    static bool IsValidWord(const std::string& word);

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(const std::string& text) const;

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };
    Query ParseQuery(const std::string& text) const;


    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
        std::map<int, double> document_to_relevance;
        for (const std::string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }
};
