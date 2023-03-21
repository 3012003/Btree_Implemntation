#include <iostream>
#include "BTree.h"

void separate();

void log(const std::string &message, int value);

void insertions(BTree &btree);

void deletions(BTree &btree);

int main() {
    BTree btree("../btree", 10, 5, 5);

    insertions(btree);

    separate();

    deletions(btree);

    return 0;
}

void deletions(BTree &btree) {
    int target;

    target = 10;
    log("Removed ", target);
    btree.remove(target);
    btree.display();
    separate();

    target = 9;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 8;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 1;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 2;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 3;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 5;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 6;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 7;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 15;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 19;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 32;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 14;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 18;
    log("Removed ", target);
    btree.remove(target);
    btree.display();

    target = 30;
    log("Removed ", target);
    btree.remove(target);
    btree.display();
}

void insertions(BTree &btree) {

    separate();

    btree.insert(3, 12);
    btree.insert(7, 24);
    btree.insert(10, 48);
    btree.insert(24, 60);
    btree.insert(14, 72);

    btree.display();

    separate();

    btree.insert(19, 84);

    btree.display();

    separate();

    btree.insert(30, 96);
    btree.insert(15, 108);
    btree.insert(1, 120);
    btree.insert(5, 132);

    btree.display();

    separate();

    btree.insert(2, 144);

    btree.display();

    separate();

    btree.insert(8, 156);
    btree.insert(9, 168);
    btree.insert(6, 180);
    btree.insert(11, 192);
    btree.insert(12, 204);
    btree.insert(17, 216);
    btree.insert(18, 228);

    btree.display();

    separate();

    btree.insert(32, 240);

    btree.display();
}

void log(const std::string &message, int value) {
    std::cout << message << value << '\n';
}

void separate() {
    std::cout << "-----------------------------------------------------\n";
}
