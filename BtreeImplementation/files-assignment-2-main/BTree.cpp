#include <sstream>
#include <utility>
#include <iostream>
#include <algorithm>
#include <stack>
#include "BTree.h"
#include "exceptions/InvalidRecordNumber.h"
#include "exceptions/InvalidPairNumber.h"
#include "utils.h"


BTree::BTree(std::string _path, int _numberOfRecords, int _m, int _cellSize) :
        path{std::move(_path)},
        m{_m},
        numberOfRecords{_numberOfRecords},
        cellSize{_cellSize} {
    openFile();
    initialize();
}

void BTree::openFile() {
    file.open("../btree", std::ios::trunc | std::ios::in | std::ios::out);
}

void BTree::display() {
    // For each record
    for (int i = 0; i < numberOfRecords; ++i) {
        // Read the record
        char record[recordSize()];
        file.seekg(i * recordSize(), std::ios::beg);
        file.read(record, recordSize());

        // Instead of printing the array of characters, 
        // print each character one by one to avoid weird null-terminator issue
        for (char r: record) std::cout << r;

        std::cout << '\n';
    }
}

BTree::~BTree() {
    file.close();
}

std::pair<int, int> BTree::pair(int recordNumber, int pairNumber) {
    // Validate inputs
    validateRecordNumber(recordNumber);
    validatePairNumber(pairNumber);

    // Create and return the cell
    std::pair<int, int> thePair;
    thePair.first = cell(recordNumber, 2 * pairNumber - 1);
    thePair.second = cell(recordNumber, 2 * pairNumber);
    return thePair;
}

std::vector<std::pair<int, int>> BTree::node(int recordNumber) {
    // Validate input
    validateRecordNumber(recordNumber);

    // Go to the specified record on the b-tree file
    file.seekg(recordNumber * recordSize() + cellSize, std::ios::beg);

    // Create and return the node
    std::vector<std::pair<int, int>> theNode{};

    // Read every pair in the node
    for (int i = 1; i <= m; ++i) {
        std::pair<int, int> p = pair(recordNumber, i);

        // If it is empty then the rest is empty, so return
        if (p.second == -1) return theNode;

        // Otherwise, continue reading the node
        theNode.push_back(p);
    }
    return theNode;
}

void BTree::validateRecordNumber(int recordNumber) const {
    // If the record number is not between 1 and numberOfRecords
    if (recordNumber <= 0 || recordNumber > numberOfRecords)
        // then it is not a valid record number
        throw InvalidRecordNumber(recordNumber);
}

void BTree::validatePairNumber(int pairNumber) const {
    // If the pair number is not between 1 and m
    if (pairNumber <= 0 || pairNumber > m)
        // then it is not a valid pair number
        throw InvalidPairNumber(pairNumber);
}

int BTree::nextEmpty() {
    return cell(0, 1);
}

int BTree::cell(int rowIndex, int columnIndex) {
    // Go to the specified cell
    file.seekg(rowIndex * recordSize() + columnIndex * cellSize, std::ios::beg);

    // Read and return the integer value in the cell
    char cell[cellSize];
    file.read(cell, cellSize);
    return ctoi(cell);
}

int BTree::insert(int recordId, int reference) {
    std::vector<std::pair<int, int>> current;

    // If the root is empty
    if (isEmpty(1)) {
        // Insert in root

        // The next empty for the record at next empty
        int nextEmptyNext = cell(nextEmpty(), 1);

        // Update the next empty
        writeCell(nextEmptyNext, 0, 1);

        // Create the node
        current = node(1);

        // Insert the new pair
        current.emplace_back(recordId, reference);

        // Sort the node
        std::sort(current.begin(), current.end());

        // Write the node in root
        writeNode(current, 1);

        // Mark the root as leaf
        markLeaf(1, 0);

        // Return the index of the record in which the insertion happened
        // i.e. the root in this case
        return 1;
    }

    // Keep track of visited records to updateAfterInsert them after insertion
    std::stack<int> visited;

    // Search for recordId in every node in the b-tree
    // starting with the root
    int i = 1;
    bool found;
    while (!isLeaf(i)) {
        visited.push(i);
        current = node(i);
        found = false;
        for (auto p: current) {
            // If a greater value is found
            if (p.first >= recordId) {
                // B-Tree traversal
                i = p.second;
                found = true;
                break;
            }
        }

        // B-Tree traversal
        if (!found) i = current.back().second;
    }

    current = node(i);

    // Insert the new pair
    current.emplace_back(recordId, reference);

    // Sort the node
    std::sort(current.begin(), current.end());

    int newFromSplitIndex = -1;

    // If record overflowed after insertion
    if (current.size() > m)
        newFromSplitIndex = split(i, current);
    else
        // Write the node in root
        writeNode(current, i);

    // If the insertion happened in root
    // Then there are no parents to updateAfterInsert
    if (i == 1) return i;

    // Otherwise, updateAfterInsert parents
    while (!visited.empty()) {
        int lastVisitedIndex = visited.top();
        visited.pop();

        newFromSplitIndex = updateAfterInsert(lastVisitedIndex, newFromSplitIndex);
    }

    // Return the index of the inserted record
    // or -1 if insertion failed
    return i;
}

