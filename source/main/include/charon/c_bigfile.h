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

    namespace ngd
    {
        typedef s64 fileid_t;

        struct mft_t;
        struct fdb_t;
        struct hdb_t;

        struct hash_t
        {
            u32 mHash[5]; // SHA1 hash, 160 bits
        };

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
            alloc_t*             mAlloc;
            void*                mBasePtr;
            mft_t*               mMFT;      // The .gdt file
            fdb_t*               mFDB;      // The .gdf file
            nfile::file_handle_t mBigfile;  // The .gda file
            s32                  mIndex;    // The index of the bigfile
        };

        // So the final bigfile will become something like this:
        // NOTE: Localization data should be packed into a specific Bigfile, and the language object
        //       should have a FileId to each defined language. If the language is not available it
        //       can redirect to English.
        class bigfile2_t
        {
        public:
            s32     mIndex;             // The index of the bigfile
            u32     mNumberOfSections;  // How many sections (bigfiles) are in this bigfile
            mft_t** mArrayOfMFT;        // The tables with file offset and file size of a fileid_t
            fdb_t** mArrayOfFDB;        // In DEBUG mode if you want to know the filename of a fileid_t
            hdb_t** mArrayOfHDB;        // In DEBUG mode if you want to know the hash of a fileid_t
        };

    }  // namespace ngd
}  // namespace ncore

#endif
