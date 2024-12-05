#ifndef __CHARON_BIG_FILE_MANAGER_H__
#define __CHARON_BIG_FILE_MANAGER_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cfile/c_file.h"
#include "charon/c_object.h"

namespace ncore
{
    class alloc_t;

    namespace charon
    {
        typedef s64 fileid_t;

        struct gda_t;
        struct toc_t;
        struct mft_t;
        struct fdb_t;
        struct hdb_t;

        class bigfile_t
        {
        public:
            bigfile_t(alloc_t* allocator);

            bool open(const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename);
            void close();

            s32      index() const { return mIndex; }
            bool     exists(fileid_t id) const;                           // Return True if file exists in Archive
            bool     isEqual(fileid_t firstId, fileid_t secondId) const;  // Return True if both fileIds reference the same physical file
            bool     isCompressed(fileid_t id) const;                     //
            string_t filename(fileid_t id) const;                         // Return Filename associated with file id

            s64 size(fileid_t id) const;                                           // Return size of a file
            s64 read(fileid_t id, void* destination) const;                        // Read whole file in destination
            s64 read(fileid_t id, s32 size, void* destination) const;              // Read part of file header in destination
            s64 read(fileid_t id, s32 offset, s32 size, void* destination) const;  // Read part of file in destination

        protected:
            alloc_t* mAlloc;
            void*    mBasePtr;  // The TOC of the bigfile in memory
            s32      mIndex;    // Index of the bigfile in the bigfile manager
            gda_t*   mGDA;      // The .gda file
            toc_t*   mMFT;      // The TOC of the bigfile
            fdb_t*   mFDB;      // In DEBUG mode if you want to know the filename of a fileid_t
            hdb_t*   mHDB;      // In DEBUG mode if you want to know the hash of a fileid_t
        };

        class bigfile_manager_t
        {
        public:
        private:
            s32         mNumBigfiles;
            bigfile_t** mBigfiles;
        };

    }  // namespace charon
}  // namespace ncore

#endif
