#include "process_queries.h"
#include <execution>
#include <algorithm>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    
    std::vector<std::vector<Document>> res(queries.size());

    std::transform(
        std::execution::par,
        queries.begin(), queries.end(), 
        res.begin(), 
        [&search_server](const std::string& query) {
            return search_server.FindTopDocuments(query);
        }
    );
    return res;
}

JoinedDocuments ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> res(queries.size());
    size_t size = 0;

    std::transform(
        std::execution::par,
        queries.begin(), queries.end(),
        res.begin(),
        [&search_server, &size](const std::string& query) {
            std::vector<Document> queryRes = search_server.FindTopDocuments(query);
            size += queryRes.size();
            return queryRes;
        }
    );
    return JoinedDocuments(std::move(res));
}
