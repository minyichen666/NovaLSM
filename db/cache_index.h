#ifndef LEVELDB_CACHE_INDEX_H
#define LEVELDB_CACHE_INDEX_H

#include <atomic>
#include "leveldb/slice.h"
#include <list>
#include <mutex>
#include <unordered_map>
#include <memory>


namespace leveldb {

    struct Node{
        Node *prev;
        Node *next;
        uint64_t hash; //it is used to erase Node in tableid_map
        uint64_t table_id;
        bool is_memtableid;
        ~Node(){}
    };

    class LinkedList{
        public:
            LinkedList() : head(nullptr), tail(nullptr){};
            void push_front(Node *node);
            void remove_without_freeing(Node *node);
            void remove(Node *node);
            Node *head;
            Node *tail;
            ~LinkedList();
    };

    struct CachePartition{
        std::unordered_map<uint64_t, Node*> *tableid_map; //store memtable_id/sstable_id/tombstone in each cache partition
        uint32_t size; // define the size for each partition 
        std::mutex mutex;
        LinkedList *lru_queue; //Doubly linked list for LRU eviction for each partition
        uint64_t cache_hits;
        uint64_t cache_misses;
    };

    class CacheIndex {
        public:
            CacheIndex(uint32_t partition_num, uint32_t partition_size);

            ~CacheIndex();

            uint64_t get(const Slice &key, uint64_t hash, bool &is_memtableid);

            void put(const Slice &key, uint64_t hash, uint64_t memtableid, bool is_memtableid);

            bool remove(const Slice &key, uint64_t hash);
            
            void cache_stats(int *cache_hits, int *cache_misses, int* sizes); //return cache hits and misses for each partition

        private:
            uint32_t partition_num_; //the size of the cache_index
            CachePartition *cache_index_; //an array of cache partition

    };
}

#endif //LEVELDB_CACHE_INDEX_H