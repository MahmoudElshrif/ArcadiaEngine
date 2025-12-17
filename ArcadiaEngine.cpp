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
    // TODO: Define your data structures here
    // Hint: You'll need a hash table with double hashing collision resolution

public:
    ConcretePlayerTable()
    {
        // TODO: Initialize your hash table
    }

    void insert(int playerID, string name) override
    {
        // TODO: Implement double hashing insert
        // Remember to handle collisions using h1(key) + i * h2(key)
    }

    string search(int playerID) override
    {
        // TODO: Implement double hashing search
        // Return "" if player not found
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
        return id1 < id2;           // lower playerID first if scores are equal
    }

public:
    ConcreteLeaderboard()
    {
        // TODO: Initialize your skip list
        maxLevel = 16;    // let max level be 16
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
            newNode->forward[i] = update[i]->forward[i];                         // link new node to next node
            update[i]->forward[i] = newNode;                                     // link previous node to new node
        }
    }

    void removePlayer(int playerID) override
    {
        // TODO: Implement skip list deletion
        SkipListNode *currentNode = head;
        SkipListNode *toBeDelete = nullptr;
        // searching in level 0
        while(currentNode!=nullptr){
            if (currentNode->forward[0]!=nullptr && currentNode->forward[0]->playerID == playerID){
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
        int size=n;
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

public:
    ConcreteAuctionTree()
    {
        // TODO: Initialize your Red-Black Tree
        initializeTNULL();
        root = TNULL;
    }

    void insertItem(int itemID, int price) override
    {
        // TODO: Implement Red-Black Tree insertion
        // Remember to maintain RB-Tree properties with rotations and recoloring
    }

    void deleteItem(int itemID) override
    {
        // TODO: Implement Red-Black Tree deletion
        // This is complex - handle all cases carefully
    }
};

// =========================================================
// PART B: INVENTORY SYSTEM (Dynamic Programming)
// =========================================================

int InventorySystem::optimizeLootSplit(int n, vector<int> &coins)
{
    // TODO: Implement partition problem using DP
    // Goal: Minimize |sum(subset1) - sum(subset2)|
    // Hint: Use subset sum DP to find closest sum to total/2
    return 0;
}

int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int>> &items)
{
    // TODO: Implement 0/1 Knapsack using DP
    // items = {weight, value} pairs
    // Return maximum value achievable within capacity
    return 0;
}

long long InventorySystem::countStringPossibilities(string s)
{
    // TODO: Implement string decoding DP
    // Rules: "uu" can be decoded as "w" or "uu"
    //        "nn" can be decoded as "m" or "nn"
    // Count total possible decodings
    return 0;
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
    // TODO: Implement task scheduler with cooling time
    // Same task must wait 'n' intervals before running again
    // Return minimum total intervals needed (including idle time)
    // Hint: Use greedy approach with frequency counting
    return 0;
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
