#include "db/cache_index.h"
#include "leveldb/slice.h"
#include <iostream>

using namespace std;

leveldb::CacheIndex *cache_index_;

void insertAndGetTest1(){
    int hash = 100;
    int memtable_id = 1;
    bool is_memtableid;
    for(int i = 0; i < 4; i ++){
        cache_index_ -> put(leveldb::Slice(), hash, memtable_id, false);
        hash += 100;
        memtable_id ++;
    }
    assert(cache_index_ -> get(leveldb::Slice(), 100, is_memtableid) == 1);
    assert(!is_memtableid);
    assert(cache_index_ -> get(leveldb::Slice(), 200, is_memtableid) == 2);
    assert(!is_memtableid);
    assert(cache_index_ -> get(leveldb::Slice(), 300, is_memtableid) == 3);
    assert(!is_memtableid);
    assert(cache_index_ -> get(leveldb::Slice(), 400, is_memtableid) == 4);
    assert(!is_memtableid);
}

void insertAndGetTest2(){
    int hash = 100;
    int memtable_id = 1;
    bool is_memtableid;
    for(int i = 0; i < 5; i ++){
        cache_index_ -> put(leveldb::Slice(), hash, memtable_id, false);
        hash += 100;
        memtable_id ++;
    }
    assert(cache_index_ -> get(leveldb::Slice(), 200, is_memtableid) == 2);
    assert(cache_index_ -> get(leveldb::Slice(), 300, is_memtableid) == 3);
    assert(cache_index_ -> get(leveldb::Slice(), 400, is_memtableid) == 4);
    assert(cache_index_ -> get(leveldb::Slice(), 500, is_memtableid) == 5);
    //key 100 is evicted and is not in cache anymore
    assert(cache_index_ -> get(leveldb::Slice(), 100, is_memtableid) == 0);
}

//4 partition with size 4
void insertAndGetTest3(){
    int hash = 100;
    int memtable_id = 1;
    bool is_memtableid;
    for(int i = 0; i < 16; i ++){
        cache_index_ -> put(leveldb::Slice(), hash, memtable_id, false);
        hash ++;
        memtable_id ++;
    }
    //partition 1
    assert(cache_index_ -> get(leveldb::Slice(), 100, is_memtableid) == 1);
    assert(cache_index_ -> get(leveldb::Slice(), 104, is_memtableid) == 5);
    assert(cache_index_ -> get(leveldb::Slice(), 108, is_memtableid) == 9);
    assert(cache_index_ -> get(leveldb::Slice(), 112, is_memtableid) == 13);
    //partition 2
    assert(cache_index_ -> get(leveldb::Slice(), 101, is_memtableid) == 2);
    assert(cache_index_ -> get(leveldb::Slice(), 105, is_memtableid) == 6);
    assert(cache_index_ -> get(leveldb::Slice(), 109, is_memtableid) == 10);
    assert(cache_index_ -> get(leveldb::Slice(), 113, is_memtableid) == 14);
    //partition 3
    assert(cache_index_ -> get(leveldb::Slice(), 102, is_memtableid) == 3);
    assert(cache_index_ -> get(leveldb::Slice(), 106, is_memtableid) == 7);
    assert(cache_index_ -> get(leveldb::Slice(), 110, is_memtableid) == 11);
    assert(cache_index_ -> get(leveldb::Slice(), 114, is_memtableid) == 15);
    //partition 4
    assert(cache_index_ -> get(leveldb::Slice(), 103, is_memtableid) == 4);
    assert(cache_index_ -> get(leveldb::Slice(), 107, is_memtableid) == 8);
    assert(cache_index_ -> get(leveldb::Slice(), 111, is_memtableid) == 12);
    assert(cache_index_ -> get(leveldb::Slice(), 115, is_memtableid) == 16);
}

//4 partition with size 4 with eviction
void insertAndGetTest4(){
    int hash = 100;
    int memtable_id = 1;
    bool is_memtableid;
    for(int i = 0; i < 32; i ++){
        cache_index_ -> put(leveldb::Slice(), hash, memtable_id, false);
        hash ++;
        memtable_id ++;
    }
    //partition 1
    assert(cache_index_ -> get(leveldb::Slice(), 116, is_memtableid) == 17);
    assert(cache_index_ -> get(leveldb::Slice(), 120, is_memtableid) == 21);
    assert(cache_index_ -> get(leveldb::Slice(), 124, is_memtableid) == 25);
    assert(cache_index_ -> get(leveldb::Slice(), 128, is_memtableid) == 29);
    //partition 2
    assert(cache_index_ -> get(leveldb::Slice(), 117, is_memtableid) == 18);
    assert(cache_index_ -> get(leveldb::Slice(), 121, is_memtableid) == 22);
    assert(cache_index_ -> get(leveldb::Slice(), 125, is_memtableid) == 26);
    assert(cache_index_ -> get(leveldb::Slice(), 129, is_memtableid) == 30);
    //partition 3
    assert(cache_index_ -> get(leveldb::Slice(), 118, is_memtableid) == 19);
    assert(cache_index_ -> get(leveldb::Slice(), 122, is_memtableid) == 23);
    assert(cache_index_ -> get(leveldb::Slice(), 126, is_memtableid) == 27);
    assert(cache_index_ -> get(leveldb::Slice(), 130, is_memtableid) == 31);
    //partition 4
    assert(cache_index_ -> get(leveldb::Slice(), 119, is_memtableid) == 20);
    assert(cache_index_ -> get(leveldb::Slice(), 123, is_memtableid) == 24);
    assert(cache_index_ -> get(leveldb::Slice(), 127, is_memtableid) == 28);
    assert(cache_index_ -> get(leveldb::Slice(), 131, is_memtableid) == 32);

    //partition 1
    assert(cache_index_ -> get(leveldb::Slice(), 100, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 104, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 108, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 112, is_memtableid) == 0);
    //partition 2
    assert(cache_index_ -> get(leveldb::Slice(), 101, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 105, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 109, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 113, is_memtableid) == 0);
    //partition 3
    assert(cache_index_ -> get(leveldb::Slice(), 102, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 106, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 110, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 114, is_memtableid) == 0);
    //partition 4
    assert(cache_index_ -> get(leveldb::Slice(), 103, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 107, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 111, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 115, is_memtableid) == 0);
}

