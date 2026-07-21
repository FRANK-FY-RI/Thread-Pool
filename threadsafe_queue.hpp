#ifndef __THREADSAFE_QUEUE
#define __THREADSAFE_QUEUE

#include <condition_variable>
#include <memory>
#include <atomic>
#include <mutex>
#include <utility>

/*
A more granular lock-based thread safe queue implemented using a linked list with a dummy node.
*/

template <typename T, typename Alloc = std::allocator<T>>
class threadsafe_queue { 
    struct Node {
        T val;
        Node *next{nullptr};
        Node() : next(nullptr) {}
        template <typename U>
        Node(U&& value) : val(std::forward<U>(value)) {}
    };
    using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    node_allocator alloc;
    alignas(64) mutable std::mutex pushmt;
    alignas(64) mutable std::mutex popmt;
    std::condition_variable cv;
    std::atomic<bool> closed{false};
    using guard_type = std::lock_guard<std::mutex>;
    Node *head{nullptr}, *tail{nullptr};

    Node* get_head() const {
        guard_type guard(pushmt);
        return head;
    }
public:
    threadsafe_queue() {
        head = tail = alloc.allocate(1);
    }
    threadsafe_queue(const threadsafe_queue& other) {
        guard_type push_guard(other.pushmt);
        guard_type pop_guard(other.popmt);
        Node *otherptr = other.tail, *thisptr = tail;
        while(otherptr) {
            Node *newNode = alloc.allocate(1, otherptr->val);
            if(!tail) tail = thisptr = newNode;
            else {
                thisptr->next = newNode;
                thisptr = thisptr->next;
            }
            otherptr = otherptr->next;
        }
        head = thisptr;
    }
    threadsafe_queue& operator=(const threadsafe_queue&)=delete; 
    threadsafe_queue(threadsafe_queue&&)=delete;
    threadsafe_queue& operator=(threadsafe_queue&&)=delete;
    ~threadsafe_queue() {
        close();
        while(tail) {
            Node *ptr = tail;
            tail = tail->next;
            alloc.deallocate(ptr, 1);
        }
    }

    template <typename U>
    void push(U&& val) {
        Node* ptr = alloc.allocate(1);
        {
            guard_type guard(pushmt);
            head->val = std::forward<U>(val);
            head->next = ptr;
            head = head->next;
        }
        cv.notify_one();
    }

    bool try_pop(T& res) {
        Node *ptr;
        {
            guard_type guard(popmt);
            if(tail == get_head()) return false;
            res = std::move(tail->val);
            ptr = tail;
            tail = tail->next;
        }
        alloc.deallocate(ptr, 1);
        return true;
    }

    std::shared_ptr<T> try_pop() {
        Node *ptr;
        std::shared_ptr<T> res;
        {
            guard_type guard(popmt);
            if(tail == get_head()) return nullptr;
            res = std::make_shared<T>(std::move(tail->val));
            ptr = tail;
            tail = tail->next;
        }
        alloc.deallocate(ptr, 1);
        return res;
    }

    bool wait_and_pop(T& res) {
        Node *ptr;
        {
            std::unique_lock<std::mutex> uq(popmt);
            cv.wait(uq, [this](){return closed || tail!=get_head();}); 
            if(closed) return false;
            res = std::move(tail->val);
            ptr = tail;
            tail = tail->next;
        }
        alloc.deallocate(ptr, 1);
        return true;
    }

    std::shared_ptr<T> wait_and_pop() {
        Node *ptr;
        std::shared_ptr<T> res;
        {
            std::unique_lock<std::mutex> uq(popmt);
            cv.wait(uq, [this](){return closed || tail!=get_head();});
            if(closed) return nullptr;
            res = std::make_shared<T>(std::move(tail->val));
            ptr = tail;
            tail = tail->next;
        }
        alloc.deallocate(ptr, 1);
        return res;
    }

    bool empty() const {
        guard_type guard_pop(popmt);
        guard_type guard_push(pushmt);
        return head == tail;
    }

    void close() {
        closed = true;
        cv.notify_all();
    }

};


#endif
