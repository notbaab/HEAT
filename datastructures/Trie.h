#pragma once

#include <map>
#include <string>
#include <vector>

class TrieNode
{
  public:
    char symbol;
    TrieNode* left;
    TrieNode* right;
    TrieNode* mid;
    bool isAWord;
    int freq;

    /* Constructor */
    TrieNode(char symbol, TrieNode* left = 0, TrieNode* right = 0, TrieNode* mid = 0, bool isAWord = false,
             int freq = 0)
        : symbol(symbol), left(left), right(right), mid(mid), isAWord(isAWord), freq(freq){};
};

class DictionaryTrie
{
  public:
    DictionaryTrie();

    unsigned int num_vertices = 0;
    bool insert(std::string word, unsigned int freq);

    bool find(std::string word) const;

    std::vector<std::string> predictCompletions(std::string prefix, unsigned int num_completions);

    /* Destructor */
    ~DictionaryTrie();

  private:
    TrieNode* root;
    std::pair<TrieNode*, unsigned int> traverse(std::string word) const;
    bool prefixExists(std::string prefix);
    void findPrefixWords(std::string prefix, std::multimap<int, std::string, std::greater<int>>& prefixWords);
    void getPrefixWords(TrieNode* curr, std::string prefix,
                        std::multimap<int, std::string, std::greater<int>>& prefixWords);

    void deleteAll(TrieNode* n);
};
