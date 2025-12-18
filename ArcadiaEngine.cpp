// ArcadiaEngine.cpp - STUDENT TEMPLATE
// TODO: Implement all the functions below according to the assignment requirements

#include "ArcadiaEngine.h"
#include <algorithm>
#include <queue>
#include <numeric>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <functional>

using namespace std;

// =========================================================
// PART A: DATA STRUCTURES (Concrete Implementations)
// =========================================================

// --- 1. PlayerTable (Double Hashing) ---

class ConcretePlayerTable : public PlayerTable
{
private:
	struct Player
	{
		int id;
		string name;
		bool occupied;

		Player()
		{
			id = -1;
			name = "";
			occupied = false;
		}
	};

	const int tableSize = 101;
	vector<Player> table;

	int h1(int key)
	{
		return key % tableSize;
	}

	int h2(int key)
	{
		// 97 because it's the smallest prime before 101
		return 97 - (key % 97);
	}

public:
	ConcretePlayerTable()
	{
		table.resize(tableSize);
	}

	void insert(int playerID, string name) override
	{
		// input error cases
		if (playerID < 0)
		{
			cout << "invalid id must be 0 or higher" << endl;
			return;
		}
		if (name == "")
		{
			cout << "invalid name, no name provided" << endl;
			return;
		}

		int hash1 = h1(playerID);
		int hash2 = h2(playerID);
		for (int i = 0; i < tableSize; i++)
		{
			int index = (hash1 + i * hash2) % tableSize;

			if (table[index].id == playerID)
			{
				cout << "this is a duplicate ID" << endl;
				return;
			}
			if (!table[index].occupied)
			{
				table[index].name = name;
				table[index].id = playerID;
				table[index].occupied = true;
				return;
			}
			// collision: loop continues to next i
		}

		// if loop ends then the table is full
		cout << "Table is full" << endl;
	}

	string search(int playerID) override
	{
		// input error cases
		if (playerID < 0)
		{
			cout << "invalid id must be be 0 or higher" << endl;
			return "";
		}

		int hash1 = h1(playerID);
		int hash2 = h2(playerID);

		for (int i = 0; i < tableSize; i++)
		{
			int index = (hash1 + i * hash2) % tableSize;

			// if while iterating you hit an empty slot that means
			// that means the player is not in the table
			if (!table[index].occupied)
			{
				return "";
			}

			if (table[index].id == playerID)
			{
				return table[index].name;
			}
		}
		// table is full and id isn't in the table
		return "";
	}
};

// --- 2. Leaderboard (Skip List) ---

class ConcreteLeaderboard : public Leaderboard
{
private:
	// TODO: Define your skip list node structure and necessary variables
	// Hint: You'll need nodes with multiple forward pointers
	struct SkipListNode
	{
		int playerID;
		int score;
		vector<SkipListNode *> forward;
		SkipListNode(int id, int score, int level) : playerID(id), score(score), forward(level, nullptr) {} // a constructor
	};
	SkipListNode *head;
	int maxLevel;
	int probability;

	// Helper Functions
	int randomLevel()
	{
		int level = 1;
		while (rand() % 100 < probability && level < maxLevel)
			level++;
		return level;
	}

	// here is a function to check if a node should come before another based on score and then playerID
	bool comesBefore(int score1, int id1, int score2, int id2)
	{
		if (score1 != score2)
			return score1 > score2; // higher score first
		return id1 < id2;			// lower playerID first if scores are equal
	}

public:
	ConcreteLeaderboard()
	{
		// TODO: Initialize your skip list
		maxLevel = 16;	  // let max level be 16
		probability = 50; // 50% chance to increase level
		head = new SkipListNode(-1, INT_MAX, maxLevel);
		srand(42);
	}

