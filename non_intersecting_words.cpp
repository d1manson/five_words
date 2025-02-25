#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include "popcntintrin.h"
#include <chrono>
//#include <bitset>

using namespace std::chrono;

using namespace std;



// Loads words and leaves only 5-letter words with no repititions,
// also leaving only single world among anagrams.
vector<string> LoadWords(std::string filename) {
	vector<string> words;
	ifstream file(filename);
	vector<bool> seen(26*26*26*26*26);
	while (!file.eof()) {
		string word;
		file >> word;
		if (word.length() != 5) continue;
		string tmp = word;
		sort(tmp.begin(), tmp.end());
		bool bad_word = false;
		for (int i = 0; i < 4; ++i) {
			if (tmp[i] == tmp[i+1]) {
				bad_word = true;
				break;
			}
		}
		if (bad_word) continue;
		int hash = 0;
		for (int i = 0; i < 5; ++i) {
			hash = hash*26 + tmp[i] - 'a';
		}
		if (seen[hash]) continue;
		seen[hash] = true;
		words.push_back(word);
	}
	return words;
}

inline int mask_from_omitted_char(const char omit_char){
    return ((1 << 26) -1) ^ (1 << (omit_char - 'a'));
}


void OutputAllSets(const vector<bool> &can_construct,
					const vector<string> &words,
					const vector<int> &masks,
					vector<int> &result,
					int mask,
					int start_from) {
	if (result.size() == 5) {
		for (int i = 0; i < 5; ++i) {
			cout << words[result[i]] << ' ';
		}
		cout << "\n";
		return;
	}
	for (int cur_word = start_from; cur_word < (int)words.size(); ++cur_word) {
		if (((mask & masks[cur_word]) == masks[cur_word]) && (result.size() == 4 || can_construct[mask ^ masks[cur_word]])) {
			result.push_back(cur_word);
			OutputAllSets(can_construct, words, masks, result, mask ^ masks[cur_word], cur_word + 1);
			result.pop_back();
		}
	}
}


void Solve(const vector<string> &words) {
	vector<bool> can_construct(vector<bool>(1 << 26));
	vector<int> masks(words.size());
	cerr << "Memory allocated\n";
	for (int i = 0 ; i < (int)words.size(); ++i) {
		int mask = 0;
		for (auto c: words[i]) {
			mask |= 1 << (c - 'a');
		}
		masks[i] = mask;
		can_construct[mask] = true;
	}

	auto start = high_resolution_clock::now();
	for (int cnt = 0; cnt < 3; ++cnt) {
	    int n_visits = 0;
		for (int mask = 0; mask < (1 << 26); ++mask) {
			if (!can_construct[mask] || _mm_popcnt_u32(mask) != 5*(cnt+1) ) continue;
			n_visits++;
			for (int i = 0; i < (int)words.size(); ++i) {
				if ((masks[i] & mask) == 0) {
					can_construct[masks[i] | mask] = true;
				}
			}
		}
		cerr << "Word " << (cnt + 1) << " provided " << n_visits << " starting masks to find a next word for. Main loops so far: " << duration_cast<microseconds>(high_resolution_clock::now() - start).count()/1000 << " miliseconds" << endl;
	}
	cerr << "Main loop time: " <<  duration_cast<microseconds>(high_resolution_clock::now() - start).count()/1000 << " miliseconds" << endl;

	cerr << "DP done\n";


	vector<int> result;
	for (char omit_char = 'a'; omit_char <= 'z'; ++omit_char) {
	        int mask = mask_from_omitted_char(omit_char);
       	    can_construct[mask] = true;
			OutputAllSets(can_construct, words, masks, result, mask, 0);
	}
}


int main() {
    auto start = high_resolution_clock::now();
	cerr << "Loading words...";
	vector<string> words = LoadWords("words_alpha.txt");
	cerr << "loading time: " <<  duration_cast<microseconds>(high_resolution_clock::now() - start).count()/1000 << " miliseconds" << endl;
	cerr << "solving...";
	Solve(words);
	cerr << "Done\n";
	cerr << "total time: " <<  duration_cast<microseconds>(high_resolution_clock::now() - start).count()/1000 << " miliseconds" << endl;
}