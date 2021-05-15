#pragma once
#include <mutex>
#include <map>
#include <vector>#include <mutex>
#include <map>
#include <vector>


template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access {
        Value& ref_to_value;

        Access(Value& ref_to_value_, std::mutex& m)
            : ref_to_value(ref_to_value_),
            m_(m)
        {
        }

        ~Access() {
            //end mutex zone
            m_.unlock();
        }

    private:
        std::mutex& m_;
    };

    explicit ConcurrentMap(size_t bucket_count) : bucket_count_(bucket_count) {
        // создадим вектор необходимого размера и мьютексы для каждого словаря в векторе
        bucket_ = std::vector<std::map<Key, Value>>(bucket_count_);
        bucket_mutex_ = std::vector<std::mutex>(bucket_count_);
    }

    Access operator[](const Key& key) {
        // работа этого оператора возвращает ссылку на струкруру, которая полностью блокирует часть словаря. поэтому время жизни 
        // этой структуры должно быть минимально
        size_t bucketNum = getBucketNum(key);
        std::map<Key, Value>& ref_to_map = bucket_[bucketNum];
        std::mutex& ref_to_mutex = bucket_mutex_[bucketNum];
        //start mutex zone
        ref_to_mutex.lock();
        Value& ref_to_value = ref_to_map[key];
        return Access(ref_to_value, ref_to_mutex);
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> ordinaryMap(bucket_[0]);
        for (size_t i = 1; i < bucket_count_; ++i) {
            std::lock_guard g(bucket_mutex_[i]); // Потокобезопасность другие потоки могут работать с другими частями частями словаря
            ordinaryMap.insert(bucket_[i].begin(), bucket_[i].end());
        }
        return ordinaryMap;
    };

private:
    size_t getBucketNum(const Key& key) {
        // номером части словаря будет остаток от деления ключа на количество ключей   
        return key % bucket_count_;
    }

    std::vector<std::map<Key, Value>> bucket_;
    std::vector<std::mutex> bucket_mutex_;
    const size_t bucket_count_;
};