#include "test_example_functions.h"
#include "search_server.h"
#include <iostream>
#include <cassert>
#include <map>

using namespace std;

SearchServer makeSmallServer() {
	SearchServer server("и в на за"s);
    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    return server;
}

void test_SS_iterators(SearchServer& server) {
    auto it = server.begin();
    for (int i = 0; i < 4; ++i) {
        assert(*it == i);
        it++;
    }
    assert(it == server.end());
}


template< typename Map>
bool isEqual(const Map& m1, const Map& m2) {
    auto pred = [](auto a, auto b) { return a.first == b.first; };

    cout << endl << m1.size() << "-" << m2.size() << endl;


    return m1.size() == m2.size() && std::equal(m1.begin(), m1.end(), m2.begin(), pred);
};

void test_SS_GetWordFrequencies(SearchServer& server) {
    const map<string, double>& word_freq = server.GetWordFrequencies(1);
    map<string, double> map1{ {"пушистый"s, 0.5}, { "кот"s, 0.25 }, { "хвост"s, 0.25 } };
    assert(isEqual(word_freq, map1));
    assert(server.GetWordFrequencies(10).empty());
}

void test_all() {
    SearchServer& server = makeSmallServer();

    test_SS_iterators(server);
    test_SS_GetWordFrequencies(server);





	std::cerr << "All tests passed!!!";
}