#pragma once

#include <fstream>
#include <vector>


class BTree {
private:
    // The b-tree's file path.
    std::string path;

    // The file that holds the b-tree nodes.
    std::fstream file;

    // The number of values one node can hold.
    int m;

    // The maximum number of records the b-tree file can hold.
    int numberOfRecords;

    // The number of characters allocated for each pair
    // in the b-tree file.
    int cellSize;
public:

    // Returns the number of characters a record in the b-tree file takes
    // based on the specified pair size and number of values that one record can hold.
    int recordSize() const { return cellSize + 2 * m * cellSize; }

    // Returns the number of characters a pair values takes in a record
    // based on the specified pair size. 
    int pairSize() const { return 2 * cellSize; }
public:

    // Initializes the b-tree file with the maximum number of
    // records it can hold, and the maximum number of values
    // one record can hold, and the size of each pair in the b-tree file.
    BTree(std::string _path, int _numberOfRecords, int _m, int _cellSize);

    // Closes the b-tree file.
    ~BTree();

    //  Inserts a new value in the b-tree.
    //  Returns the index of the record in the b-tree file.
    //  Returns -1 if insertion failed.
    //  Insertion fails if there are no enough empty
    //  records to complete the insertion.
    int insert(int recordId, int reference);

    // Searches for the node with the given value.
    // Returns the index of the record in the b-tree.
    // Returns -1 if the given value is not found in any node.
    int search(int recordId);

    // Prints the b-tree file in a table format.
    void display();

    // Searches for and removes the given value from the b-tree.
    void remove(int recordId);

    // Reads and returns the cell at the specified record and pair numbers.
    std::pair<int, int> pair(int recordNumber, int pairNumber);

    // Reads and returns the node at the specified record number.
    std::vector<std::pair<int, int>> node(int recordNumber);

    // Returns the value of the second pair in the header.
    // This value is the number of the first empty record
    // available for allocation.
    int nextEmpty();

    // Returns true if the record's leaf status is equal to 0
    bool isLeaf(int recordNumber);

    // Returns true if the record's leaf status is equal to -1
    bool isEmpty(int recordNumber);
private:
    // Splits the record into two.
    // Returns the number of the newly allocated record in the b-tree file.
    int split(int recordNumber, std::vector<std::pair<int, int >> originalNode);

    // Splits the root into two and allocates a new root.
    bool split(std::vector<std::pair<int, int>> root);

    // Opens the b-tree's file.
    void openFile();

    // Returns the integer value that the specified cell holds.
    int cell(int rowIndex, int columnIndex);
    
    // Writes the given value in the specified cell
    // in the b-tree file.
    void writeCell(int value, int rowIndex, int columnIndex);

    // Asserts the given record number is within a valid range.
    // This valid range depends on the maximum number of records
    // that the b-tree file can hold.
    void validateRecordNumber(int recordNumber) const;

    // Asserts the given pair number is within a valid range.
    // This valid range depends on the maximum number of values
    // a record in the b-tree file can hold.
    void validatePairNumber(int pairNumber) const;

    // Initializes the b-tree file with -1s
    // and the available list
    void initialize();

    // Returns the given value in a padded string
    // with the size of the cell in the b-tree file.
    std::string pad(int value) const;

    void writeNode(const std::vector<std::pair<int, int>>& node, int recordNumber);

    void markLeaf(int recordNumber, int leafStatus);

    int updateAfterInsert(int parentRecordNumber, int newChildRecordNumber);

    void clearRecord(int recordNumber);

    void markNonLeaf(int recordNumber);

    int leafStatus(int recordNumber);

    void markEmpty(int recordNumber);

    bool
    redistribute(int parentRecordNumber, int currentRecordNumber, std::vector<std::pair<int, int>> currentNode);

    void
    merge(int parentRecordNumber, int currentRecordNumber, std::vector<std::pair<int, int>> currentNode);

    void updateAfterDelete(int parentRecordNumber, int grandParentRecordNumber);
};
