#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include "enums.h"

using namespace std;

// B+ Tree node structure
class BPlusTreeNode {
public:
    bool isLeaf;
    vector<int> keys;
    vector<vector<int>> rowIds;  // For leaf nodes, stores row IDs for each key
    vector<BPlusTreeNode*> children;  // For internal nodes
    BPlusTreeNode* next;  // For leaf nodes, points to the next leaf

    BPlusTreeNode(bool leaf = false) {
        isLeaf = leaf;
        next = nullptr;
    }

    ~BPlusTreeNode() {
        try {
            // Clean up children if this is an internal node
            if (!isLeaf) {
                for (auto child : children) {
                    if (child != nullptr) {
                        delete child;
                    }
                }
            }
            // Clear vectors to free memory
            keys.clear();
            rowIds.clear();
            children.clear();
        } catch (const exception& e) {
            cout << "ERROR: Exception in BPlusTreeNode destructor: " << e.what() << endl;
        }
    }
};

// B+ Tree class
class BPlusTree {
private:
    BPlusTreeNode* root;
    int order;  // Maximum number of children per node
    string tableName;
    string columnName;
    string indexFileName;

    // Helper function to find the appropriate leaf node for a key
    BPlusTreeNode* findLeafNode(int key) {
        if (!root) {
            return nullptr;
        }
        
        BPlusTreeNode* current = root;
        
        try {
            // Traverse the tree to find the leaf node
            while (current && !current->isLeaf) {
                // Safety check for empty keys
                if (current->keys.empty()) {
                    // If there are no keys but there are children, go to the first child
                    if (!current->children.empty()) {
                        current = current->children[0];
                        continue;
                    } else {
                        // No keys and no children, this is an invalid node
                        return nullptr;
                    }
                }
                
                int i = 0;
                while (i < current->keys.size() && key >= current->keys[i]) {
                    i++;
                }
                
                // Safety check to avoid segmentation fault
                if (current->children.empty()) {
                    // This should not happen in a well-formed B+ tree
                    return nullptr;
                }
                
                if (i >= current->children.size()) {
                    // This should not happen in a well-formed B+ tree
                    // But we'll handle it gracefully by using the last child
                    i = current->children.size() - 1;
                }
                
                current = current->children[i];
            }
        } catch (const exception& e) {
            cout << "ERROR: Exception in findLeafNode: " << e.what() << endl;
            return nullptr;
        }
        
        return current;
    }

    // Helper function to insert a key-value pair into a leaf node
    void insertIntoLeaf(BPlusTreeNode* leaf, int key, int rowId) {
        try {
            // Check if leaf is valid
            if (!leaf) {
                cout << "ERROR: Null leaf node in insertIntoLeaf" << endl;
                return;
            }
            
            // Find the position to insert the key
            int i = 0;
            while (i < leaf->keys.size() && leaf->keys[i] < key) {
                i++;
            }
            
            // Insert the key at position i
            if (i < leaf->keys.size() && leaf->keys[i] == key) {
                // Key already exists, add the row ID to the existing key
                // Make sure rowIds is properly sized
                if (i >= leaf->rowIds.size()) {
                    cout << "WARNING: rowIds index out of bounds, resizing" << endl;
                    leaf->rowIds.resize(leaf->keys.size());
                }
                
                // Check if this rowId is already in the list to avoid duplicates
                if (find(leaf->rowIds[i].begin(), leaf->rowIds[i].end(), rowId) == leaf->rowIds[i].end()) {
                    leaf->rowIds[i].push_back(rowId);
                }
            } else {
                // Insert new key and row ID
                leaf->keys.insert(leaf->keys.begin() + i, key);
                vector<int> newRowIds = {rowId};
                
                // Make sure rowIds is properly sized
                if (leaf->rowIds.size() < leaf->keys.size() - 1) {
                    leaf->rowIds.resize(leaf->keys.size() - 1);
                }
                
                // Insert the new rowIds at position i
                if (i <= leaf->rowIds.size()) {
                    leaf->rowIds.insert(leaf->rowIds.begin() + i, newRowIds);
                } else {
                    cout << "WARNING: rowIds insertion index out of bounds, appending" << endl;
                    leaf->rowIds.push_back(newRowIds);
                }
            }
        } catch (const exception& e) {
            cout << "ERROR: Exception in insertIntoLeaf: " << e.what() << endl;
        }
    }

