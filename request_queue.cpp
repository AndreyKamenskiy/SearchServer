#include "request_queue.h"

    RequestQueue::RequestQueue(const SearchServer& search_server):
        search_server_(search_server)
    {}

    std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        auto result = search_server_.FindTopDocuments(raw_query, status);
        requests_.push_back({raw_query, result});
        check_overflow();
        return result;
    }

    std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
        auto result = search_server_.FindTopDocuments(raw_query);
        requests_.push_back({raw_query, result});
        check_overflow();
        return result;
    }

    int RequestQueue::GetNoResultRequests() const {
        return count_if(requests_.begin(), requests_.end(), [](QueryResult result) {
            return result.result.empty();
        });
    }

    void RequestQueue::check_overflow() {
        while(requests_.size() > min_in_day_) {
            requests_.pop_front();
        }
    }