	void addScore(int playerID, int score) override
	{
		// TODO: Implement skip list insertion
		// Remember to maintain descending order by score
		int newLevel = randomLevel();
		vector<SkipListNode *> update(maxLevel, nullptr);
		SkipListNode *currentNode = head;
		// here we find the position to insert
		for (int i = maxLevel - 1; i >= 0; i--)
		{
			// moving right while the next node has higher score (or same score but lower playerID)
			while (currentNode->forward[i] != nullptr && comesBefore(currentNode->forward[i]->score, currentNode->forward[i]->playerID, score, playerID))
			{
				currentNode = currentNode->forward[i];
			}
			update[i] = currentNode;
		}
		for (int i = 0; i < newLevel; i++)
		{
			if (i >= (int)update.size())
				break;
			// as we insert in the middle, we need to adjust pointers
			SkipListNode *newNode = new SkipListNode(playerID, score, newLevel); // create new node
			newNode->forward[i] = update[i]->forward[i];						 // link new node to next node
			update[i]->forward[i] = newNode;									 // link previous node to new node
		}
	}

	void removePlayer(int playerID) override
	{
		// TODO: Implement skip list deletion
		SkipListNode *currentNode = head;
		SkipListNode *toBeDelete = nullptr;
		// searching in level 0
		while (currentNode != nullptr)
		{
			if (currentNode->forward[0] != nullptr && currentNode->forward[0]->playerID == playerID)
			{
				toBeDelete = currentNode->forward[0];
				break;
			}
		}
		if (toBeDelete == nullptr)
			return; // player not found
		int targetScore = toBeDelete->score;
		vector<SkipListNode *> update(maxLevel, nullptr);
		currentNode = head;
		// we will mover right and down to find the node to delete
		for (int i = maxLevel - 1; i >= 0; i--)
		{
			while (currentNode->forward[i] != nullptr && comesBefore(currentNode->forward[i]->score, currentNode->forward[i]->playerID, targetScore, playerID))
			{
				currentNode = currentNode->forward[i];
			}
			update[i] = currentNode;
		}

		// now i will just unlink the node from all levels
		// from level 0 to its highest level
		for (int i = 0; i < toBeDelete->forward.size(); i++)
		{
			// if true then the node exists at this level
			// so we just unlink it
			if (update[i]->forward[i] != nullptr && update[i]->forward[i]->playerID == playerID)
			{
				update[i]->forward[i] = update[i]->forward[i]->forward[i];
			}
		}
	}

	vector<int> getTopN(int n) override
	{
		// TODO: Return top N player IDs in descending score order
		int size = n;
		vector<int> result(n);
		SkipListNode *currentNode = head->forward[0]; // starting from the head as its the highest score
		for (int i = 0; i < size && currentNode != nullptr; i++)
		{
			result[i] = currentNode->playerID;
			currentNode = currentNode->forward[0];
		}
		return result;
	}
};

// --- 3. AuctionTree (Red-Black Tree) ---

class ConcreteAuctionTree : public AuctionTree
{
private:
	// TODO: Define your Red-Black Tree node structure
	// Hint: Each node needs: id, price, color, left, right, parent pointers
	struct RBNode
	{
		int itemID;
		int price;
		bool color; // true for Red, false for Black
		RBNode *left, *right, *parent;
		RBNode(int id, int price) : itemID(id), price(price), color(true), left(nullptr), right(nullptr), parent(nullptr) {}
	};
	RBNode *root;
	RBNode *TNULL; // sentinel node for leaves
	// Helper function for TNULL
	void initializeTNULL()
	{
		TNULL = new RBNode(0, 0);
		TNULL->color = false; // if false then its black
		TNULL->left = nullptr;
		TNULL->right = nullptr;
	}
	// Helper functions for rotations
	// Left Rotate for node x
	// x
	void leftRotate(RBNode *&x)
	{
		RBNode *y = x->right;
		x->right = y->left;
		if (y->left != TNULL)
			y->left->parent = x;

		y->parent = x->parent;
		// if x is root
		if (x->parent == nullptr)
			this->root = y;
		// else if x is left child
		else if (x == x->parent->left)
			x->parent->left = y;
		// else x is right child
		else
			x->parent->right = y;
		y->left = x;
		x->parent = y;
	}
	// Right Rotate for node x
	// same as leftRotate but inverted
	void rightRotate(RBNode *&x)
	{
		RBNode *y = x->left;
		x->left = y->right;
		if (y->right != TNULL)
			y->right->parent = x;

		y->parent = x->parent;
		// if x is root
		if (x->parent == nullptr)
			this->root = y;
		// else if x is right child
		else if (x == x->parent->right)
			x->parent->right = y;
		// else x is left child
		else
			x->parent->left = y;
		y->right = x;
		x->parent = y;
	}