    // Helper function to split a leaf node
    BPlusTreeNode* splitLeafNode(BPlusTreeNode* leaf) {
        // Check if the leaf is valid
        if (!leaf || leaf->keys.empty()) {
            return new BPlusTreeNode(true);
        }
        
        // Create a new leaf node
        BPlusTreeNode* newLeaf = new BPlusTreeNode(true);
        
        // Calculate the middle index
        int mid = leaf->keys.size() / 2;
        
        // Make sure mid is valid
        if (mid <= 0 || mid >= leaf->keys.size()) {
            mid = leaf->keys.size() / 2;
            if (mid <= 0) {
                mid = 1;  // At least move one element
            }
            if (mid >= leaf->keys.size()) {
                mid = leaf->keys.size() - 1;  // Leave at least one element
            }
        }
        
        // Special case: If we have very few distinct values (like binary 0/1)
        // and they're all the same, we need a different splitting strategy
        bool allSameKeys = true;
        for (size_t i = 1; i < leaf->keys.size(); i++) {
            if (leaf->keys[i] != leaf->keys[0]) {
                allSameKeys = false;
                break;
            }
        }
        
        if (allSameKeys && leaf->keys.size() > 1) {
            // If all keys are the same, split by row IDs instead
            int totalRows = 0;
            for (const auto& rows : leaf->rowIds) {
                totalRows += rows.size();
            }
            
            int halfRows = totalRows / 2;
            int currentRows = 0;
            int splitIndex = 0;
            
            for (size_t i = 0; i < leaf->rowIds.size(); i++) {
                currentRows += leaf->rowIds[i].size();
                if (currentRows >= halfRows) {
                    splitIndex = i + 1;
                    break;
                }
            }
            
            // Ensure we have a valid split index
            if (splitIndex <= 0) splitIndex = 1;
            if (splitIndex >= leaf->keys.size()) splitIndex = leaf->keys.size() - 1;
            
            // Use this index for splitting
            mid = splitIndex;
        }
        
        // Make sure we have enough elements to split
        if (mid <= 0 || mid >= leaf->keys.size()) {
            // Can't split properly, return an empty leaf
            return newLeaf;
        }
        
        // Move half of the keys and values to the new leaf
        newLeaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
        newLeaf->rowIds.assign(leaf->rowIds.begin() + mid, leaf->rowIds.end());
        
        // Resize the original leaf
        leaf->keys.resize(mid);
        leaf->rowIds.resize(mid);
        
        // Update the next pointers
        newLeaf->next = leaf->next;
        leaf->next = newLeaf;
        
        return newLeaf;
    }

    // Helper function to insert a key into an internal node
    void insertIntoInternal(BPlusTreeNode* node, int key, BPlusTreeNode* child) {
        // Check if the node is valid
        if (!node || !child) {
            return;
        }
        
        // Find the position to insert the key
        int i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            i++;
        }
        
        // Insert the key and child pointer
        node->keys.insert(node->keys.begin() + i, key);
        
