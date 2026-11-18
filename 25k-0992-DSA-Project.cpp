/*
 * Library Management System
 * Data Structures Lab - Group Project (individual)
 * Author : Ariz Faiz Siddiqui (25K-0992), Section J
 *
 * A console program that manages books, members, issuing, returning
 * and waiting lists for a small library.
 *
 * All main data structures are written by hand (no STL containers):
 *   Module 1 - Singly linked list : BookList, BorrowedList
 *   Module 2 - Hash table         : HashTable (separate chaining)
 *   Module 3 - Queue (FIFO)       : WaitQueue, plus the menu interface
 *
 * Only <iostream> and <string> are used. Written to compile in Dev-C++
 * (old C++98 style, no C++11 features).
 */

#include <iostream>
#include <string>
using namespace std;

// ---------------------------------------------------------------
// Constants
// ---------------------------------------------------------------
const int MAX_BORROW = 3;     // most books one member can hold at a time
const int TABLE_SIZE = 101;   // number of buckets in each hash table
const int MAX_RESULTS = 50;   // most results shown by a title search

// ---------------------------------------------------------------
// Small helper functions
// ---------------------------------------------------------------

// Returns a lower case copy of s (used for case-insensitive title search).
string toLower(string s)
{
    for (int i = 0; i < (int)s.size(); i++)
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}

// True for space, tab and line break characters.
bool isSpaceChar(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// Removes spaces from the start and end of s.
string trim(const string &s)
{
    int start = 0;
    int end = (int)s.size();
    while (start < end && isSpaceChar(s[start])) start++;
    while (end > start && isSpaceChar(s[end - 1])) end--;
    return s.substr(start, end - start);
}
