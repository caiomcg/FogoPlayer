#pragma once

#include <mutex>
#include <iostream>
#include <condition_variable>

extern "C" {
    #include <libavcodec/avcodec.h>
}

template <typename T>
class RingQueue {
private:
    std::mutex mutex_;
    std::condition_variable cond_;

    std::string name_;

    unsigned queue_size_;
    unsigned head_;
    unsigned tail_;
    T* queue_;
public:
    RingQueue(const unsigned& size) : queue_size_{size}, head_{0}, tail_{0} {
        this->queue_ = new T[size];
    }

    ~RingQueue() {
        if (this->queue_ != nullptr) {
            delete[] this->queue_;
        }
    }

    void put(T& data) {
        std::unique_lock<std::mutex> lock(this->mutex_);
        
        this->queue_[this->head_] = data;
        this->head_ = (this->head_ + 1) % this->queue_size_;

        if (this->isEmpty()) {
            this->tail_ = (this->tail_ + 1) % this->queue_size_;
        }

        lock.unlock();
        this->cond_.notify_one();
    }
    
    T take() {
        std::unique_lock<std::mutex> lock(this->mutex_);
        
        while (this->isEmpty()) {
            this->cond_.wait(lock);
        }
    
        T data = this->queue_[this->tail_];
        this->tail_ = (this->tail_ + 1) % this->queue_size_;
        
        lock.unlock();
        return data;
    }

    bool isEmpty() {
        return this->head_ == this->tail_;
    }

    int currentSize() {
        return this->queue_size_ - (this->tail_ - this->head_) + (-((int) (this->tail_ <= this->head_)) & this->queue_size_);
    }
    
};


template <>
class RingQueue<AVFrame*> {
private:
    std::mutex mutex_;
    std::mutex take_mutex_;
    std::condition_variable cond_;
    std::condition_variable produce_cond_;

    std::string name_;

    unsigned queue_size_;
    unsigned drain_limit_;
    unsigned head_;
    unsigned tail_;

    AVFrame** queue_;
public:
    RingQueue(const unsigned& size) : queue_size_{size}, drain_limit_{((size * 50) / 100)}, head_{0}, tail_{0} {
        this->queue_ = new AVFrame*[size];
    }

    ~RingQueue() {
        if (this->queue_ != nullptr) {
            delete[] this->queue_;
        }
    }

    void put(AVFrame** data) {
        std::unique_lock<std::mutex> lock(this->mutex_);

        if (this->currentSize() == this->queue_size_ - 1) {
            while (this->currentSize() > this->drain_limit_) {
                this->produce_cond_.wait(lock);
            }
        }
    
        if (this->queue_[this->head_] != nullptr) {
            av_frame_free(&this->queue_[this->head_]);
        }
        
        this->queue_[this->head_] = *data;
        this->head_ = (this->head_ + 1) % this->queue_size_;

        if (this->isEmpty()) {
            this->tail_ = (this->tail_ + 1) % this->queue_size_;
        }

        lock.unlock();
        this->cond_.notify_one();
    }
    
    AVFrame* take() {
        std::unique_lock<std::mutex> lock(this->take_mutex_);

        if (this->currentSize() < this->drain_limit_) {
            this->produce_cond_.notify_one();
        }
        
        while (this->isEmpty()) {
            this->cond_.wait(lock);
        }
    
        AVFrame* data = this->queue_[this->tail_];
        this->queue_[this->tail_] = nullptr;
        this->tail_ = (this->tail_ + 1) % this->queue_size_;
        
        lock.unlock();
        return data;
    }

    bool isEmpty() {
        return this->head_ == this->tail_;
    }
    
    unsigned currentSize() {
        return this->queue_size_ - ((this->tail_ - this->head_) + (-((int) (this->tail_ <= this->head_)) & this->queue_size_));
    }

};

template <>
class RingQueue<AVPacket*> {
private:
    std::mutex mutex_;
    std::mutex take_mutex_;
    std::condition_variable cond_;
    std::condition_variable produce_cond_;

    std::string name_;

    unsigned queue_size_;
    unsigned drain_limit_;
    unsigned head_;
    unsigned tail_;

    AVPacket** queue_;
public:
    RingQueue(const unsigned& size) : queue_size_{size}, drain_limit_{((size * 50) / 100)}, head_{0}, tail_{0} {
        this->queue_ = new AVPacket*[size];
    }

    ~RingQueue() {
        if (this->queue_ != nullptr) {
            delete[] this->queue_;
        }
    }

    void put(AVPacket** data) {
        std::unique_lock<std::mutex> lock(this->mutex_);

        if (this->currentSize() == this->queue_size_ - 1) {
            while (this->currentSize() > this->drain_limit_) {
                this->produce_cond_.wait(lock);
            }
        }
    
        if (this->queue_[this->head_] != nullptr) {
            av_packet_unref(this->queue_[this->head_]);
        }
        
        this->queue_[this->head_] = *data;
        this->head_ = (this->head_ + 1) % this->queue_size_;

        if (this->isEmpty()) {
            this->tail_ = (this->tail_ + 1) % this->queue_size_;
        }

        lock.unlock();
        this->cond_.notify_one();
    }
    
    AVPacket* take() {
        std::unique_lock<std::mutex> lock(this->take_mutex_);

        if (this->currentSize() < this->drain_limit_) {
            this->produce_cond_.notify_one();
        }
        
        while (this->isEmpty()) {
            this->cond_.wait(lock);
        }
    
        AVPacket* data = this->queue_[this->tail_];
        this->queue_[this->tail_] = nullptr;
        this->tail_ = (this->tail_ + 1) % this->queue_size_;
        
        lock.unlock();
        return data;
    }

    bool isEmpty() {
        return this->head_ == this->tail_;
    }
    
    unsigned currentSize() {
        return this->queue_size_ - ((this->tail_ - this->head_) + (-((int) (this->tail_ <= this->head_)) & this->queue_size_));
    }

};