#include "Trie.h"
#include <iostream>

/* Create a new Dictionary that uses a Trie back end */
DictionaryTrie::DictionaryTrie() { root = NULL; }

/* Destructor */
DictionaryTrie::~DictionaryTrie() { deleteAll(root); }

bool DictionaryTrie::insert(std::string word, unsigned int freq)
{
    if (word.length() == 0)
    {
        return false;
    }

    if (find(word))
    {
        return false;
    }

    auto retVal = traverse(word);
    auto curr = retVal.first;

    if (root == NULL)
    {
        root = new TrieNode(word[0]);
        curr = root;
        ++num_vertices;
    }

    else if (retVal.second == word.length())
    {
        retVal.first->isAWord = true;
        return true;
    }

    else if (retVal.first->symbol != word[retVal.second])
    {
        if (word[retVal.second] < curr->symbol)
        {
            curr->left = new TrieNode(word[retVal.second]);
            curr = curr->left;
            ++num_vertices;
        }
        else
        {
            curr->right = new TrieNode(word[retVal.second]);
            curr = curr->right;
            ++num_vertices;
        }
    }

    for (auto i = retVal.second + 1; i < word.length(); ++i)
    {
        curr->mid = new TrieNode(word[i]);
        curr = curr->mid;
        ++num_vertices;
    }

    curr->isAWord = true;
    curr->freq = freq;

    return true;
}

bool DictionaryTrie::prefixExists(std::string word)
{
    if (root == NULL)
    {
        return false;
    }

    auto retVal = traverse(word);

    return word[word.length() - 1] == retVal.first->symbol && retVal.second == word.length() - 1;
}

bool DictionaryTrie::find(std::string word) const
{
    if (root == NULL)
        return false;

    auto retVal = traverse(word);

    return word[word.length() - 1] == retVal.first->symbol && retVal.second == word.length() - 1 &&
           retVal.first->isAWord;
}

std::pair<TrieNode*, unsigned int> DictionaryTrie::traverse(std::string word) const
{
    TrieNode* curr = root;

    // If TST is empty, returns an empty pair
    if (curr == NULL)
        return std::make_pair(curr, 0);

    // We are going to traverse the trie as long as there is chars to cover
    unsigned int i = 0;

    // We will traverse as long there is characters to look for
    while (i < word.length())
    {
        if (curr->symbol == word[i])
        {
            if (curr->mid != NULL && i != word.length() - 1)
            {
                curr = curr->mid;
                ++i;
            }
            else
                return std::make_pair(curr, i);
        }

        else if (word[i] < curr->symbol)
        {
            if (curr->left != NULL)
                curr = curr->left;
            else
                return std::make_pair(curr, i);
        }

        else
        {
            if (curr->right != NULL)
                curr = curr->right;
            else
                return std::make_pair(curr, i);
        }
    }
    // Added as a precaution
    return std::make_pair(curr, i);
}

std::vector<std::string> DictionaryTrie::predictCompletions(std::string prefix, unsigned int num_completions)
{

    // First check if the prefix is in the TST & if not return an empty vector
    if (!prefixExists(prefix))
    {
        return {};
    }

    if (num_completions == 0)
    {
        std::cout << "The number of completions is set to 0, 0 returned." << std::endl;
        return {};
    }

    std::vector<std::string> words;
    std::multimap<int, std::string, std::greater<int>> prefixWords;

    // Finds all the words with the given prefix
    findPrefixWords(prefix, prefixWords);

    // Based on the num_completions add only the top # of words
    unsigned int count = 0;
    for (auto it = prefixWords.begin(); (count < num_completions) && (it != prefixWords.end()); ++it)
    {

        words.push_back(it->second);
        count++;
    }
    return words;
}

void DictionaryTrie::findPrefixWords(std::string prefix,
                                     std::multimap<int, std::string, std::greater<int>>& prefixWords)
{

    auto retVal = traverse(prefix);
    TrieNode* parent = retVal.first;

    // Checks if the prefix doesn't have any words, exits
    if (parent == NULL)
    {
        return;
    }

    // If the prefix is a word, add it to prefixWords
    if (parent->isAWord)
    {
        prefixWords.insert(std::pair<int, std::string>(parent->freq, prefix));
    }

    std::string buffer = "";
    // Get all prefix words
    getPrefixWords(parent->mid, prefix, prefixWords);
}

void DictionaryTrie::getPrefixWords(TrieNode* curr, std::string prefix,
                                    std::multimap<int, std::string, std::greater<int>>& prefixWords)
{

    // Base case
    if (curr == NULL)
        return;

    // Traverse the left side of TST
    getPrefixWords(curr->right, prefix, prefixWords);
    getPrefixWords(curr->left, prefix, prefixWords);

    // Check if node is a word, & store prefix + current character
    if (curr->isAWord)
        prefixWords.insert(std::pair<int, std::string>(curr->freq, prefix + curr->symbol));

    getPrefixWords(curr->mid, prefix + curr->symbol, prefixWords);
}

void DictionaryTrie::deleteAll(TrieNode* root)
{
    if (root == NULL)
        return;

    deleteAll(root->left);
    deleteAll(root->right);
    deleteAll(root->mid);

    delete root;
}
