#pragma once
#include <vector>
#include <functional>
#include <stdexcept>

template<typename T>
class Queue {
private:
    std::vector<T> buf_;
    size_t head_;
    size_t tail_;
    size_t count_;
    size_t capacity_;

public:
    Queue(size_t initialCap = 8) 
        : buf_(initialCap), head_(0), tail_(0), count_(0), capacity_(initialCap) {}

    void enqueue(const T& value) {
        if(count_ == capacity_) {
            // Resize buffer
            size_t newCapacity = capacity_ * 2;
            std::vector<T> newBuf(newCapacity);
            
            for(size_t i = 0; i < count_; i++) {
                newBuf[i] = buf_[(head_ + i) % capacity_];
            }
            
            buf_ = newBuf;
            head_ = 0;
            tail_ = count_;
            capacity_ = newCapacity;
        }
        
        buf_[tail_] = value;
        tail_ = (tail_ + 1) % capacity_;
        count_++;
    }

    void dequeue() {
        if(isEmpty()) 
            throw std::runtime_error("Queue underflow");
        
        head_ = (head_ + 1) % capacity_;
        count_--;
    }

    T& front() {
        if(isEmpty())
            throw std::runtime_error("Queue is empty");
        return buf_[head_];
    }

    const T& front() const {
        if(isEmpty())
            throw std::runtime_error("Queue is empty");
        return buf_[head_];
    }

    T& back() {
        if(isEmpty())
            throw std::runtime_error("Queue is empty");
        return buf_[(tail_ + capacity_ - 1) % capacity_];
    }

    bool isEmpty() const { return count_ == 0; }
    size_t size() const { return count_; }
    size_t capacity() const { return capacity_; }

    void clear() {
        head_ = tail_ = count_ = 0;
    }

    bool contains(const T& value) const {
        for(size_t i = 0; i < count_; i++) {
            if(buf_[(head_ + i) % capacity_] == value)
                return true;
        }
        return false;
    }

    bool removeIf(const std::function<bool(const T&)>& pred) {
        std::vector<T> newBuf(capacity_);
        size_t newCount = 0;
        
        for(size_t i = 0; i < count_; i++) {
            T& elem = buf_[(head_ + i) % capacity_];
            if(!pred(elem)) {
                newBuf[newCount++] = elem;
            }
        }
        
        if(newCount == count_) return false;
        
        buf_ = newBuf;
        head_ = 0;
        tail_ = newCount;
        count_ = newCount;
        return true;
    }

    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count_);
        for(size_t i = 0; i < count_; i++) {
            result.push_back(buf_[(head_ + i) % capacity_]);
        }
        return result;
    }
};
