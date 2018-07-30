#pragma once

#include <mutex>
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
    
};


template <>
class RingQueue<AVFrame*> {
private:
    std::mutex mutex_;
    std::condition_variable cond_;

    std::string name_;

    unsigned queue_size_;
    unsigned head_;
    unsigned tail_;
    AVFrame** queue_;
public:
    RingQueue(const unsigned& size) : queue_size_{size}, head_{0}, tail_{0} {
        this->queue_ = new AVFrame*[size];
    }

    ~RingQueue() {
        if (this->queue_ != nullptr) {
            delete[] this->queue_;
        }
    }

    void put(AVFrame** data) {
        std::unique_lock<std::mutex> lock(this->mutex_);

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
        std::unique_lock<std::mutex> lock(this->mutex_);
        
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
    
};

template<>
class RingQueue <AVPacket*> {
private:
    std::mutex mutex_;
    std::condition_variable cond_;

    std::string name_;

    unsigned queue_size_;
    unsigned head_;
    unsigned tail_;
    AVPacket** queue_;
public:
    RingQueue(const unsigned& size) : queue_size_{size}, head_{0}, tail_{0} {
        this->queue_ = new AVPacket*[size];
        
        for (unsigned i = 0; i < size; i++) {
            this->queue_[i] = nullptr; // Guarantee that everything start as null
        }
    }

    ~RingQueue() {
        if (this->queue_ != nullptr) {
            delete[] this->queue_;
        }
    }

    void put(AVPacket** data) {
        std::unique_lock<std::mutex> lock(this->mutex_);
        
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
        std::unique_lock<std::mutex> lock(this->mutex_);
        
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
    
};