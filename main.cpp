#include <mutex>
#include <iostream>
#include <queue>
#include <iostream>
#include <thread>

struct Node
{
	int value;
	Node* next;
	std::mutex* node_mutex;
};

class FineGrainedQueue
{
	Node* head;
	std::mutex* queue_mutex;
public:
    //FineGrainedQueue() : head(nullptr) {}
    void show();
    void insert(int pos, int data);
    void insert_first(int data);
    void remove(int value);
    void insert_thread(int pos, int data);
};

void FineGrainedQueue::show()
{
    Node* node = head;
    while (node != nullptr)
    {
        std::cout << node->value << " ";
        node = node->next;
    }
}

void FineGrainedQueue::insert_first(int data)
{
    //head = nullptr;
    Node* node = new Node;
    node->value = data;
    node->next = nullptr;
    head = node;
}

void FineGrainedQueue::insert(int pos, int data)
{
    Node* newNode = new Node;
    newNode->value = data;
    newNode->next = nullptr;
    if (head == nullptr)
    {
        head = newNode;
        return;
    }

    if (pos == 0)
    {
        newNode->next = head;
        head = newNode;
        return;
    }

    int currPos = 0;

    Node* current = head;
    while (currPos < pos - 1 && current->next != nullptr)
    {
        current = current->next;
        currPos++;
    }
    Node* next = current->next;
    current->next = newNode;
    newNode->next = next;
}

void FineGrainedQueue::insert_thread(int pos, int data)
{
    Node* newNode = new Node;
    newNode->value = data;
    newNode->next = nullptr;
    int currPos = 0;

    queue_mutex->lock();
    Node* current = head;
    queue_mutex->unlock();
    if (current) 
        current->node_mutex->lock();
    while (currPos < pos - 1 && current->next != nullptr)
    {
        current = current->next;
        current->node_mutex->unlock();
        currPos++;
    }
    Node* next = current->next;
    current->next = newNode;
    newNode->next = next;
}

void FineGrainedQueue::remove(int value)
{
    Node* prev = new Node;
    Node* cur = new Node;
    queue_mutex->lock();

    prev = head;
    cur = head->next;

    prev->node_mutex->lock();
    queue_mutex->unlock();

    if (cur) 
        cur->node_mutex->lock();
    while (cur)
    {
        if (cur->value == value)
        {
            prev->next = cur->next;
            prev->node_mutex->unlock();
            cur->node_mutex->unlock();
            delete cur;
            return;
        }
        Node* old_prev = prev;
        prev = cur;
        cur = cur->next;
        old_prev->node_mutex->unlock();
        if (cur) 
            cur->node_mutex->lock();
    }
    prev->node_mutex->unlock();
}

int main()
{
    FineGrainedQueue qu;
    qu.insert_first(111);
    qu.insert(2, 987);
    qu.insert(3, 876);
    qu.insert(4, 765);
    qu.show();
    std::cout << std::endl;

    std::thread t1(&FineGrainedQueue::insert_thread, qu, 5, 555);
    std::thread t2(&FineGrainedQueue::insert_thread, qu, 5, 666);
    std::thread t3(&FineGrainedQueue::insert_thread, qu, 10, 777);
    if (t1.joinable())
        t1.join();
    if (t2.joinable())
        t2.join();
    if (t3.joinable())
        t3.join();
    qu.show();
    std::cout << std::endl;
    std::thread t4(&FineGrainedQueue::remove, qu, 876);
    if (t4.joinable())
        t4.join();
    qu.show();

    return 0;
}