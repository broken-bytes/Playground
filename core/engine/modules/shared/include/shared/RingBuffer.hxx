template <typename T, size_t Capacity>
class RingBuffer {
    static_assert((Capacity& (Capacity - 1)) == 0, "Capacity must be power of 2");

public:
    RingBuffer() : head(0), tail(0) {}

    // Returns false if full
    bool enqueue(const T& item) {
        size_t currentTail = tail.load(std::memory_order_relaxed);
        size_t nextTail = (currentTail + 1) & (Capacity - 1);

        if (nextTail == head.load(std::memory_order_acquire)) {
            return false; // Full
        }

        buffer[currentTail] = item;
        tail.store(nextTail, std::memory_order_release);
        return true;
    }

    // Returns false if empty
    bool dequeue(T& outItem) {
        size_t currentHead = head.load(std::memory_order_relaxed);
        if (currentHead == tail.load(std::memory_order_acquire)) {
            return false; // Empty
        }

        outItem = buffer[currentHead];
        head.store((currentHead + 1) & (Capacity - 1), std::memory_order_release);
        return true;
    }

    bool isEmpty() const {
        return head.load() == tail.load();
    }

    bool isFull() const {
        return ((tail.load() + 1) & (Capacity - 1)) == head.load();
    }

private:
    std::array<T, Capacity> buffer;
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
};