	void RBFixInsert(RBNode *&k)
	{
		RBNode *u; // uncle

		while (k->parent->color == true)
		{
			if (k->parent == k->parent->parent->right)
			{
				u = k->parent->parent->left; // uncle
				// second senario
				if (u->color == true)
				{
					// recoloring parent and uncle to black and grandparent to red
					u->color = false;
					k->parent->color = false;
					k->parent->parent->color = true;
					k = k->parent->parent;
				}
				// third senarios
				// note third senario leads to fourth senario
				else
				{
					if (k == k->parent->left)
					{
						// rotate at the oppostite direction of the node to form a line
						k = k->parent;
						rightRotate(k);
					}
					// forth senario
					// rotate grandparent (the oppostite direction) of the node and swap colors of parent and grandparent
					k->parent->color = false;
					k->parent->parent->color = true;
					leftRotate(k->parent->parent);
				}
			}
			// same as above but inverted
			else
			{
				u = k->parent->parent->right; // uncle

				if (u->color == true)
				{
					// mirror case 3.1
					u->color = false;
					k->parent->color = false;
					k->parent->parent->color = true;
					k = k->parent->parent;
				}
				else
				{
					if (k == k->parent->right)
					{
						// mirror case 3.2.2
						k = k->parent;
						leftRotate(k);
					}
					// mirror case 3.2.1
					k->parent->color = false;
					k->parent->parent->color = true;
					rightRotate(k->parent->parent);
				}
			}
			if (k == root)
				break;
		}
		root->color = false;
	}

	// Helper Functions for Deletion

	// searching a node
	RBNode *searchNode(RBNode *node, int itemID)
	{
		if (node == TNULL || node == nullptr)
			return nullptr;
		if (node->itemID == itemID)
			return node;
		// since the tree is ordered by price first then itemID we need to search both sides
		RBNode *leftSearch = searchNode(node->left, itemID);
		if (leftSearch != nullptr)
			return leftSearch;
		return searchNode(node->right, itemID);
	}

	// finding successor
	RBNode *successor(RBNode *node)
	{
		RBNode *current = node;
		while (current->left != TNULL)
		{
			current = current->left;
		}
		return current;
	}

	// we need to replace nodes during deletion
	// node u will be replaced by node v
	void rbTransplant(RBNode *u, RBNode *v)
	{
		if (u->parent == nullptr)
		{
			root = v;
		}
		else if (u == u->parent->left)
		{
			u->parent->left = v;
		}
		else
		{
			u->parent->right = v;
		}
		v->parent = u->parent;
	}
	// deleting a black node may violate RB properties
	// while deleting a red node will not violate any properties

	// there are multiple scenarios to fix violations after deletion
	// 1) sibling is red ===> recolor sibling and parent and do rotation on parent
	// 2) sibling is black with two black children ===> recolor sibling to red
	// 3) sibling is black with at least one red child
	//    3.1) sibling's right child is black ===> recolor sibling and its left child and do rotation on sibling
	//    3.2) sibling's right child is red ===> recolor sibling and parent and sibling's right child and do rotation on parent
	void RBFixDelete(RBNode *x)
	{
		RBNode *s;
		while (x != root && x->color == false)
		{
			if (x == x->parent->left)
			{
				s = x->parent->right;
				// Case 1: sibling is red
				if (s->color == true)
				{
					// we recolor sibling and parent and do left rotate on parent
					s->color = false;
					x->parent->color = true;
					leftRotate(x->parent);
					s = x->parent->right;
				}

				// Case 2: sibling is black with two black children
				if (s->left->color == false && s->right->color == false)
				{
					// recolor sibling to red
					s->color = true;
					x = x->parent;
				}
				// Case 3: sibling is black with at least one red child
				else
				{
					// Case 3.1: sibling's right child is black
					if (s->right->color == false)
					{
						// recolor sibling and its left child and do right rotate on sibling
						s->left->color = false;
						s->color = true;
						rightRotate(s);
						s = x->parent->right;
					}
					// Case 3.2: sibling's right child is red
					// recolor sibling and parent and sibling's right child
					s->color = x->parent->color;
					x->parent->color = false;
					s->right->color = false;
					leftRotate(x->parent);
					x = root;
				}
			}
			// same as above but inverted
			else
			{
				s = x->parent->left;
				if (s->color == true)
				{
					s->color = false;
					x->parent->color = true;
					rightRotate(x->parent);
					s = x->parent->left;
				}

				if (s->right->color == false && s->left->color == false)
				{
					s->color = true;
					x = x->parent;
				}
				else
				{
					if (s->left->color == false)
					{
						s->right->color = false;
						s->color = true;
						leftRotate(s);
						s = x->parent->left;
					}

					s->color = x->parent->color;
					x->parent->color = false;
					s->left->color = false;
					rightRotate(x->parent);
					x = root;
				}
			}
		}
		x->color = false;
	}

public:
	ConcreteAuctionTree()
	{
		// TODO: Initialize your Red-Black Tree
		initializeTNULL();
		root = TNULL;
	}

