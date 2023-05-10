#include <iostream>
#include <unistd.h>
#include <random> // for random number generator
#include <iomanip>
#include "SHA/sha256.h"
#define HEX( x, len ) std::setw(2 * len) << std::setfill('0') << std::hex << std::uppercase << (((1ll << (8 * len)) - 1) & (unsigned int)( x )) << std::dec

using namespace std;

struct ListNode
{
    int value;
    char prev_hash[32];
    ListNode* prev;
};

struct List
{
    ListNode* tail;
    char tail_hash[32];
};

// Inserts an integer to the end of linked list
void append(List& list, int value)
{
    auto new_node = new ListNode;
    memset(new_node, 0, sizeof(ListNode));

    // Initialise the entries of new tail
    new_node->value = value;
    new_node->prev = list.tail;
    memcpy(new_node->prev_hash, list.tail_hash, 32);

    // Calculate hash of new node
    SHA256 sha;
    sha.update((uint8_t*)new_node, sizeof(ListNode));
    auto digest = sha.digest();

    // Update the list
    list.tail = new_node;
    memcpy(list.tail_hash, digest, 32);

    // Delete the hash from heap, as it is copied in list
    delete[] digest;
}

// Verifies that the list is intact
void verify_list(List& list)
{
    if (list.tail == nullptr)
    {
        cout << "List is empty" << endl;
        return;
    }

    auto current = list.tail;
    char expected_hash[32];
    memcpy(expected_hash, list.tail_hash, 32);

    while (current)
    {
        // Calculate hash of current node
        SHA256 sha;
        sha.update((uint8_t*)current, sizeof(ListNode));
        auto digest = sha.digest();

        // Print the information about node
        cout << "Node value is: " << current->value << endl;
        cout << "Expected node hash: ";
        for (auto &x: expected_hash)
            cout << HEX(x, 1);
        cout << endl;

        cout << "Computed hash:      ";
        for (int i = 0; i < 32; ++i)
            cout << HEX(digest[i], 1);
        cout << endl;

        // Compare the hash of current node with the hash stored in list
        if (memcmp(digest, expected_hash, 32) != 0)
        {
            cout << "List content is changed!!!" << endl;
            delete[] digest;
            return;
        }

        // Delete the hash from heap, as it is copied in list
        delete[] digest;

        // Move to previous node
        memcpy(expected_hash, current->prev_hash, 32);
        current = current->prev;
    }

    cout << "We reached the beginning of list!" << endl;
}

int main()
{
    // Create an empty list
    List list;
    list.tail = nullptr;
    memset(list.tail_hash, 0, 32);

    // Insert random 5 elements to list
    std::mt19937 mt{std::random_device{}() }; // Instantiate a 32-bit Mersenne Twister

    for (int i = 0; i < 5; ++i)
        append(list, (int)mt());

    // Verify the list
    cout << "Verify initial list: " << endl;
    verify_list(list);

    // Change the content of some random index
    auto current = list.tail->prev->prev;
    auto old_val = current->value;
    current->value = 0;

    // Verify if our program detects the change
    cout << endl << endl << "After change the value of third node to 0: " << endl;
    verify_list(list);

    // Resets the value
    current->value = old_val;

    // Delete the head of list
    auto next_head = list.tail;
    while (next_head->prev->prev != nullptr)
        next_head = next_head->prev;

    next_head->prev = nullptr;

    // Check if our list detects the change!
    cout << endl << endl << "After removing the head from the list: " << endl;
    verify_list(list);
}