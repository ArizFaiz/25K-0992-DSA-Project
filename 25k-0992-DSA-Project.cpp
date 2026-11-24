/*
 * Data Structures LAB Project
 * Ariz Faiz Siddiqui (25K-0992), Section J
 
 * A console program that manages books, members, issuing, returning and waiting lists for a small library.
 * All main data structures are written by hand (no STL containers):
 * Module 1 - Singly linked list : BookList, BorrowedList (+ merge sort by ID)
 * Module 2 - Hash table         : HashTable (separate chaining)
 * Module 3 - Queue (FIFO)       : WaitQueue, plus the menu interface
*/

#include <iostream>
#include <string>
using namespace std;

// Constants
const int MAX_BORROW = 3;     // most books one member can hold at a time
const int TABLE_SIZE = 101;   // number of buckets in each hash table
const int MAX_RESULTS = 50;   // most results shown by a title search

// Small helper functions

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
class QueueNode
{
public:
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

public:
    WaitQueue()
    {
        front = 0;
        rear = 0;
        count = 0;
    }

    ~WaitQueue()
    {
        clear();
    }

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

    bool isEmpty() const
    {
        return count == 0;
    }

    int size() const
    {
        return count;
    }

    // Checks if a member is already waiting. O(n)
    bool contains(int memberId) const
    {
        for (QueueNode *cur = front; cur != 0; cur = cur->next)
            if (cur->memberId == memberId)
                return true;
        return false;
    }

    // Gives access to the first node so the list can be printed.
    QueueNode *getFront() const
    {
        return front;
    }

    // Deletes every node.
    void clear()
    {
        while (front != 0)
        {
            QueueNode *old = front;
            front = front->next;
            delete old;
        }
        rear = 0;
        count = 0;
    }
};

// Book record. Each book keeps its own waiting list.
class Book
{
public:
    int id;
    string title;
    string author;
    int issuedTo;        // member ID who has the book, 0 means available
    WaitQueue waitlist;  // members waiting for this book

    Book(int bookId, const string &t, const string &a)
    {
        id = bookId;
        title = t;
        author = a;
        issuedTo = 0;
    }

    bool isAvailable() const
    {
        return issuedTo == 0;
    }
};

// MODULE 1: LINKED LISTS

// BorrowedList: the book IDs one member has borrowed
class IntNode
{
public:
    int value;
    IntNode *next;
};

class BorrowedList
{
private:
    IntNode *head;
    int count;

public:
    BorrowedList()
    {
        head = 0;
        count = 0;
    }

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

    int size() const
    {
        return count;
    }

    IntNode *getHead() const
    {
        return head;
    }
};

// BookList: all books in the library
class BookNode
{
public:
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

public:
    BookList()
    {
        head = 0;
        tail = 0;
        count = 0;
    }

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

    int size() const
    {
        return count;
    }

    BookNode *getHead() const
    {
        return head;
    }
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
class HashEntry
{
public:
    Book *book;
    HashEntry *next;
};

// The same class is used twice: once keyed by book ID and once keyed by
// title. setKeyIsId() decides which hash function and key are used.
class HashTable
{
private:
    HashEntry *buckets[TABLE_SIZE];
    bool byId;

    int hashId(int id) const
    {
        return id % TABLE_SIZE;
    }

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
    HashTable()
    {
        byId = true;
        for (int i = 0; i < TABLE_SIZE; i++)
            buckets[i] = 0;
    }