	// strategy for insertion
	// first do normal BST insert and color the new node red
	// then fix any violations of RB properties

	// there are four scenarios
	// 1) the node is the root ===> just color it black
	// 2) the uncle is red ===> recolor parent and uncle to black and grandparent to red
	// 3) the uncle is black and form a triangle (> or <) ==> rotate parent to the opposite direction of the node to form a line
	// 4) the uncle is black and form a line (\ or /) ==> rotate grandparent to the opposite direction of the node and swap colors of parent and grandparent

	void insertItem(int itemID, int price) override
	{
		// TODO: Implement Red-Black Tree insertion
		// Remember to maintain RB-Tree properties with rotations and recoloring
		RBNode *node = new RBNode(itemID, price);
		node->parent = nullptr;
		node->left = TNULL;
		node->right = TNULL;
		node->color = true; // remember True means Red

		// now a standard BST insertion
		RBNode *y = nullptr;
		RBNode *x = this->root;
		while (x != TNULL)
		{
			// we should compare Price first then itemID for uniqueness
			y = x;
			if (node->price < x->price)
				x = x->left;
			else if (node->price > x->price)
				x = x->right;
			else
			{
				// prices are equal, compare itemID
				if (node->itemID < x->itemID)
					x = x->left;
				else if (node->itemID > x->itemID)
					x = x->right;
				else
				{
					// for edge test case
					// duplicate itemID, do not insert
					delete node;
					return;
				}
			}
		}
		// now linking the pointers
		node->parent = y;
		if (y == nullptr)
			root = node;
		else if (node->price < y->price)
			y->left = node;
		else if (node->price > y->price)
			y->right = node;
		else
		{
			if (node->itemID < y->itemID)
				y->left = node;
			else
				y->right = node;
		}
		if (node->parent == nullptr)
		{
			node->color = false; // if root color it black
			return;
		}
		else if (node->parent->parent == nullptr)
			return;

		// fixing violations
		RBFixInsert(node);
	}

	// we have multiple scenarios for deletion
	// 1) node to be deleted has no children ===> just remove it
	// 2) node to be deleted has one child ===> replace node with its child
	// 3) node to be deleted has two children ===> find its successor, replace the node with its successor, and replace successor with its right child

	void deleteItem(int itemID) override
	{
		// TODO: Implement Red-Black Tree deletion
		// This is complex - handle all cases carefully
		RBNode *nodeToBeDeleted = searchNode(this->root, itemID);
		if (nodeToBeDeleted == nullptr)
			return; // item not found
		RBNode *y = nodeToBeDeleted;
		RBNode *x;
		bool yOriginalColor = y->color;
		if (nodeToBeDeleted->left == TNULL)
		{
			x = nodeToBeDeleted->right;
			rbTransplant(nodeToBeDeleted, nodeToBeDeleted->right);
		}
		else if (nodeToBeDeleted->right == TNULL)
		{
			x = nodeToBeDeleted->left;
			rbTransplant(nodeToBeDeleted, nodeToBeDeleted->left);
		}
		else
		{
			y = successor(nodeToBeDeleted->right);
			yOriginalColor = y->color;
			x = y->right;
			if (y->parent == nodeToBeDeleted)
			{
				x->parent = y;
			}
			else
			{
				rbTransplant(y, y->right);
				y->right = nodeToBeDeleted->right;
				y->right->parent = y;
			}

			rbTransplant(nodeToBeDeleted, y);
			y->left = nodeToBeDeleted->left;
			y->left->parent = y;
			y->color = nodeToBeDeleted->color;
		}
		delete nodeToBeDeleted;
		if (yOriginalColor == false) // if the deleted node was black then fix violations
		{
			RBFixDelete(x);
		}
	}
};

