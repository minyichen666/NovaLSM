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

    void LinkedList::remove(MemManager *mem_manager, Node *node){
        if(node){
            remove_without_freeing(node);
            if(node -> prev){
                node -> prev = nullptr;
            }
            if(node -> next){
                node -> next = nullptr;
            }
            uint32_t scid = mem_manager->slabclassid(node -> hash, sizeof(Node));
            mem_manager->FreeItem(node -> hash, (char *)node, scid);
            // delete node;
        }
    }

    // LinkedList::~LinkedList(){
    //     while(head){
    //         remove(head);
    //     }
    // }

    CacheIndex::CacheIndex(uint32_t partition_num, uint32_t partition_size, MemManager *mem_manager){
        partition_num_ = partition_num;
        mem_manager_ = mem_manager;

        uint32_t scid = mem_manager_->slabclassid(partition_num_, sizeof(CachePartition) * partition_num_);
        char *buf = mem_manager_->ItemAlloc(partition_num_, scid);
        cache_index_ = new(buf) CachePartition[partition_num_];

        uint64_t recordcount = nova::NovaConfig::config -> recordcount;
        scid = mem_manager_->slabclassid(recordcount, sizeof(Node*) * recordcount);
        buf = mem_manager_->ItemAlloc(recordcount, scid);
        tableid_map = new(buf) Node*[recordcount];
        
        for(int i = 0; i < partition_num_; i ++){
            CachePartition &partition = cache_index_[i];
            partition.size = partition_size;
            scid = mem_manager_->slabclassid(i, sizeof(LinkedList));
            buf = mem_manager_->ItemAlloc(i, scid);
            // partition.lru_queue = reinterpret_cast<LinkedList*>(buf);
            partition.lru_queue = new(buf) LinkedList();
            partition.cache_hits.store(0);
            partition.cache_misses.store(0);
        }
        NOVA_LOG(rdmaio::INFO)
            << fmt::format("Create cache index with {} partition with size {}", partition_num, partition_size);
        NOVA_LOG(rdmaio::INFO)
            << fmt::format("size of cache index {}, size of Node {} ", sizeof(CacheIndex), sizeof(Node));
    }

    CacheIndex::~CacheIndex(){
        for(int i = 0; i < partition_num_; i ++){
            CachePartition &partition = cache_index_[i];
            while(partition.lru_queue->head){
                partition.lru_queue->remove(mem_manager_,partition.lru_queue->head);
            }
            uint32_t scid = mem_manager_->slabclassid(i, sizeof(LinkedList));
            mem_manager_->FreeItem(i, (char *)partition.lru_queue, scid);
            //delete partition.lru_queue;
        }

        uint64_t recordcount = nova::NovaConfig::config -> recordcount;
        uint64_t scid = mem_manager_->slabclassid(recordcount, sizeof(Node*) * recordcount);
        mem_manager_->FreeItem(recordcount, (char *)tableid_map, scid);
        delete[] tableid_map;

        scid = mem_manager_->slabclassid(partition_num_, sizeof(CachePartition) * partition_num_);
        mem_manager_->FreeItem(partition_num_, (char *)cache_index_, scid);
        delete[] cache_index_;
    }

    uint64_t CacheIndex::get(const Slice &key, uint64_t hash, bool &is_memtableid){
        CachePartition &partition = cache_index_[hash % partition_num_];
        partition.mutex.lock();
        Node *node = tableid_map[hash];
        if(node != nullptr){
            //if key is found，move lastest used hash to the front
            partition.lru_queue -> remove_without_freeing(node);
            partition.lru_queue -> push_front(node);
            //return value
            is_memtableid = node -> is_memtableid;
            uint64_t table_id = node -> table_id;
            partition.mutex.unlock();
            return table_id;
        }else{
            partition.mutex.unlock();
            return -1;
        }
    }

    void CacheIndex::put(const Slice &key, uint64_t hash, uint64_t memtableid, bool is_memtableid){
        CachePartition &partition = cache_index_[hash % partition_num_];
        partition.mutex.lock();
        Node *node = tableid_map[hash];
        //if key is not in the cache
        if(node == nullptr){
            //if cache is full, evict least recently used
            if(partition.lru_queue -> size_ == partition.size){
                uint64_t evict_hash = partition.lru_queue -> tail -> hash;
                partition.lru_queue -> remove(mem_manager_, partition.lru_queue -> tail);
                tableid_map[evict_hash] = nullptr;
                partition.cache_evictions ++;
                partition.lru_queue -> size_ --;
            }
        }else{
            partition.lru_queue -> remove(mem_manager_, node);
            tableid_map[hash] = nullptr;
            partition.lru_queue -> size_ --;
        }
        //put key value into cache
        uint32_t scid = mem_manager_->slabclassid(hash, sizeof(Node));
        char *buf = mem_manager_->ItemAlloc(hash, scid);
        if(buf != nullptr){
            // Node *newNode = reinterpret_cast<Node*>(buf);
            
            //Node *newNode = new(buf) Node();

            Node *newNode = (Node*) buf;
            *(newNode) = Node();

            newNode -> table_id = memtableid;
            newNode -> is_memtableid = is_memtableid;
            newNode -> hash = hash;
            partition.lru_queue -> push_front(newNode);
            tableid_map[hash] = newNode;
            partition.lru_queue -> size_ ++;
        }else{
            if(partition.lru_queue -> size_ != 0){
                uint64_t evict_hash = partition.lru_queue -> tail -> hash;
                partition.lru_queue -> remove(mem_manager_, partition.lru_queue -> tail);
                tableid_map[evict_hash] = nullptr;
                partition.cache_evictions ++;
                partition.lru_queue -> size_ --;
            }
        }
        partition.mutex.unlock();
    }

    bool CacheIndex::remove(const Slice &key, uint64_t hash, uint64_t table_id, bool is_memtableid){
        CachePartition &partition = cache_index_[hash % partition_num_];
        partition.mutex.lock();
        Node *node = tableid_map[hash];
        //if key is in the cache
        if(node != nullptr){
            bool is_memtableid_in_cache = node -> is_memtableid;
            uint64_t table_id_in_cache = node -> table_id;
            //avoid race condition that accidentally remove different value
            if(is_memtableid_in_cache == is_memtableid && table_id == table_id_in_cache){
                partition.lru_queue -> remove(mem_manager_, node);
                tableid_map[hash] = nullptr;
                partition.cache_evictions ++;
                partition.lru_queue -> size_ --;
                partition.mutex.unlock();
                return true;
            }
        }
        partition.mutex.unlock();
        return false;
    }

    void CacheIndex::cache_stats(int *cache_hits, int *cache_misses, int *sizes, int *evictions){ 
        for(int i = 0; i < partition_num_; i ++){
            CachePartition &partition = cache_index_[i];
            cache_hits[i] = partition.cache_hits.load();
            cache_misses[i] = partition.cache_misses.load();
            partition.mutex.lock();
            sizes[i] = partition.lru_queue -> size_;
            evictions[i] = partition.cache_evictions;
            partition.mutex.unlock();
        }
    }

    void CacheIndex::incre_hit(const Slice &key, uint64_t hash){
        CachePartition &partition = cache_index_[hash % partition_num_];
        partition.cache_hits ++;
        
    }

    void CacheIndex::incre_miss(const Slice &key, uint64_t hash){
        CachePartition &partition = cache_index_[hash % partition_num_];
        partition.cache_misses ++;
    }
}