void removeTest(){
    int hash = 116;
    int memtable_id = 17;
    bool is_memtableid;
    for(int i = 0; i < 16; i ++){
        cache_index_ -> remove(leveldb::Slice(), hash, memtable_id++, false);
        hash ++;
    }
    //partition 1
    assert(cache_index_ -> get(leveldb::Slice(), 116, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 120, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 124, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 128, is_memtableid) == 0);
    //partition 2
    assert(cache_index_ -> get(leveldb::Slice(), 117, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 121, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 125, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 129, is_memtableid) == 0);
    //partition 3
    assert(cache_index_ -> get(leveldb::Slice(), 118, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 122, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 126, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 120, is_memtableid) == 0);
    //partition 4
    assert(cache_index_ -> get(leveldb::Slice(), 119, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 123, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 127, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 131, is_memtableid) == 0);
}

void LRUTest(){
    int hash = 100;
    int memtable_id = 1;
    bool is_memtableid;
    for(int i = 0; i < 4; i ++){
        cache_index_ -> put(leveldb::Slice(), hash, memtable_id, false);
        hash += 100;
        memtable_id ++;
    }
    assert(cache_index_ -> get(leveldb::Slice(), 400, is_memtableid) == 4);
    assert(cache_index_ -> get(leveldb::Slice(), 300, is_memtableid) == 3);
    assert(cache_index_ -> get(leveldb::Slice(), 200, is_memtableid) == 2);
    assert(cache_index_ -> get(leveldb::Slice(), 100, is_memtableid) == 1);
    cache_index_ -> put(leveldb::Slice(), 500, 5, false);

    //key 100 evicted
    assert(cache_index_ -> get(leveldb::Slice(), 500, is_memtableid) == 5);
    assert(cache_index_ -> get(leveldb::Slice(), 300, is_memtableid) == 3);
    assert(cache_index_ -> get(leveldb::Slice(), 200, is_memtableid) == 2);
    assert(cache_index_ -> get(leveldb::Slice(), 100, is_memtableid) == 1);
    assert(cache_index_ -> get(leveldb::Slice(), 400, is_memtableid) == 0);

    cache_index_ -> put(leveldb::Slice(), 700, 7, false);
    cache_index_ -> put(leveldb::Slice(), 600, 6, false);

    //key 500 & 300 evicted
    assert(cache_index_ -> get(leveldb::Slice(), 600, is_memtableid) == 6);
    assert(cache_index_ -> get(leveldb::Slice(), 700, is_memtableid) == 7);
    assert(cache_index_ -> get(leveldb::Slice(), 200, is_memtableid) == 2);
    assert(cache_index_ -> get(leveldb::Slice(), 100, is_memtableid) == 1);
    assert(cache_index_ -> get(leveldb::Slice(), 500, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 300, is_memtableid) == 0);

    //key 100 & 600 & 700 evicted
    cache_index_ -> get(leveldb::Slice(), 200, is_memtableid);
    cache_index_ -> put(leveldb::Slice(), 300, 3, false);
    cache_index_ -> put(leveldb::Slice(), 400, 4, false);
    cache_index_ -> put(leveldb::Slice(), 500, 5, false);

    assert(cache_index_ -> get(leveldb::Slice(), 700, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 600, is_memtableid) == 0);
    assert(cache_index_ -> get(leveldb::Slice(), 100, is_memtableid) == 0);

    assert(cache_index_ -> get(leveldb::Slice(), 500, is_memtableid) == 5);
    assert(cache_index_ -> get(leveldb::Slice(), 400, is_memtableid) == 4);
    assert(cache_index_ -> get(leveldb::Slice(), 300, is_memtableid) == 3);
    assert(cache_index_ -> get(leveldb::Slice(), 200, is_memtableid) == 2);

}

void sizeTest(){
    // delete cache_index_;
    // cache_index_ = new leveldb::CacheIndex(32, 10000);

    // int hash = 1;
    // for(int i = 0; i < 320000; i ++){
    //     cache_index_ -> put(leveldb::Slice(), hash, 1, false);
    //     hash ++;
    // }
    // std::cout << "size of cache index: " << sizeof(*cache_index_) << std::endl;
}

int main(int argc, char *argv[]) {
    // cache_index_ = new leveldb::CacheIndex(4, 4);

    // insertAndGetTest1();
    // insertAndGetTest2();
    // insertAndGetTest3();
    // insertAndGetTest4();
    // removeTest();
    // LRUTest();
    // sizeTest();
    // delete cache_index_;
    return 0;
}
