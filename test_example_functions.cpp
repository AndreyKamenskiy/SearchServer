#include "test_example_functions.h"
#include "search_server.h"
#include <iostream>
#include <cassert>

using namespace std;

SearchServer makeServer() {
	SearchServer server("и в на за"s);
    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    return server;
}

void test_SS_iterators() {
	SearchServer& server = makeServer();
    auto it = server.begin();
    for (int i = 0; i < 4; ++i) {
        assert(*it == i);
        it++;
    }
    assert(it == server.end());
}

void test_all() {
    test_SS_iterators();

	std::cerr << "All tests passed!!!";
}