/*
 * Library Management System
 * Data Structures Lab - Group Project (individual)
 * Author : Ariz Faiz Siddiqui (25K-0992), Section J
 *
 * A console program that manages books, members, issuing, returning
 * and waiting lists for a small library.
 *
 * All main data structures are written by hand (no STL containers):
 *   Module 1 - Singly linked list : BookList, BorrowedList (+ merge sort by ID)
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

// Converts a whole number to text.
string intToStr(int n)
{
    if (n == 0)
        return "0";
    bool negative = n < 0;
    if (negative)
        n = -n;
    string result = "";
    while (n > 0)
    {
        result = string(1, (char)('0' + n % 10)) + result;
        n /= 10;
    }
    if (negative)
        result = "-" + result;
    return result;
}

// Adds spaces after s until it is width characters long (used to line up columns).
string pad(const string &s, int width)
{
    string result = s;
    while ((int)result.size() < width)
        result += " ";
    return result;
}

// ===============================================================
// MODULE 3 (part 1): QUEUE - waiting list for a book
// ===============================================================

// One node of the queue. It stores the ID of a waiting member.
struct QueueNode
{
    int memberId;
    QueueNode *next;
};

// A FIFO queue built on a linked list. The member who joins first
// is the first one to get the book when it is returned.
class WaitQueue
{
private:
    QueueNode *front;   // next member to be served
    QueueNode *rear;    // last member in the line
    int count;          // number of members waiting

    // Copying is disabled so two queues never share the same nodes.
    WaitQueue(const WaitQueue &);
    WaitQueue &operator=(const WaitQueue &);

public:
    WaitQueue() : front(0), rear(0), count(0) {}
    ~WaitQueue() { clear(); }

    // Adds a member at the back of the line. O(1)
    void enqueue(int memberId)
    {
        QueueNode *node = new QueueNode;
        node->memberId = memberId;
        node->next = 0;
        if (rear == 0)           // queue was empty
            front = rear = node;
        else
        {
            rear->next = node;
            rear = node;
        }
        count++;
    }

    // Removes the member at the front and gives the ID in memberId.
    // Returns false if the queue is empty. O(1)
    bool dequeue(int &memberId)
    {
        if (front == 0)
            return false;
        QueueNode *old = front;
        memberId = old->memberId;
        front = front->next;
        if (front == 0)          // queue became empty
            rear = 0;
        delete old;
        count--;
        return true;
    }

    bool isEmpty() const { return count == 0; }
    int size() const { return count; }

    // Checks if a member is already waiting. O(n)
    bool contains(int memberId) const
    {
        for (QueueNode *cur = front; cur != 0; cur = cur->next)
            if (cur->memberId == memberId)
                return true;
        return false;
    }

    // Gives access to the first node so the list can be printed.
    QueueNode *getFront() const { return front; }

    // Deletes every node.
    void clear()
    {
        int dummy;
        while (dequeue(dummy)) {}
    }
};

// ---------------------------------------------------------------
// Book record. Each book keeps its own waiting list.
// ---------------------------------------------------------------
struct Book
{
    int id;
    string title;
    string author;
    int issuedTo;        // member ID who has the book, 0 means available
    WaitQueue waitlist;  // members waiting for this book

    Book(int bookId, const string &t, const string &a)
        : id(bookId), title(t), author(a), issuedTo(0) {}

    bool isAvailable() const { return issuedTo == 0; }
};

// ===============================================================
// MODULE 1: LINKED LISTS
// ===============================================================

// ---------- BorrowedList: the book IDs one member has borrowed ----------
struct IntNode
{
    int value;
    IntNode *next;
};