// =========================================================
// PART B: INVENTORY SYSTEM (Dynamic Programming)
// =========================================================

int InventorySystem::optimizeLootSplit(int n, vector<int> &coins)
{

	int target = 0;

	for (auto c : coins)
	{
		target += c;
	}
	target = ceil(target / 2.);

	vector<vector<int>> dp(coins.size() + 1);

	for (int i = 0; i <= coins.size(); i++)
	{
		dp[i] = vector<int>(target + 1);
	}

	for (int i = 0; i <= coins.size(); i++)
	{
		for (int j = 0; j <= target; j++)
		{
			if (i == 0 || j == 0)
			{
				dp[i][j] = 0;
			}
			else
			{
				if (coins[i - 1] > j)
				{
					dp[i][j] = dp[i - 1][j];
				}
				else
				{
					dp[i][j] = max(dp[i - 1][j], coins[i - 1] + dp[i - 1][j - coins[i - 1]]);
				}
			}
		}
	}

	return abs(dp[coins.size() - 1][target] - target);
}

int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int>> &items)
{
	vector<vector<int>> dp(items.size() + 1);

	for (int i = 0; i <= items.size(); i++)
	{
		dp[i] = vector<int>(capacity + 1);
	}

	for (int i = 0; i <= items.size(); i++)
	{
		for (int j = 0; j <= capacity; j++)
		{
			if (i == 0 || j == 0)
			{
				dp[i][j] = 0;
			}
			else
			{
				if (items[i - 1].first > j)
				{
					dp[i][j] = dp[i - 1][j];
				}
				else
				{
					dp[i][j] = max(dp[i - 1][j], items[i - 1].second + dp[i - 1][j - items[i - 1].first]);
				}
			}
		}
	}

	return dp[items.size()][capacity];
}

long long InventorySystem::countStringPossibilities(string s)
{
	int res = 1;
	for (int i = 1; i < s.size(); i++)
	{
		if ((s[i] == 'u' || s[i] == 'n') && s[i] == s[i - 1])
		{
			res *= 2;
		}
	}
	return res;
}

// =========================================================
// PART C: WORLD NAVIGATOR (Graphs)
// =========================================================

bool WorldNavigator::pathExists(int n, vector<vector<int>> &edges, int source, int dest)
{
	if (n == 0)
		return false;
	if (source < 0 || source >= n || dest < 0 || dest >= n)
		return false;
	if (source == dest)
		return true;

	vector<vector<int>> adj(n);
	for (auto &e : edges)
	{
		adj[e[0]].push_back(e[1]);
		adj[e[1]].push_back(e[0]);
	}

	vector<bool> visited(n, false);
	queue<int> q;
	q.push(source);
	visited[source] = true;

	while (!q.empty())
	{
		int u = q.front();
		q.pop();
		for (int v : adj[u])
		{
			if (!visited[v])
			{
				if (v == dest)
					return true;
				visited[v] = true;
				q.push(v);
			}
		}
	}
	return false;
}