void BTree::initialize() {
    // For each record
    for (int recordIndex = 0; recordIndex < numberOfRecords + 1; ++recordIndex) {
        // Write -1 in the first cell to indicate it is 
        // an empty record (available for allocation)
        writeCell(-1, recordIndex, 0);

        // Write the number of the next empty record
        // in the available list
        if (recordIndex == numberOfRecords - 1)
            writeCell(-1, recordIndex, 1);
        else
            writeCell(recordIndex + 1, recordIndex, 1);

        // Fill the rest of the record with -1s
        for (int cellIndex = 2; cellIndex <= m * 2; ++cellIndex)
            writeCell(-1, recordIndex, cellIndex);
    }
}

std::string BTree::pad(int value) const {
    std::stringstream result;

    // Convert the integer value into a string
    std::string stringValue = std::to_string(value);

    // Write the string
    result << stringValue;

    // Write spaces until the final result's size becomes the cell size
    for (int i = 0; i < cellSize - stringValue.size(); ++i) result << ' ';
    return result.str();
}

void BTree::writeCell(int value, int rowIndex, int columnIndex) {
    // Go to the specified cell
    file.seekg(rowIndex * recordSize() + columnIndex * cellSize, std::ios::beg);

    // Write the given value in the cell
    file << pad(value);
}

bool BTree::isLeaf(int recordNumber) {
    return cell(recordNumber, 0) == 0;
}

bool BTree::isEmpty(int recordNumber) {
    return cell(recordNumber, 0) == -1;
}

void BTree::writeNode(const std::vector<std::pair<int, int>> &node, int recordNumber) {
    clearRecord(recordNumber);
    file.seekg(recordNumber * recordSize() + cellSize, std::ios::beg);
    for (auto p: node)
        file << pad(p.first) << pad(p.second);
}

void BTree::markLeaf(int recordNumber, int leafStatus) {
    file.seekg(recordNumber * recordSize(), std::ios::beg);
    file << pad(leafStatus);
}

int BTree::updateAfterInsert(int parentRecordNumber, int newChildRecordNumber) {
    std::vector<std::pair<int, int>> newParent;
    auto parent = node(parentRecordNumber);
    // For each value in parent
    for (auto p: parent) {
        auto childNode = node(p.second);
        // Add the maximum of the value's child
        newParent.emplace_back(childNode.back().first, p.second);
    }
    // If there was a new child from previous split
    if (newChildRecordNumber != -1)
        //  Add the maximum of the new value's child
        newParent.emplace_back(node(newChildRecordNumber).back().first, newChildRecordNumber);

    std::sort(newParent.begin(), newParent.end());

    int newFromSplitIndex = -1;

    // If record overflowed after insertion
    if (newParent.size() > m)
        newFromSplitIndex = split(parentRecordNumber, newParent);
    else
        // Write new parent
        writeNode(newParent, parentRecordNumber);

    return newFromSplitIndex;
}

int BTree::split(int recordNumber, std::vector<std::pair<int, int>> originalNode) {
    if (recordNumber == 1)
        return split(originalNode);

    // Get the index of the new record created after split
    int newRecordNumber = nextEmpty();

    // If there are no empty records, then splitting fails
    if (newRecordNumber == -1) return -1;

    // Update the next empty cell with the next in available list
    writeCell(cell(newRecordNumber, 1), 0, 1);

    // Distribute originalNode on two new nodes
    std::vector<std::pair<int, int>> firstNode, secondNode;

    // Fill first and second nodes from originalNode
    auto middle(originalNode.begin() + (int) (originalNode.size()) / 2);
    for (auto it = originalNode.begin(); it != originalNode.end(); ++it) {
        if (std::distance(it, middle) > 0) firstNode.push_back(*it);
        else secondNode.push_back(*it);
    }

    // Clear originalNodeIndex and newNodeIndex
    clearRecord(recordNumber);
    clearRecord(newRecordNumber);

    markLeaf(recordNumber, 0);
    writeNode(firstNode, recordNumber);

    markLeaf(newRecordNumber, 0);
    writeNode(secondNode, newRecordNumber);

    return newRecordNumber;
}

