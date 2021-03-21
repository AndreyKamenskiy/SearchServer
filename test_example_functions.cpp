#include "test_example_functions.h"
#include "search_server.h"
#include "remove_duplicates.h"
#include <iostream>
#include <cassert>
#include <map>

using namespace std;

SearchServer makeSmallServer() {
	SearchServer server("� � �� ��"s);
    server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
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
    return m1.size() == m2.size() && std::equal(m1.begin(), m1.end(), m2.begin(), pred);
};

void test_SS_GetWordFrequencies(SearchServer& server) {
    const map<string, double>& word_freq = server.GetWordFrequencies(1);
    map<string, double> map1{ {"��������"s, 0.5}, { "���"s, 0.25 }, { "�����"s, 0.25 } };
    assert(isEqual(word_freq, map1));
    assert(server.GetWordFrequencies(10).empty());
}

void test_SS_RemoveDocument(SearchServer server) {

    int count = server.GetDocumentCount();
    server.RemoveDocument(1);
    assert(server.GetWordFrequencies(1).empty());
    assert(server.GetDocumentCount() == count - 1);
    assert(find(server.begin(), server.end(), 1) == server.end());

    try{
        auto& [words, status] = server.MatchDocument("�������� ��� �����", 1);
        for (const auto& word : words) {
            cout << word << endl;
        }
        assert(1 == 2);
    }
    catch (invalid_argument e) {
        e.what();
    }

    auto& docs = server.FindTopDocuments
    (
        "�������� ��� �����",
        [](int document_id, DocumentStatus document_status, int rating) {
            return true;
        }
    );
    auto& it = find_if(docs.begin(), docs.end(), [](Document doc) {return doc.id == 1; });
    assert(docs.end() == it);
}

void test_RemoveDuplicates(SearchServer server) {
    server.AddDocument(5, "��������� ������� ��������� �������"s, DocumentStatus::ACTUAL, { 9, 5, 4});
    assert(server.GetDocumentCount() == 4);
    RemoveDuplicates(server);
    assert(server.GetDocumentCount() == 3);
}

void test_all() {
    SearchServer& server = makeSmallServer();

    test_SS_iterators(server);
    test_SS_GetWordFrequencies(server);
    test_SS_RemoveDocument(server);
    test_RemoveDuplicates(server);

	std::cerr << "All tests passed!!!\n";
}