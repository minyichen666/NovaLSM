#include "cache_index.h"
#include "common/nova_console_logging.h"
#include <fmt/core.h>


namespace leveldb {

    void LinkedList::push_front(Node *node){
        node -> prev = nullptr;
        if(head == nullptr){
            head = node;
            tail = node;
            node -> next = nullptr;
            return;
        }
        head -> prev = node;
        node -> next = head;
        head = node;
    }

    void LinkedList::remove_without_freeing(Node *node){
        if(node == head && node == tail){
            head = nullptr;
            tail = nullptr;
        }else if(node == head){
            head = head -> next;
            if(head){
                head -> prev = nullptr;
            }
        }else if(node == tail){
            tail = tail -> prev;
            if(tail){
                tail -> next = nullptr;
            }
        }else{
            node -> prev -> next = node -> next;
            node -> next -> prev = node -> prev;
        }
    }

    void LinkedList::remove(Node *node){
        remove_without_freeing(node);
        if(node){
            if(node -> prev){
                node -> prev = nullptr;
            }
            if(node -> next){
                node -> next = nullptr;
            }
            delete node;
        }
    }

    LinkedList::~LinkedList(){
        while(head){
            remove(head);
        }
    }

    CacheIndex::CacheIndex(uint32_t partition_num, uint32_t partition_size){
        cache_index_ = new CachePartition[partition_num];
        partition_num_ = partition_num;
        for(int i = 0; i < partition_num_; i ++){
            CachePartition &partition = cache_index_[i];
            partition.size = partition_size;
            partition.lru_queue = new LinkedList();
            partition.tableid_map = new std::unordered_map<uint64_t, Node *>();
            partition.cache_hits = 0;
            partition.cache_misses = 0;
        }
        NOVA_LOG(rdmaio::INFO)
            << fmt::format("Create cache index with {} partition with size {}", partition_num, partition_size);
        int cache_sizes = 0;
        for(int i = 0; i < partition_num_; i ++){
            CachePartition &partition = cache_index_[i];
            cache_sizes += partition.tableid_map -> size();
        }
        NOVA_LOG(rdmaio::INFO)
            << fmt::format("Create cache index initial size {}", cache_sizes);
    }

    CacheIndex::~CacheIndex(){
        for(int i = 0; i < partition_num_; i ++){
            CachePartition &partition = cache_index_[i];
            delete partition.lru_queue;
            partition.tableid_map -> clear();
            delete partition.tableid_map;
        }
        delete[] cache_index_;
    }

    uint64_t CacheIndex::get(const Slice &key, uint64_t hash, bool &is_memtableid){
        CachePartition &partition = cache_index_[hash % partition_num_];
        partition.mutex.lock();
        auto it = partition.tableid_map -> find(hash);
        if(it != partition.tableid_map -> end()){
            partition.cache_hits ++;
            Node *node = it -> second;
            //if key is found，move lastest used hash to the front
            partition.lru_queue -> remove_without_freeing(node);
            partition.lru_queue -> push_front(node);
            //return value
            is_memtableid = node -> is_memtableid;
            uint64_t table_id = node -> table_id;
            partition.mutex.unlock();
            return table_id;
        }else{
            partition.cache_misses ++;
            partition.mutex.unlock();
            return 0;
        }
    }

    void CacheIndex::put(const Slice &key, uint64_t hash, uint64_t memtableid, bool is_memtableid){
        CachePartition &partition = cache_index_[hash % partition_num_];
        partition.mutex.lock();
        auto it = partition.tableid_map -> find(hash);
        //if key is not in the cache
        if(it == partition.tableid_map -> end()){
            //if cache is full, evict least recently used
            if(partition.tableid_map -> size() == partition.size){
                uint64_t evict_hash = partition.lru_queue -> tail -> hash;
                partition.lru_queue -> remove(partition.lru_queue -> tail);
                partition.tableid_map -> erase(evict_hash);
            }
        }else{
            partition.lru_queue -> remove(it -> second);
            partition.tableid_map -> erase(it -> first);
        }
        //put key value into cache
        Node *newNode = new Node();
        newNode -> table_id = memtableid;
        newNode -> is_memtableid = is_memtableid;
        newNode -> hash = hash;
        partition.lru_queue -> push_front(newNode);
        partition.tableid_map -> insert({hash, newNode});
        partition.mutex.unlock();
    }

    bool CacheIndex::remove(const Slice &key, uint64_t hash){
        CachePartition &partition = cache_index_[hash % partition_num_];
        partition.mutex.lock();
        auto it = partition.tableid_map -> find(hash);
        //if key is in the cache
        if(it != partition.tableid_map -> end()){
            partition.lru_queue -> remove(it -> second);
            partition.tableid_map -> erase(it -> first);
            partition.mutex.unlock();
            return true;
        }else{
            partition.mutex.unlock();
            return false;
        }
    }

    void CacheIndex::cache_stats(int *cache_hits, int *cache_misses, int *sizes){ 
        for(int i = 0; i < partition_num_; i ++){
            CachePartition &partition = cache_index_[i];
            partition.mutex.lock();
            cache_hits[i] = partition.cache_hits;
            cache_misses[i] = partition.cache_misses;
            sizes[i] = partition.tableid_map -> size();
            partition.mutex.unlock();
        }
    }
}