        // Make sure i+1 is a valid position
        if (i + 1 > node->children.size()) {
            node->children.push_back(child);
        } else {
            node->children.insert(node->children.begin() + i + 1, child);
        }
    }

    // Helper function to split an internal node
    pair<BPlusTreeNode*, int> splitInternalNode(BPlusTreeNode* node) {
        // Check if the node is valid
        if (!node || node->keys.empty() || node->children.empty()) {
            return make_pair(new BPlusTreeNode(false), 0);
        }
        
        // Create a new internal node
        BPlusTreeNode* newNode = new BPlusTreeNode(false);
        
        // Calculate the middle index
        int mid = node->keys.size() / 2;
        
        // Make sure mid is valid
        if (mid < 0 || mid >= node->keys.size()) {
            return make_pair(newNode, 0);
        }
        
        // Get the middle key (to be moved up)
        int midKey = node->keys[mid];
        
        // Make sure we have enough elements to split
        if (mid + 1 >= node->keys.size() || mid + 1 >= node->children.size()) {
            // Can't split properly, return an empty node
            return make_pair(newNode, midKey);
        }
        
        // Move half of the keys and children to the new node
        newNode->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
        newNode->children.assign(node->children.begin() + mid + 1, node->children.end());
        
        // Resize the original node
        node->keys.resize(mid);
        node->children.resize(mid + 1);
        
        return make_pair(newNode, midKey);
    }

    // Helper function to save a node to disk
    void saveNodeToDisk(BPlusTreeNode* node, ofstream& outFile) {
        // Check if the node is valid
        if (!node) {
            // Write a placeholder for null nodes
            bool isNull = true;
            outFile.write(reinterpret_cast<char*>(&isNull), sizeof(bool));
            return;
        }
        
        // Write that this is a valid node
        bool isNull = false;
        outFile.write(reinterpret_cast<char*>(&isNull), sizeof(bool));
        
        // Write whether the node is a leaf
        outFile.write(reinterpret_cast<char*>(&node->isLeaf), sizeof(bool));
        
        // Write the number of keys
        int keyCount = node->keys.size();
        outFile.write(reinterpret_cast<char*>(&keyCount), sizeof(int));
        
        // Write the keys
        for (int key : node->keys) {
            outFile.write(reinterpret_cast<char*>(&key), sizeof(int));
        }
        
        if (node->isLeaf) {
            // For leaf nodes, write the row IDs
            for (int i = 0; i < keyCount; i++) {
                // Make sure rowIds is valid
                if (i >= node->rowIds.size()) {
                    // Write an empty rowIds list
                    int rowIdCount = 0;
                    outFile.write(reinterpret_cast<char*>(&rowIdCount), sizeof(int));
                    continue;
                }
                
                int rowIdCount = node->rowIds[i].size();
                outFile.write(reinterpret_cast<char*>(&rowIdCount), sizeof(int));
                
                for (int rowId : node->rowIds[i]) {
                    outFile.write(reinterpret_cast<char*>(&rowId), sizeof(int));
                }
            }
        } else {
            // For internal nodes, recursively save the children
            int childCount = node->children.size();
            outFile.write(reinterpret_cast<char*>(&childCount), sizeof(int));
            
            for (BPlusTreeNode* child : node->children) {
                saveNodeToDisk(child, outFile);
            }
        }
    }

    // Helper function to load a node from disk
    BPlusTreeNode* loadNodeFromDisk(ifstream& inFile) {
        // Check if this is a null node
        bool isNull;
        inFile.read(reinterpret_cast<char*>(&isNull), sizeof(bool));
        
        if (isNull) {
            return nullptr;
        }
        
        // Read whether the node is a leaf
        bool isLeaf;
        inFile.read(reinterpret_cast<char*>(&isLeaf), sizeof(bool));
        
        BPlusTreeNode* node = new BPlusTreeNode(isLeaf);
        
        // Read the number of keys
        int keyCount;
        inFile.read(reinterpret_cast<char*>(&keyCount), sizeof(int));
        
        // Sanity check for keyCount
        if (keyCount < 0 || keyCount > 1000) {  // Arbitrary upper limit
            delete node;
            throw runtime_error("Invalid key count in B+ tree node");
        }
        
        // Read the keys
        for (int i = 0; i < keyCount; i++) {
            int key;
            inFile.read(reinterpret_cast<char*>(&key), sizeof(int));
            node->keys.push_back(key);
        }
        
        if (isLeaf) {
            // For leaf nodes, read the row IDs
            for (int i = 0; i < keyCount; i++) {
                int rowIdCount;
                inFile.read(reinterpret_cast<char*>(&rowIdCount), sizeof(int));
                
                // Sanity check for rowIdCount
                if (rowIdCount < 0 || rowIdCount > 10000) {  // Arbitrary upper limit
                    delete node;
                    throw runtime_error("Invalid row ID count in B+ tree node");
                }
                
                vector<int> rowIds;
                for (int j = 0; j < rowIdCount; j++) {
                    int rowId;
                    inFile.read(reinterpret_cast<char*>(&rowId), sizeof(int));
                    rowIds.push_back(rowId);
                }
                
                node->rowIds.push_back(rowIds);
            }
        } else {
            // Read the number of children
            int childCount;
            inFile.read(reinterpret_cast<char*>(&childCount), sizeof(int));
            
            // Sanity check for childCount
            if (childCount < 0 || childCount > 1000) {  // Arbitrary upper limit
                delete node;
                throw runtime_error("Invalid child count in B+ tree node");
            }
            
            // For internal nodes, recursively load the children
            for (int i = 0; i < childCount; i++) {
                BPlusTreeNode* child = loadNodeFromDisk(inFile);
                node->children.push_back(child);
            }
        }
        
        return node;
    }

    // Helper function to connect leaf nodes after loading
    void connectLeafNodes() {
        // Check if the tree is empty
        if (!root) {
            return;
        }
        
        // Find the leftmost leaf
        BPlusTreeNode* current = root;
        
        // If the root is already a leaf, we're done
        if (current->isLeaf) {
            return;
        }
        
        // Navigate to the leftmost leaf
        while (!current->isLeaf) {
            // Check if the node has children
            if (current->children.empty()) {
                return;
            }
            current = current->children[0];
        }
        
        // Collect all leaf nodes in order
        vector<BPlusTreeNode*> leafNodes;
        collectLeafNodes(root, leafNodes);
        
        // Connect the leaf nodes
        for (size_t i = 0; i < leafNodes.size() - 1; i++) {
            leafNodes[i]->next = leafNodes[i + 1];
        }
        
        // The last leaf node's next pointer should be null
        if (!leafNodes.empty()) {
            leafNodes.back()->next = nullptr;
        }
    }
    
    // Helper function to collect all leaf nodes in order
    void collectLeafNodes(BPlusTreeNode* node, vector<BPlusTreeNode*>& leafNodes) {
        if (!node) {
            return;
        }
        
        if (node->isLeaf) {
            leafNodes.push_back(node);
            return;
        }
        
        // Recursively collect leaf nodes from all children
        for (BPlusTreeNode* child : node->children) {
            collectLeafNodes(child, leafNodes);
        }
    }

    // Helper function to find the parent of a node
    BPlusTreeNode* findParent(BPlusTreeNode* node, BPlusTreeNode* child) {
        if (node == nullptr || node->isLeaf) {
            return nullptr;
        }
        
        // Check if any of the children is the target
        for (BPlusTreeNode* nodeChild : node->children) {
            if (nodeChild == child) {
                return node;
            }
        }
        
        // Recursively check each child
        for (BPlusTreeNode* nodeChild : node->children) {
            BPlusTreeNode* parent = findParent(nodeChild, child);
            if (parent != nullptr) {
                return parent;
            }
        }
        
        return nullptr;
    }