void BTree::clearRecord(int recordNumber) {
    for (int i = 1; i <= m * 2; ++i) {
        file.seekg(recordNumber * recordSize() + i * cellSize, std::ios::beg);
        file << pad(-1);
    }
}

bool BTree::split(std::vector<std::pair<int, int>> root) {
    // Find 2 empty records for the new nodes
    int firstNodeIndex = nextEmpty();
    if (firstNodeIndex == -1) return false;

    // Get next empty node in available list
    int secondNodeIndex = cell(firstNodeIndex, 1);
    if (secondNodeIndex == -1) return false;

    // Update the next empty cell with the next in available list
    writeCell(cell(secondNodeIndex, 1), 0, 1);

    std::vector<std::pair<int, int>> firstNode, secondNode;

    // Fill first and second nodes from root
    auto middle(root.begin() + (int) (root.size()) / 2);
    for (auto it = root.begin(); it != root.end(); ++it) {
        if (std::distance(it, middle) > 0) firstNode.push_back(*it);
        else secondNode.push_back(*it);
    }

    markLeaf(firstNodeIndex, leafStatus(1));
    writeNode(firstNode, firstNodeIndex);

    markLeaf(secondNodeIndex, leafStatus(1));
    writeNode(secondNode, secondNodeIndex);

    clearRecord(1);

    // Create new root with max values from the 2 new nodes
    std::vector<std::pair<int, int>> newRoot;
    newRoot.emplace_back(firstNode.back().first, firstNodeIndex);
    newRoot.emplace_back(secondNode.back().first, secondNodeIndex);
    markNonLeaf(1);
    writeNode(newRoot, 1);

    return true;
}

void BTree::markNonLeaf(int recordNumber) {
    file.seekg(recordNumber * recordSize(), std::ios::beg);
    file << pad(1);
}

int BTree::leafStatus(int recordNumber) {
    return cell(recordNumber, 0);
}

int BTree::search(int recordId) {
    if (isEmpty(1)) return -1;

    std::vector<std::pair<int, int>> current;

    // Search for recordId in every node in the b-tree
    // starting with the root
    int i = 1;
    bool found;
    while (!isLeaf(i)) {
        current = node(i);
        found = false;
        for (auto p: current) {
            // If a greater value is found
            if (p.first >= recordId) {
                // B-Tree traversal
                i = p.second;
                found = true;
                break;
            }
        }

        // B-Tree traversal
        if (!found) i = current.back().second;
    }

    current = node(i);
    for (auto pair: current)
        if (pair.first == recordId)
            return pair.second;

    return -1;
}

void BTree::remove(int recordId) {
    // If the root is empty
    if (isEmpty(1)) return;

    std::vector<std::pair<int, int>> current;

    // Keep track of visited records to updateAfterInsert them after insertion
    std::stack<int> visited;

    // Search for recordId in every node in the b-tree
    // starting with the root
    int currentRecordNumber = 1, parentRecordNumber = -1;
    bool found;
    while (!isLeaf(currentRecordNumber)) {
        visited.push(currentRecordNumber);
        current = node(currentRecordNumber);
        found = false;
        for (auto p: current) {
            // If a greater value is found
            if (p.first >= recordId) {
                // B-Tree traversal
                parentRecordNumber = currentRecordNumber;
                currentRecordNumber = p.second;
                found = true;
                break;
            }
        }

        // B-Tree traversal
        if (!found) {
            parentRecordNumber = currentRecordNumber;
            currentRecordNumber = current.back().second;
        }
    }

    current = node(currentRecordNumber);

    // Delete first pair with first == recordId
    for (auto pair = current.begin(); pair != current.end(); ++pair)
        if (pair->first == recordId) {
            current.erase(pair);
            break;
        }


    if (current.size() < m / 2) {
        if (!redistribute(parentRecordNumber, currentRecordNumber, current)) {
            merge(parentRecordNumber, currentRecordNumber, current);
        }
    } else {
        writeNode(current, currentRecordNumber);
    }

    // Otherwise, updateAfterInsert parents
    while (!visited.empty()) {
        int lastVisitedIndex = visited.top();
        visited.pop();
        if (!visited.empty())
            updateAfterDelete(lastVisitedIndex, visited.top());
        else
            updateAfterDelete(lastVisitedIndex, -1);
    }
}

void BTree::markEmpty(int recordNumber) {
    file.seekg(recordNumber * recordSize(), std::ios::beg);
    file << pad(-1);
}