long long WorldNavigator::minBribeCost(int n, int m, long long goldRate, long long silverRate, vector<vector<int>> &roadData)
{
	if (n == 0)
		return -1;
	if (n == 1)
		return 0;
	if (m == 0)
		return -1;

	vector<tuple<long long, int, int>> edges;
	for (auto &r : roadData)
	{
		if (r[0] == r[1])
			continue;
		long long cost = r[2] * goldRate + r[3] * silverRate;
		edges.push_back(make_tuple(cost, r[0], r[1]));
	}

	sort(edges.begin(), edges.end());

	vector<int> parent(n), rank(n, 0);
	for (int i = 0; i < n; i++)
		parent[i] = i;

	function<int(int)> find = [&](int x)
	{
		return parent[x] == x ? x : parent[x] = find(parent[x]);
	};

	auto unite = [&](int a, int b)
	{
		a = find(a);
		b = find(b);
		if (a != b)
		{
			if (rank[a] < rank[b])
				swap(a, b);
			parent[b] = a;
			if (rank[a] == rank[b])
				rank[a]++;
			return true;
		}
		return false;
	};

	long long total = 0;
	int used = 0;

	for (auto &e : edges)
	{
		long long cost = get<0>(e);
		int u = get<1>(e);
		int v = get<2>(e);

		if (unite(u, v))
		{
			total += cost;
			used++;
		}
	}

	return (used == n - 1) ? total : -1;
}

string WorldNavigator::sumMinDistancesBinary(int n, vector<vector<int>> &roads)
{
	if (n <= 1)
		return "0";

	const long long INF = LLONG_MAX / 4;
	vector<vector<long long>> dist(n, vector<long long>(n, INF));

	for (int i = 0; i < n; i++)
		dist[i][i] = 0;

	for (auto &r : roads)
	{
		dist[r[0]][r[1]] = min(dist[r[0]][r[1]], (long long)r[2]);
		dist[r[1]][r[0]] = min(dist[r[1]][r[0]], (long long)r[2]);
	}

	for (int k = 0; k < n; k++)
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				if (dist[i][k] < INF && dist[k][j] < INF)
					dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);

	long long sum = 0;
	for (int i = 0; i < n; i++)
		for (int j = i + 1; j < n; j++)
			if (dist[i][j] < INF)
				sum += dist[i][j];

	if (sum == 0)
		return "0";

	string binary = "";
	while (sum > 0)
	{
		binary.push_back((sum & 1) + '0');
		sum >>= 1;
	}
	reverse(binary.begin(), binary.end());
	return binary;
}

// =========================================================
// PART D: SERVER KERNEL (Greedy)
// =========================================================

int ServerKernel::minIntervals(vector<char> &tasks, int n)
{

	// error handling
	if (n < 0)
	{
		cout << "Error: Cooling interval 'n' cannot be negative." << endl;
		return 0;
	}

	if (tasks.empty())
	{
		return 0;
	}

	// if no interval then tasks are done back to back
	if (n == 0)
	{
		return tasks.size();
	}

	// calc frequency of each letter
	vector<int> freq(26, 0);
	int maxFreq = 0;
	int totalValidTasks = 0;

	for (char c : tasks)
	{
		// ignore invalid tasks
		if (c < 'A' || c > 'Z')
		{
			cout << "Warning: Invalid task '" << c << "' ignored. Only 'A'-'Z' allowed." << endl;
			continue;
		}

		int index = c - 'A';
		freq[index]++;

		// Track the highest frequency found so far
		if (freq[index] > maxFreq)
		{
			maxFreq = freq[index];
		}
		totalValidTasks++;
	}

	// if all cases are invalid
	if (totalValidTasks == 0)
	{
		return 0;
	}

	// Greedy part
	// count how many tasks have the same max frequency
	int numMaxFreqTasks = 0;
	for (int f : freq)
	{
		if (f == maxFreq)
		{
			numMaxFreqTasks++;
		}
	}

	int partCount = maxFreq - 1;
	int partLength = n + 1;

	// formula: result = (maxFreq - 1) * (n-1) + number of maxFreq tasks
	int minSlots = partCount * partLength + numMaxFreqTasks;

	// final result
	return max(minSlots, totalValidTasks);
}

// =========================================================
// FACTORY FUNCTIONS (Required for Testing)
// =========================================================

extern "C"
{
	PlayerTable *createPlayerTable()
	{
		return new ConcretePlayerTable();
	}

	Leaderboard *createLeaderboard()
	{
		return new ConcreteLeaderboard();
	}

	AuctionTree *createAuctionTree()
	{
		return new ConcreteAuctionTree();
	}
}
