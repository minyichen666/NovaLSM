// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_DB_BUILDER_H_
#define STORAGE_LEVELDB_DB_BUILDER_H_

#include "leveldb/status.h"
#include "leveldb/env.h"
#include "leveldb/env_bg_thread.h"
#include "cache_index.h"
#include "common/nova_common.h"

namespace leveldb {

    struct Options;
    struct FileMetaData;

    class Env;

    class Iterator;

    class TableCache;

    class VersionEdit;

// Build a Table file from the contents of *iter.  The generated file
// will be named according to meta->number.  On success, the rest of
// *meta will be filled with metadata about the generated table.
// If no data is present in *iter, meta->file_size will be set to
// zero, and no Table file will be produced.
    Status
    BuildTable(const std::string &dbname, Env *env, const Options &options,
               TableCache *table_cache, Iterator *iter, FileMetaData *meta,
               EnvBGThread *bg_thread, bool prune_memtables, CacheIndex *cache_index, uint32_t memtable_id = 0);

    Status
    TestBuildTable(const std::string &dbname, Env *env, const Options &options,
                   TableCache *table_cache, Iterator *iter, FileMetaData *meta,
                   EnvBGThread *bg_thread);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_BUILDER_H_
