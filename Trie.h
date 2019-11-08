#ifndef TRIE_H
#define TRIE_H

#include "TrieNode.h"
#include <string>
#include <cctype>
using namespace std;

class Trie
{
private:
	TrieNode* _root = nullptr;

protected:

public:
	Trie()
	{
		_root = new TrieNode{};
	}

	virtual ~Trie()
	{
		delete this;
	}

	TrieNode* getRoot()
	{
		return _root;
	}

	void addWord(const string& word)
	{
		TrieNode* iter = _root;

		for (auto character : word)
		{
			if (iter->hasChild(character) == false)
				iter->setChild(character, new TrieNode(character));
			iter = iter->getChild(character);

		}
		iter->setChild('$', new TrieNode('$'));
	}

	vector<string> search(const string& word)
	{
		vector<string> matches;
		TrieNode* iter = _root;

		for (auto character : word)
		{
			if (iter->hasChild(character))
				iter = iter->getChildren()[character];
			else
				return matches;
		}

		string temp_word = word;
		for (auto child : iter->getChildren())
		{
			if (child.first != '$')
			{
				temp_word = temp_word + child.first;
				searchHelper(matches, word + child.first);
			}
			else
				matches.push_back(temp_word);
		}

		return matches;
	}

	void searchHelper(vector<string>& matches, const string& word)
	{
		TrieNode* iter = _root;

		for (auto character : word)
		{
			if (iter->hasChild(character))
				iter = iter->getChildren()[character];
			else
				return;
		}

		string temp_word = word;
		for (auto child : iter->getChildren())
		{
			if (child.first != '$')
			{
				temp_word = temp_word + child.first;
				searchHelper(matches, word + child.first);
			}
			else
				matches.push_back(temp_word);
		}
	}
};


#endif // !TRIE_H