bool
BTree::redistribute(int parentRecordNumber, int currentRecordNumber, std::vector<std::pair<int, int>> currentNode) {
    auto parent = node(parentRecordNumber);

    if (parent[0].second == currentRecordNumber) {
        return false;
    }

    // For each pair in parent node
    for (int i = 0; i < parent.size() - 1; ++i) {
        // If the pair after the current pair is pointing to the record where deletion happened
        // i.e. If we reached the pair to left of the pair where the deletion happened
        if (parent[i + 1].second == currentRecordNumber) {
            int siblingRecordNumber = parent[i].second;
            auto sibling = node(siblingRecordNumber);
            // Check the size of the child node of this pair
            // If it is going to be less than m/2 after redistribution, do nothing and return false
            if (sibling.size() == m / 2) {
                return false;
            } else { // Otherwise, if we can redistribute
                // Take one pair from sibling and put it in the node where deletion happened
                currentNode.push_back(sibling.back());

                // Remove the moved pair from the sibling node
                sibling.pop_back();

                // Sort the current node and write both nodes
                std::sort(currentNode.begin(), currentNode.end());
                clearRecord(currentRecordNumber);
                writeNode(currentNode, currentRecordNumber);
                clearRecord(siblingRecordNumber);
                writeNode(sibling, siblingRecordNumber);
                return true;
            }
        }
    }
    return false;
}

void BTree::updateAfterDelete(int parentRecordNumber, int grandParentRecordNumber) {
    std::vector<std::pair<int, int>> newParent;

    auto parent = node(parentRecordNumber);

    // For each pair in parent
    for (auto p: parent) {
        auto childNode = node(p.second);
        if (!childNode.empty())
            // Add the maximum of the value's child
            newParent.emplace_back(childNode.back().first, p.second);
    }

    std::sort(newParent.begin(), newParent.end());

    // If record overflowed after insertion
    if (newParent.size() < m / 2 && grandParentRecordNumber != -1) {
//        newFromSplitIndex = split(parentRecordNumber, newParent);
        if (!redistribute(grandParentRecordNumber, parentRecordNumber, newParent)) {
            merge(grandParentRecordNumber, parentRecordNumber, newParent);
        }
    } else
        // Write new parent
        writeNode(newParent, parentRecordNumber);
}

void BTree::merge(int parentRecordNumber, int currentRecordNumber, std::vector<std::pair<int, int>> currentNode) {
    auto parent = node(parentRecordNumber);

    if (parent[0].second == currentRecordNumber) {
        if (parent.size() > 1) {
            int siblingRecordNumber = parent[1].second;
            auto sibling = node(siblingRecordNumber);
            while (!currentNode.empty()) {
                sibling.push_back(currentNode.back());
                currentNode.pop_back();
            }
            std::sort(sibling.begin(), sibling.end());
            writeNode(sibling, siblingRecordNumber);
            clearRecord(currentRecordNumber);
            markEmpty(currentRecordNumber);
            int empty = nextEmpty();
            writeCell(currentRecordNumber, 0, 1);
            writeCell(empty, currentRecordNumber, 1);
        } else {
/*
            int extraRecordNumber = parent[0].second;
            parent[0].second = currentNode[0].second;
            writeNode(parent, parentRecordNumber);
            clearRecord(extraRecordNumber);
            markEmpty(extraRecordNumber);
            int empty = nextEmpty();
            writeCell(extraRecordNumber, 0, 1);
            writeCell(empty, extraRecordNumber, 1);

          1 [<24,9>,-1,-1,-1,-1]
             /
            9 [<24,7>,-1,-1,-1,-1]
                /
               7 [12,17,24,-1,-1]
*/
        }
        return;
    }

    // For each pair in parent node
    for (int i = 0; i < parent.size() - 1; ++i) {
        // If the pair after the current pair is pointing to the record where deletion happened
        // i.e. If we reached the pair to left of the pair where the deletion happened
        if (parent[i + 1].second == currentRecordNumber) {
            int siblingRecordNumber = parent[i].second;
            auto sibling = node(siblingRecordNumber);
            while (!currentNode.empty()) {
                sibling.push_back(currentNode.back());
                currentNode.pop_back();
            }
            std::sort(sibling.begin(), sibling.end());
            writeNode(sibling, siblingRecordNumber);
            clearRecord(currentRecordNumber);
            markEmpty(currentRecordNumber);
            int empty = nextEmpty();
            writeCell(currentRecordNumber, 0, 1);
            writeCell(empty, currentRecordNumber, 1);
            return;
        }
    }
}
