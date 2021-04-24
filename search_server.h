#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>
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

    // скорее всего придется написать конструктор для контейнера из строк.


    template <typename StringViewContainer>
    explicit SearchServer(const StringViewContainer& stop_words)
        : stop_words_(SearchServer::saveUniqueWords(MakeUniqueNonEmptyStrings(stop_words)))
    {
    }

    explicit SearchServer(const std::string stop_words_text);

    explicit SearchServer(const std::string_view stop_words_view);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    void AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const {
        //LOG_DURATION_STREAM("Operation time", std::cerr);
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

    std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string_view& raw_query) const;

    int GetDocumentCount() const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;


    template<class ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const std::string_view& raw_query, int document_id) const {
        using namespace std;
        if ((document_id < 0) || (documents_.count(document_id) == 0)) {
            throw invalid_argument("Invalid document_id"s);
        }
        //LOG_DURATION_STREAM("Operation time", std::cerr);
        const auto query = ParseQuery(raw_query);
        vector<string_view> matched_words;
        /*bool findMinus = false;
        auto& word_to_document_freqs = word_to_document_freqs_;
        {
            std::vector<int> temp(query.minus_words.size());
            std::transform(policy,
                query.minus_words.begin(), query.minus_words.end(),
                temp.begin(),
                [&findMinus, document_id, &word_to_document_freqs](auto& minusWord) {
                    if (!findMinus // если еще не нашли минус слово
                        && word_to_document_freqs.count(minusWord) // если такое слово есть в документах
                        && word_to_document_freqs.at(minusWord).count(document_id)) { // если оно есть в искомом документе
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
            const std::string_view emptyStringView = ""s;
            std::vector<const std::string_view> temp(query.plus_words.size());
            std::transform(policy,
                query.plus_words.begin(), query.plus_words.end(),
                temp.begin(),
                [&emptyStringView, document_id, &word_to_document_freqs](const std::string_view& plusWord) {
                    if (word_to_document_freqs.count(plusWord)
                        && word_to_document_freqs.at(plusWord).count(document_id)) {
                        return plusWord;
                    }
                    return emptyStringView;
                }
            );
            for (const string_view& curr : temp) {
                if (curr != emptyStringView) {
                    matched_words.push_back(curr);
                }
            }
        }*/
        return { matched_words, documents_.at(document_id).status };
    }

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view& raw_query, int document_id) const;

    /*template<class ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const std::string& raw_query, int document_id) const {
        return MatchDocument(policy, static_cast<string_view>(raw_query), document_id);
    }

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;*/

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

    std::list<std::string> words_;
    std::set<std::string_view> all_words_;

    const std::set<std::string_view> stop_words_;

    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> document_to_word_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;

    const std::map<std::string_view, double> emty_map_;


    bool IsStopWord(const std::string_view& word) const;

    static bool IsValidWord(const std::string_view& word);

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view& text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(const std::string_view& text) const;

    struct Query {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };
    Query ParseQuery(const std::string_view& text) const;


    double ComputeWordInverseDocumentFreq(const std::string_view& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
        std::map<int, double> document_to_relevance;
        for (const std::string_view& word : query.plus_words) {
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

        for (const std::string_view& word : query.minus_words) {
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


    // принимает контейнер из string_view, проверяет есть ли такие слова в базе сервера (words_ и all_words);
    // сохраняет отсутствующие слова 
    // меняет words так, чтобы он ссылался на сохраненнные в сервере слова.
    template <typename StringViewContainer> 
    std::set<std::string_view> saveUniqueWords(const StringViewContainer& words) {
        std::set<std::string_view> saved;
        for (const std::string_view& word : words) {
            if (all_words_.count(word) == 0) {
                words_.push_back(static_cast<string>(word));
                all_words_.insert(words_.back());
                saved.insert(std::string_view(words_.back()));
            }
            else {
                saved.insert(*all_words_.find(word));
            }
        }
        return saved;
    }

};