    // Chooses what this table uses as its key: true = book ID, false = title.
    void setKeyIsId(bool keyIsId)
    {
        byId = keyIsId;
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
class Member
{
public:
    int id;
    string name;
    BorrowedList borrowed;   // books this member currently holds
    Member *next;

    Member(int memberId, const string &n)
    {
        id = memberId;
        name = n;
        next = 0;
    }
};

class MemberList
{
private:
    Member *head;
    int count;

public:
    MemberList()
    {
        head = 0;
        count = 0;
    }

    ~MemberList()
    {
        while (head != 0)
        {
            Member *old = head;
            head = head->next;
            delete old;
        }
    }

    void add(Member *m)
    {
        m->next = head;     // insert at the front
        head = m;
        count++;
    }

    // Linear search by member ID. O(n)
    Member *find(int id) const
    {
        for (Member *cur = head; cur != 0; cur = cur->next)
            if (cur->id == id)
                return cur;
        return 0;
    }

    int size() const
    {
        return count;
    }
};

// Input helpers (safe reading from the keyboard)

string readLine(const string &prompt)
{
    cout << prompt;
    string line;
    if (!getline(cin, line))
    {
        cout << "\nInput ended. Exiting." << endl;
        throw 0;
    }
    return trim(line);
}

// Keeps asking until the user types a whole number (at most 9 digits).
int readInt(const string &prompt)
{
    while (true)
    {
        string line = readLine(prompt);
        int i = 0;
        bool negative = false;
        if (i < (int)line.size() && (line[i] == '-' || line[i] == '+'))
        {
            negative = (line[i] == '-');
            i++;
        }
        int digits = (int)line.size() - i;
        bool valid = digits >= 1 && digits <= 9;
        int value = 0;
        for (int j = i; valid && j < (int)line.size(); j++)
        {
            if (line[j] < '0' || line[j] > '9')
                valid = false;
            else
                value = value * 10 + (line[j] - '0');
        }
        if (valid)
            return negative ? -value : value;
        cout << "Please enter a valid number." << endl;
    }
}

// Keeps asking until the user types something.
string readNonEmpty(const string &prompt)
{
    while (true)
    {
        string line = readLine(prompt);
        if (!line.empty())
            return line;
        cout << "This cannot be empty." << endl;
    }
}

bool readYesNo(const string &prompt)
{
    while (true)
    {
        string line = toLower(readLine(prompt));
        if (line == "y" || line == "yes") return true;
        if (line == "n" || line == "no") return false;
        cout << "Please type y or n." << endl;
    }
}

// LIBRARY: connects all the modules together
class Library
{
private:
    BookList books;         // Module 1: all books
    HashTable idTable;      // Module 2: search by ID
    HashTable titleTable;   // Module 2: search by title
    MemberList members;

    void printHeader() const
    {
        cout << pad("ID", 6) << pad("Title", 30)
             << pad("Author", 22) << "Status" << endl;
        cout << string(78, '-') << endl;
    }

    void printBookRow(const Book *b) const
    {
        cout << pad(intToStr(b->id), 6) << pad(b->title, 30)
             << pad(b->author, 22);
        if (b->isAvailable())
            cout << "Available";
        else
        {
            Member *m = members.find(b->issuedTo);
            cout << "Issued to " << (m ? m->name : string("?"))
                 << " (ID " << b->issuedTo << ")";
            if (b->waitlist.size() > 0)
                cout << ", " << b->waitlist.size() << " waiting";
        }
        cout << endl;
    }

public:
    Library()
    {
        idTable.setKeyIsId(true);
        titleTable.setKeyIsId(false);
    }

    // Book operations

    void addBook(int id, const string &title, const string &author)
    {
        if (id <= 0)
        {
            cout << "Error: book ID must be a positive number." << endl;
            return;
        }
        if (idTable.findById(id) != 0)       // duplicate check, O(1)
        {
            cout << "Error: a book with ID " << id << " already exists." << endl;
            return;
        }
        Book *b = new Book(id, title, author);
        books.add(b);           // store in the linked list
        idTable.insert(b);      // index by ID
        titleTable.insert(b);   // index by title
        cout << "Book added: " << title << endl;
    }

    void removeBook(int id)
    {
        Book *b = idTable.findById(id);
        if (b == 0)
        {
            cout << "Error: no book with ID " << id << "." << endl;
            return;
        }
        if (!b->isAvailable())
        {
            cout << "Error: this book is currently issued and cannot be removed." << endl;
            return;
        }
        // Remove from both hash tables first, because deleting the book
        // from the list will free the Book object.
        idTable.remove(b);
        titleTable.remove(b);
        books.remove(id);
        cout << "Book removed." << endl;
    }

    void updateBook(int id, const string &newTitle, const string &newAuthor)
    {
        Book *b = idTable.findById(id);
        if (b == 0)
        {
            cout << "Error: no book with ID " << id << "." << endl;
            return;
        }
        // The title is the key of titleTable, so take the book out
        // before changing it and put it back afterwards.
        titleTable.remove(b);
        if (!newTitle.empty())  b->title = newTitle;
        if (!newAuthor.empty()) b->author = newAuthor;
        titleTable.insert(b);
        cout << "Book updated." << endl;
    }

    // Member operations

    void registerMember(int id, const string &name)
    {
        if (id <= 0)
        {
            cout << "Error: member ID must be a positive number." << endl;
            return;
        }
        if (members.find(id) != 0)
        {
            cout << "Error: a member with ID " << id << " already exists." << endl;
            return;
        }
        members.add(new Member(id, name));
        cout << "Member registered: " << name << endl;
    }

    // Issue and return

    void issueBook(int bookId, int memberId)
    {
        Book *b = idTable.findById(bookId);
        Member *m = members.find(memberId);
        if (b == 0)
        {
            cout << "Error: no book with ID " << bookId << "." << endl;
            return;
        }
        if (m == 0)
        {
            cout << "Error: no member with ID " << memberId << "." << endl;
            return;
        }
        if (b->issuedTo == memberId)
        {
            cout << "Error: " << m->name << " already has this book." << endl;
            return;
        }
        if (m->borrowed.size() >= MAX_BORROW)
        {
            cout << "Error: " << m->name << " has reached the limit of "
                 << MAX_BORROW << " books." << endl;
            return;
        }

        if (b->isAvailable())
        {
            b->issuedTo = memberId;
            m->borrowed.add(bookId);
            cout << "Book \"" << b->title << "\" issued to " << m->name << "." << endl;
            return;
        }

        // The book is taken. Offer a place in the waiting list.
        cout << "This book is already issued." << endl;
        if (b->waitlist.contains(memberId))
        {
            cout << m->name << " is already in the waiting list." << endl;
            return;
        }
        if (readYesNo("Join the waiting list? (y/n): "))
        {
            b->waitlist.enqueue(memberId);
            cout << m->name << " added to the waiting list (position "
                 << b->waitlist.size() << ")." << endl;
        }
    }

    void returnBook(int bookId, int memberId)
    {
        Book *b = idTable.findById(bookId);
        Member *m = members.find(memberId);
        if (b == 0)
        {
            cout << "Error: no book with ID " << bookId << "." << endl;
            return;
        }
        if (m == 0)
        {
            cout << "Error: no member with ID " << memberId << "." << endl;
            return;
        }
        if (!m->borrowed.contains(bookId))      // book was never borrowed by him
        {
            cout << "Error: " << m->name << " has not borrowed this book." << endl;
            return;
        }

        m->borrowed.remove(bookId);
        b->issuedTo = 0;
        cout << "Book \"" << b->title << "\" returned by " << m->name << "." << endl;

        // Give the book to the first eligible member in the waiting list.
        int nextId;
        while (b->waitlist.dequeue(nextId))
        {
            Member *next = members.find(nextId);
            if (next->borrowed.size() >= MAX_BORROW)
            {
                cout << next->name << " was skipped (borrow limit reached)." << endl;
                continue;
            }
            b->issuedTo = nextId;
            next->borrowed.add(bookId);
            cout << "The book was given to " << next->name
                 << " from the waiting list." << endl;
            return;
        }
        cout << "The book is now available." << endl;
    }

    // Searching and display

    void searchById(int id) const
    {
        Book *b = idTable.findById(id);
        if (b == 0)
        {
            cout << "No book found with ID " << id << "." << endl;
            return;
        }
        printHeader();
        printBookRow(b);
    }

    void searchByTitle(const string &title) const
    {
        Book *results[MAX_RESULTS];
        int n = titleTable.findByTitle(title, results, MAX_RESULTS);
        if (n == 0)
        {
            cout << "No book found with the title \"" << title << "\"." << endl;
            return;
        }
        printHeader();
        for (int i = 0; i < n; i++)
            printBookRow(results[i]);
    }

    void showAllBooks() const
    {
        if (books.size() == 0)
        {
            cout << "The library has no books." << endl;
            return;
        }
        printHeader();
        for (BookNode *cur = books.getHead(); cur != 0; cur = cur->next)
            printBookRow(cur->book);
        cout << "Total books: " << books.size() << endl;
    }

    // Shows all books sorted by ID. The sort is done on a copy of the list
    // (the nodes only point to the same books), so the original order of
    // the library list is not changed.
    void showBooksSortedById() const
    {
        if (books.size() == 0)
        {
            cout << "The library has no books." << endl;
            return;
        }
        BookNode *copyHead = 0;
        BookNode *copyTail = 0;
        for (BookNode *cur = books.getHead(); cur != 0; cur = cur->next)
        {
            BookNode *node = new BookNode;
            node->book = cur->book;
            node->next = 0;
            if (copyTail == 0)
                copyHead = copyTail = node;
            else
            {
                copyTail->next = node;
                copyTail = node;
            }
        }

        copyHead = mergeSortList(copyHead);

        printHeader();
        BookNode *cur = copyHead;
        while (cur != 0)
        {
            printBookRow(cur->book);
            BookNode *old = cur;
            cur = cur->next;
            delete old;             // delete only the copy node, not the book
        }
        cout << "Total books: " << books.size() << endl;
    }

    void showIssuedBooks() const
    {
        bool any = false;
        for (BookNode *cur = books.getHead(); cur != 0; cur = cur->next)
        {
            if (!cur->book->isAvailable())
            {
                if (!any)
                    printHeader();
                any = true;
                printBookRow(cur->book);
            }
        }
        if (!any)
            cout << "No books are issued right now." << endl;
    }

    void showWaitlist(int bookId) const
    {
        Book *b = idTable.findById(bookId);
        if (b == 0)
        {
            cout << "Error: no book with ID " << bookId << "." << endl;
            return;
        }
        if (b->waitlist.isEmpty())
        {
            cout << "Nobody is waiting for \"" << b->title << "\"." << endl;
            return;
        }
        cout << "Waiting list for \"" << b->title << "\":" << endl;
        int position = 1;
        for (QueueNode *cur = b->waitlist.getFront(); cur != 0; cur = cur->next)
        {
            Member *m = members.find(cur->memberId);
            cout << "  " << position++ << ". " << m->name
                 << " (ID " << m->id << ")" << endl;
        }
    }

    void showMemberBooks(int memberId) const
    {
        Member *m = members.find(memberId);
        if (m == 0)
        {
            cout << "Error: no member with ID " << memberId << "." << endl;
            return;
        }
        if (m->borrowed.size() == 0)
        {
            cout << m->name << " has not borrowed any books." << endl;
            return;
        }
        cout << m->name << " has borrowed (" << m->borrowed.size()
             << "/" << MAX_BORROW << "):" << endl;
        for (IntNode *cur = m->borrowed.getHead(); cur != 0; cur = cur->next)
        {
            Book *b = idTable.findById(cur->value);
            if (b != 0)
                cout << "  " << b->id << " - " << b->title << " by " << b->author << endl;
        }
    }

    // Adds some books and members so the program can be demonstrated quickly.
    void loadSampleData()
    {
        addBook(101, "Data Structures in C++", "Mark Weiss");
        addBook(102, "Introduction to Algorithms", "Thomas Cormen");
        addBook(103, "The C++ Programming Language", "Bjarne Stroustrup");
        addBook(104, "Clean Code", "Robert Martin");
        addBook(105, "Operating System Concepts", "Abraham Silberschatz");
        registerMember(1, "Ali");
        registerMember(2, "Sara");
        registerMember(3, "Hamza");
    }
};

// Menu (part of Module 3)

void printMenu()
{
    cout << endl;
    cout << "===== LIBRARY MANAGEMENT SYSTEM =====" << endl;
    cout << " 1. Add a book" << endl;
    cout << " 2. Remove a book" << endl;
    cout << " 3. Update a book" << endl;
    cout << " 4. Register a member" << endl;
    cout << " 5. Issue a book" << endl;
    cout << " 6. Return a book" << endl;
    cout << " 7. Search a book by ID" << endl;
    cout << " 8. Search a book by title" << endl;
    cout << " 9. Show all books" << endl;
    cout << "10. Show issued books" << endl;
    cout << "11. Show waiting list of a book" << endl;
    cout << "12. Show books borrowed by a member" << endl;
    cout << "13. Load sample data" << endl;
    cout << "14. Show all books sorted by ID" << endl;
    cout << " 0. Exit" << endl;
}

int main()
{
    Library lib;
    int choice = -1;

    try
    {
    while (choice != 0)
    {
        printMenu();
        choice = readInt("Enter your choice: ");
        cout << endl;

        switch (choice)
        {
        case 1:
        {
            int id = readInt("Book ID: ");
            string title = readNonEmpty("Title: ");
            string author = readNonEmpty("Author: ");
            lib.addBook(id, title, author);
            break;
        }
        case 2:
            lib.removeBook(readInt("Book ID to remove: "));
            break;
        case 3:
        {
            int id = readInt("Book ID to update: ");
            string title = readLine("New title (press Enter to keep): ");
            string author = readLine("New author (press Enter to keep): ");
            lib.updateBook(id, title, author);
            break;
        }
        case 4:
        {
            int id = readInt("Member ID: ");
            string name = readNonEmpty("Name: ");
            lib.registerMember(id, name);
            break;
        }
        case 5:
        {
            int bookId = readInt("Book ID: ");
            int memberId = readInt("Member ID: ");
            lib.issueBook(bookId, memberId);
            break;
        }
        case 6:
        {
            int bookId = readInt("Book ID: ");
            int memberId = readInt("Member ID: ");
            lib.returnBook(bookId, memberId);
            break;
        }
        case 7:
            lib.searchById(readInt("Book ID: "));
            break;
        case 8:
            lib.searchByTitle(readNonEmpty("Title: "));
            break;
        case 9:
            lib.showAllBooks();
            break;
        case 10:
            lib.showIssuedBooks();
            break;
        case 11:
            lib.showWaitlist(readInt("Book ID: "));
            break;
        case 12:
            lib.showMemberBooks(readInt("Member ID: "));
            break;
        case 13:
            lib.loadSampleData();
            break;
        case 14:
            lib.showBooksSortedById();
            break;
        case 0:
            cout << "Goodbye!" << endl;
            break;
        default:
            cout << "Invalid choice. Please pick a number from the menu." << endl;
        }
    }
    }
    catch (int)
    {
    }
    return 0;
}