public:
    BPlusTree(int order, string tableName, string columnName) {
        try {
            if (order < 2) {
                order = 4;  // Default to a reasonable value if invalid
            }
            
            this->order = order;
            this->root = nullptr;  // Initialize to nullptr, will create when needed
            this->tableName = tableName;
            this->columnName = columnName;
            
            // Make sure the data/temp directory exists
            string dataDir = "../data/temp/";
            
            // Create the directory if it doesn't exist
            system("mkdir -p ../data/temp");
            
            // Set the index file path to be in the data/temp directory
            this->indexFileName = dataDir + tableName + "_" + columnName + "_bptree.idx";
            
            // Create a minimal root node if needed
            if (!this->root) {
                this->root = new BPlusTreeNode(true);
            }
            
            // cout << "DEBUG: BPlusTree constructor completed successfully for " << tableName << "." << columnName << endl;
        } catch (const exception& e) {
            cout << "ERROR: Exception in BPlusTree constructor: " << e.what() << endl;
            // Ensure we have a valid root
            this->root = new BPlusTreeNode(true);
        }
    }

    ~BPlusTree() {
        try {
            // cout << "DEBUG: BPlusTree destructor called for " << tableName << "." << columnName << endl;
            // Clean up the tree
            if (root != nullptr) {
                delete root;
                root = nullptr;
            }
            // cout << "DEBUG: BPlusTree destructor completed for " << tableName << "." << columnName << endl;
        } catch (const exception& e) {
            cout << "ERROR: Exception in BPlusTree destructor: " << e.what() << endl;
        }
    }

    // Insert a key-value pair into the tree
    void insert(int key, int rowId) {
        try {
            // cout << "DEBUG: Inserting key " << key << " with rowId " << rowId << endl;
            
            // If the root is null, create a new root
            if (!root) {
                // cout << "DEBUG: Creating new root node" << endl;
                try {
                    root = new BPlusTreeNode(true);
                    if (!root) {
                        cout << "ERROR: Failed to create new root node" << endl;
                        return;
                    }
                } catch (const std::bad_alloc& e) {
                    cout << "ERROR: Memory allocation failed for new root node: " << e.what() << endl;
                    return;
                } catch (const exception& e) {
                    cout << "ERROR: Exception creating new root node: " << e.what() << endl;
                    return;
                }
            }
            
            // If the root is a leaf node
            if (root->isLeaf) {
                // cout << "DEBUG: Root is a leaf node, inserting directly" << endl;
                try {
                    insertIntoLeaf(root, key, rowId);
                } catch (const exception& e) {
                    cout << "ERROR: Exception in insertIntoLeaf for root: " << e.what() << endl;
                    return;
                }
                
                // Check if the root needs to be split
                if (root->keys.size() >= order) {
                    // cout << "DEBUG: Root needs to be split" << endl;
                    try {
                        // Split the root
                        BPlusTreeNode* newLeaf = splitLeafNode(root);
                        if (!newLeaf) {
                            cout << "ERROR: Failed to split root leaf node" << endl;
                            return;
                        }
                        
                        // Create a new root
                        BPlusTreeNode* newRoot = new BPlusTreeNode(false);
                        if (!newRoot) {
                            cout << "ERROR: Failed to create new root after split" << endl;
                            delete newLeaf;
                            return;
                        }
                        
                        // Make sure the new leaf has keys before accessing them
                        if (!newLeaf->keys.empty()) {
                            newRoot->keys.push_back(newLeaf->keys[0]);
                            newRoot->children.push_back(root);
                            newRoot->children.push_back(newLeaf);
                            
                            // Update the root
                            root = newRoot;
                            // cout << "DEBUG: Created new root after splitting" << endl;
                        } else {
                            cout << "WARNING: New leaf has no keys after splitting" << endl;
                            delete newRoot;
                            delete newLeaf;
                        }
                    } catch (const exception& e) {
                        cout << "ERROR: Exception while splitting root: " << e.what() << endl;
                    }
                }
            } else {
                // cout << "DEBUG: Root is an internal node, finding leaf node" << endl;
                // Find the leaf node where the key should be inserted
                BPlusTreeNode* leaf = nullptr;
                try {
                    leaf = findLeafNode(key);
                } catch (const exception& e) {
                    cout << "ERROR: Exception in findLeafNode: " << e.what() << endl;
                    // Create a new leaf as fallback
                    leaf = new BPlusTreeNode(true);
                    root->children.push_back(leaf);
                }
                
                // Check if leaf is valid
                if (!leaf) {
                    // cout << "DEBUG: Failed to find leaf node, handling special case" << endl;
                    try {
                        // This should not happen in a well-formed B+ tree
                        // But we'll handle it by inserting into the root
                        if (root->isLeaf) {
                            insertIntoLeaf(root, key, rowId);
                        } else {
                            // Create a new leaf node
                            BPlusTreeNode* newLeaf = new BPlusTreeNode(true);
                            if (!newLeaf) {
                                cout << "ERROR: Failed to create new leaf node" << endl;
                                return;
                            }
                            
                            vector<int> newRowIds = {rowId};
                            newLeaf->keys.push_back(key);
                            newLeaf->rowIds.push_back(newRowIds);
                            
                            // Add it as a child of the root
                            root->keys.push_back(key);
                            root->children.push_back(newLeaf);
                        }
                    } catch (const exception& e) {
                        cout << "ERROR: Exception handling special case: " << e.what() << endl;
                    }
                    return;
                }
                
                // cout << "DEBUG: Found leaf node, inserting key" << endl;
                // Insert the key into the leaf
                try {
                    insertIntoLeaf(leaf, key, rowId);
                } catch (const exception& e) {
                    cout << "ERROR: Exception in insertIntoLeaf: " << e.what() << endl;
                    return;
                }
                
                // Check if the leaf needs to be split
                if (leaf->keys.size() >= order) {
                    // cout << "DEBUG: Leaf node needs to be split" << endl;
                    try {
                        // Split the leaf
                        BPlusTreeNode* newLeaf = splitLeafNode(leaf);
                        if (!newLeaf) {
                            cout << "ERROR: Failed to split leaf node" << endl;
                            return;
                        }
                        
                        // Make sure the new leaf has keys before accessing them
                        if (newLeaf->keys.empty()) {
                            // cout << "DEBUG: New leaf has no keys after splitting, returning" << endl;
                            delete newLeaf;
                            return;
                        }
                        
                        // cout << "DEBUG: Finding path to leaf for updating parent" << endl;
                        // Insert the first key of the new leaf into the parent
                        int newKey = newLeaf->keys[0];
                        BPlusTreeNode* current = root;
                        vector<BPlusTreeNode*> path;
                        
                        // Find the path to the leaf
                        while (current && !current->isLeaf) {
                            path.push_back(current);
                            
                            int i = 0;
                            while (i < current->keys.size() && newKey >= current->keys[i]) {
                                i++;
                            }
                            
                            // Safety check
                            if (current->children.empty()) {
                                cout << "ERROR: Node has no children" << endl;
                                break;
                            }
                            
                            if (i >= current->children.size()) {
                                // cout << "DEBUG: Index out of bounds for children, using last child" << endl;
                                i = current->children.size() - 1;
                            }
                            
                            current = current->children[i];
                        }
                        
                        // Make sure we found a path
                        if (path.empty()) {
                            // cout << "DEBUG: Path to leaf is empty, creating new root" << endl;
                            // Create a new root
                            BPlusTreeNode* newRoot = new BPlusTreeNode(false);
                            if (!newRoot) {
                                cout << "ERROR: Failed to create new root" << endl;
                                delete newLeaf;
                                return;
                            }
                            
                            newRoot->keys.push_back(newKey);
                            newRoot->children.push_back(leaf);
                            newRoot->children.push_back(newLeaf);
                            
                            // Update the root
                            root = newRoot;
                            return;
                        }
                        
                        // cout << "DEBUG: Updating parent node" << endl;
                        // Insert the new key into the parent
                        BPlusTreeNode* parent = path.back();
                        try {
                            insertIntoInternal(parent, newKey, newLeaf);
                        } catch (const exception& e) {
                            cout << "ERROR: Exception in insertIntoInternal: " << e.what() << endl;
                            delete newLeaf;
                            return;
                        }
                        
                        // Check if the parent needs to be split
                        if (parent->keys.size() >= order) {
                            // cout << "DEBUG: Parent node needs to be split" << endl;
                            try {
                                // Split the parent
                                pair<BPlusTreeNode*, int> splitResult = splitInternalNode(parent);
                                BPlusTreeNode* newParent = splitResult.first;
                                int midKey = splitResult.second;
                                
                                if (!newParent) {
                                    cout << "ERROR: Failed to split parent node" << endl;
                                    return;
                                }
                                
                                // If the parent is the root, create a new root
                                if (parent == root) {
                                    // cout << "DEBUG: Parent is root, creating new root" << endl;
                                    BPlusTreeNode* newRoot = new BPlusTreeNode(false);
                                    if (!newRoot) {
                                        cout << "ERROR: Failed to create new root" << endl;
                                        delete newParent;
                                        return;
                                    }
                                    
                                    newRoot->keys.push_back(midKey);
                                    newRoot->children.push_back(parent);
                                    newRoot->children.push_back(newParent);
                                    
                                    // Update the root
                                    root = newRoot;
                                } else if (path.size() >= 2) {
                                    // cout << "DEBUG: Updating grandparent" << endl;
                                    // Insert the middle key into the grandparent
                                    BPlusTreeNode* grandparent = path[path.size() - 2];
                                    try {
                                        insertIntoInternal(grandparent, midKey, newParent);
                                    } catch (const exception& e) {
                                        cout << "ERROR: Exception in insertIntoInternal for grandparent: " << e.what() << endl;
                                        delete newParent;
                                        return;
                                    }
                                }
                            } catch (const exception& e) {
                                cout << "ERROR: Exception while splitting parent: " << e.what() << endl;
                            }
                        }
                    } catch (const exception& e) {
                        cout << "ERROR: Exception while handling leaf split: " << e.what() << endl;
                    }
                }
            }
            
            // cout << "DEBUG: Insert completed successfully" << endl;
        } catch (const exception& e) {
            cout << "ERROR: Exception during insert: " << e.what() << endl;
        }
    }

    // Search for all row IDs matching a key with a given binary operator
    vector<int> search(int key, BinaryOperator op) {
        vector<int> result;
        
        try {
            // Check if the tree is empty
            if (!root || (root->isLeaf && root->keys.empty())) {
                return result;
            }
            
            // For equality, we can directly find the leaf node
            if (op == EQUAL) {
                BPlusTreeNode* leaf = findLeafNode(key);
                
                // Check if leaf is valid
                if (!leaf) {
                    return result;
                }
                
                // Search for the key in the leaf
                for (size_t i = 0; i < leaf->keys.size(); i++) {
                    if (leaf->keys[i] == key) {
                        // Make sure rowIds index is valid
                        if (i < leaf->rowIds.size()) {
                            // Add all matching row IDs
                            result.insert(result.end(), leaf->rowIds[i].begin(), leaf->rowIds[i].end());
                        }
                    }
                }
                
                // Check if there are more matches in the next leaf nodes
                BPlusTreeNode* nextLeaf = leaf->next;
                while (nextLeaf != nullptr) {
                    bool foundMatch = false;
                    for (size_t i = 0; i < nextLeaf->keys.size(); i++) {
                        if (nextLeaf->keys[i] == key) {
                            // Make sure rowIds index is valid
                            if (i < nextLeaf->rowIds.size()) {
                                result.insert(result.end(), nextLeaf->rowIds[i].begin(), nextLeaf->rowIds[i].end());
                                foundMatch = true;
                            }
                        } else if (nextLeaf->keys[i] > key) {
                            // We've passed all matching keys
                            break;
                        }
                    }
                    
                    if (!foundMatch) {
                        // If we didn't find any matches in this leaf, we won't find any in subsequent leaves
                        break;
                    }
                    
                    nextLeaf = nextLeaf->next;
                }
            } else {
                // For other operators, we need to scan the leaves
                // Start with the leftmost leaf
                BPlusTreeNode* current = root;
                
                // Check if the tree is empty or just has a root
                if (!current) {
                    return result;
                }
                
                // Navigate to the leftmost leaf
                while (current && !current->isLeaf) {
                    if (current->children.empty()) {
                        // This should not happen in a well-formed B+ tree
                        return result;
                    }
                    current = current->children[0];
                }
                
                // Check if we found a valid leaf
                if (!current) {
                    return result;
                }
                
                // Scan all leaves
                while (current != nullptr) {
                    for (size_t i = 0; i < current->keys.size(); i++) {
                        int currentKey = current->keys[i];
                        
                        // Check if the key satisfies the operator
                        bool matches = false;
                        switch (op) {
                            case LESS_THAN:
                                matches = currentKey < key;
                                break;
                            case GREATER_THAN:
                                matches = currentKey > key;
                                break;
                            case LEQ:
                                matches = currentKey <= key;
                                break;
                            case GEQ:
                                matches = currentKey >= key;
                                break;
                            case NOT_EQUAL:
                                matches = currentKey != key;
                                break;
                            default:
                                break;
                        }
                        
                        if (matches) {
                            // Make sure rowIds index is valid
                            if (i < current->rowIds.size()) {
                                // Add all matching row IDs
                                result.insert(result.end(), current->rowIds[i].begin(), current->rowIds[i].end());
                            }
                        }
                        
                        // Optimization: If we've passed the value for certain operators, we can stop
                        if ((op == LESS_THAN || op == LEQ) && currentKey >= key) {
                            // For LESS_THAN, once we reach a key >= the target, we can skip to the next leaf
                            break;
                        }
                    }
                    
                    // Move to the next leaf
                    current = current->next;
                    
                    // Optimization: If we're looking for values less than key, and we've reached a leaf
                    // where all values are >= key, we can stop
                    if ((op == LESS_THAN || op == LEQ) && current != nullptr && !current->keys.empty() && current->keys[0] >= key) {
                        break;
                    }
                }
            }
        } catch (const exception& e) {
            cout << "ERROR: Exception during B+ tree search: " << e.what() << endl;
            // Return whatever results we've collected so far
        }
        
        return result;
    }

    // Save the tree to disk
    bool saveToDisk() {
        // Check if the tree is empty
        if (!root) {
            // Create an empty root
            root = new BPlusTreeNode(true);
        }
        
        ofstream outFile(indexFileName, ios::binary);
        if (!outFile) {
            return false;
        }
        
        try {
            // Write the order
            outFile.write(reinterpret_cast<char*>(&order), sizeof(int));
            
            // Write the tree
            saveNodeToDisk(root, outFile);
            
            outFile.close();
            return true;
        } catch (const exception& e) {
            cout << "Error saving B+ tree to disk: " << e.what() << endl;
            outFile.close();
            return false;
        }
    }

    // Load the tree from disk
    bool loadFromDisk() {
        ifstream inFile(indexFileName, ios::binary);
        if (!inFile) {
            return false;
        }
        
        try {
            // Clean up the existing tree
            if (root) {
                delete root;
                root = nullptr;
            }
            
            // Read the order
            inFile.read(reinterpret_cast<char*>(&order), sizeof(int));
            
            // Read the tree
            root = loadNodeFromDisk(inFile);
            
            inFile.close();
            
            // Make sure we have a valid root
            if (!root) {
                root = new BPlusTreeNode(true);
                return false;
            }
            
            // Connect the leaf nodes
            connectLeafNodes();
            
            return true;
        } catch (const exception& e) {
            cout << "Error loading B+ tree from disk: " << e.what() << endl;
            inFile.close();
            
            // Create a new empty root
            if (root) {
                delete root;
            }
            root = new BPlusTreeNode(true);
            
            return false;
        }
    }

    // Check if the index file exists
    bool exists() {
        ifstream inFile(indexFileName);
        return inFile.good();
    }
};

#endif // BPLUSTREE_H