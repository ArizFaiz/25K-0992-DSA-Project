/*
 * Data Structures LAB Project
 * Ariz Faiz Siddiqui (25K-0992), Section J
 *
 * A console program that manages books, members, issuing, returning and waiting lists for a small library.
 * All main data structures are written by hand (no STL containers):
 *   Module 1 - Singly linked list : BookList, BorrowedList (+ merge sort by ID)
 *   Module 2 - Hash table         : HashTable (separate chaining)
 *   Module 3 - Queue (FIFO)       : WaitQueue, plus the menu interface

*/

#include <iostream>
#include <string>
using namespace std;

// Constants
const int MAX_BORROW = 3;     // most books one member can hold at a time
const int TABLE_SIZE = 101;   // number of buckets in each hash table
const int MAX_RESULTS = 50;   // most results shown by a title search


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




// MODULE 3 (part 1): QUEUE - waiting list for a book

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

// Book record. Each book keeps its own waiting list.
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

// MODULE 1: LINKED LISTS

// BorrowedList: the book IDs one member has borrowed
struct IntNode
{
    int value;
    IntNode *next;
};

class BorrowedList
{
private:
    IntNode *head;
    int count;

    BorrowedList(const BorrowedList &);
    BorrowedList &operator=(const BorrowedList &);

public:
    BorrowedList() : head(0), count(0) {}
    ~BorrowedList()
    {
        while (head != 0)
        {
            IntNode *old = head;
            head = head->next;
            delete old;
        }
    }

    // Adds a book ID at the front of the list. O(1)
    void add(int bookId)
    {
        IntNode *node = new IntNode;
        node->value = bookId;
        node->next = head;
        head = node;
        count++;
    }

    // Removes a book ID. Returns false if it is not in the list. O(n)
    bool remove(int bookId)
    {
        IntNode *prev = 0;
        IntNode *cur = head;
        while (cur != 0 && cur->value != bookId)
        {
            prev = cur;
            cur = cur->next;
        }
        if (cur == 0)
            return false;
        if (prev == 0)           // removing the first node
            head = cur->next;
        else
            prev->next = cur->next;
        delete cur;
        count--;
        return true;
    }

    bool contains(int bookId) const
    {
        for (IntNode *cur = head; cur != 0; cur = cur->next)
            if (cur->value == bookId)
                return true;
        return false;
    }

    int size() const { return count; }
    IntNode *getHead() const { return head; }
};

// BookList: all books in the library
struct BookNode
{
    Book *book;
    BookNode *next;
};

// Singly linked list with head and tail pointers.
// The list owns the Book objects and deletes them in its destructor.
class BookList
{
private:
    BookNode *head;
    BookNode *tail;
    int count;

    BookList(const BookList &);
    BookList &operator=(const BookList &);

public:
    BookList() : head(0), tail(0), count(0) {}
    ~BookList()
    {
        while (head != 0)
        {
            BookNode *old = head;
            head = head->next;
            delete old->book;
            delete old;
        }
    }

    // Adds a book at the end of the list. O(1) because of the tail pointer.
    void add(Book *book)
    {
        BookNode *node = new BookNode;
        node->book = book;
        node->next = 0;
        if (tail == 0)           // list was empty
            head = tail = node;
        else
        {
            tail->next = node;
            tail = node;
        }
        count++;
    }

    // Removes (and deletes) the book with this ID. O(n)
    bool remove(int bookId)
    {
        BookNode *prev = 0;
        BookNode *cur = head;
        while (cur != 0 && cur->book->id != bookId)
        {
            prev = cur;
            cur = cur->next;
        }
        if (cur == 0)
            return false;
        if (prev == 0)
            head = cur->next;
        else
            prev->next = cur->next;
        if (cur == tail)            // removed the last node, move the tail back
            tail = prev;
        delete cur->book;
        delete cur;
        count--;
        return true;
    }

    int size() const { return count; }
    BookNode *getHead() const { return head; }
};

// Merge sort for a linked list of BookNode

