//remove duplicates

#include "remove_duplicates.h"
#include "string_processing.h"
#include <set>
#include <map>
#include <vector>


uint64_t getDocumentHash(const std::map<std::string, double>& word_freq) {
	uint64_t hash = 0;
	for (const std::pair<std::string, double>& item : word_freq) {
		hash ^= getStringHash(item.first);
	}
	return hash;
}

void RemoveDuplicates(SearchServer& search_server)
{
	using namespace std;
	std::map<uint64_t, std::vector<int>> seen;
	vector<int> to_delete;
	for (int current_id : search_server) {
		const auto& word_freq = search_server.GetWordFrequencies(current_id);
		uint64_t hash = getDocumentHash(word_freq);
		seen[hash].push_back(current_id);
	}
	for (auto& [hash, ids] : seen) {
		if (ids.size() < 2) {
			continue;
		}
		/*for (int i : ids) {
			cout << i << ' ';
		}
		cout << endl;*/
		sort(ids.begin(), ids.end());
		for (int i = 1; i < ids.size(); ++i) {
			to_delete.push_back(ids[i]);
		}
	}

	sort(to_delete.begin(), to_delete.end());
	for (int id : to_delete) {
		std::cout << "Found duplicate document id "s << id << endl;
		search_server.RemoveDocument(id);
	}

}