// Merges two lists that are already sorted by book ID into one sorted list.
BookNode *mergeSortedLists(BookNode *a, BookNode *b)
{
    BookNode start;             // temporary first node to make the code simpler
    start.book = 0;
    start.next = 0;
    BookNode *tail = &start;
    while (a != 0 && b != 0)
    {
        if (a->book->id <= b->book->id)
        {
            tail->next = a;
            a = a->next;
        }
        else
        {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = (a != 0) ? a : b;      // add whatever is left
    return start.next;
}

// Sorts a list by book ID. The list is cut into two halves (slow and fast
// pointers find the middle), each half is sorted, then the halves are merged.
BookNode *mergeSortList(BookNode *head)
{
    if (head == 0 || head->next == 0)
        return head;
    BookNode *slow = head;
    BookNode *fast = head->next;
    while (fast != 0 && fast->next != 0)
    {
        slow = slow->next;
        fast = fast->next->next;
    }
    BookNode *second = slow->next;
    slow->next = 0;
    return mergeSortedLists(mergeSortList(head), mergeSortList(second));
}

// MODULE 2: HASH TABLE (separate chaining)

// One entry in a bucket chain. It only points to the Book;
// the Book itself is owned by BookList.
struct HashEntry
{
    Book *book;
    HashEntry *next;
};

// The same class is used twice: once keyed by book ID and once keyed by
// title. The flag byId decides which hash function and key are used.
class HashTable
{
private:
    HashEntry *buckets[TABLE_SIZE];
    bool byId;

    HashTable(const HashTable &);
    HashTable &operator=(const HashTable &);

    int hashId(int id) const { return id % TABLE_SIZE; }

    // Polynomial string hash: h = h * 31 + character
    int hashTitle(const string &title) const
    {
        unsigned long h = 0;
        for (int i = 0; i < (int)title.size(); i++)
            h = h * 31 + (unsigned char)title[i];
        return (int)(h % TABLE_SIZE);
    }

    // Bucket number of a book in this table.
    int bucketOf(const Book *b) const
    {
        if (byId)
            return hashId(b->id);
        return hashTitle(toLower(b->title));
    }

public:
    HashTable(bool keyIsId) : byId(keyIsId)
    {
        for (int i = 0; i < TABLE_SIZE; i++)
            buckets[i] = 0;
    }

    ~HashTable()
    {
        for (int i = 0; i < TABLE_SIZE; i++)
        {
            HashEntry *cur = buckets[i];
            while (cur != 0)
            {
                HashEntry *old = cur;
                cur = cur->next;
                delete old;         // only the entry, not the Book
            }
        }
    }

    // Inserts a book at the front of its bucket. O(1)
    void insert(Book *book)
    {
        int index = bucketOf(book);
        HashEntry *entry = new HashEntry;
        entry->book = book;
        entry->next = buckets[index];
        buckets[index] = entry;
    }

    // Removes a book from the table (does not delete the Book). O(1) average
    bool remove(Book *book)
    {
        int index = bucketOf(book);
        HashEntry *prev = 0;
        HashEntry *cur = buckets[index];
        while (cur != 0 && cur->book != book)
        {
            prev = cur;
            cur = cur->next;
        }
        if (cur == 0)
            return false;
        if (prev == 0)
            buckets[index] = cur->next;
        else
            prev->next = cur->next;
        delete cur;
        return true;
    }

    // Looks up a book by ID (use on the ID table). O(1) average
    Book *findById(int id) const
    {
        for (HashEntry *cur = buckets[hashId(id)]; cur != 0; cur = cur->next)
            if (cur->book->id == id)
                return cur->book;
        return 0;
    }

    // Finds all books with this title, ignoring case (use on the title table).
    // Fills results[] and returns how many were found. O(1) average
    int findByTitle(const string &title, Book *results[], int maxResults) const
    {
        string key = toLower(title);
        int found = 0;
        for (HashEntry *cur = buckets[hashTitle(key)]; cur != 0; cur = cur->next)
        {
            if (toLower(cur->book->title) == key && found < maxResults)
                results[found++] = cur->book;
        }
        return found;
    }
};

// Member record and the list of members
struct Member
{
    int id;
    string name;
    BorrowedList borrowed;   // books this member currently holds
    Member *next;

    Member(int memberId, const string &n) : id(memberId), name(n), next(0) {}
};

