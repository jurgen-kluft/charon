#ifndef __CGAMEDATA_BIG_FILE_MANAGER_H__
#define __CGAMEDATA_BIG_FILE_MANAGER_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cfile/c_file.h"

namespace ncore
{
    class alloc_t;

    namespace ngd
    {
        typedef u64 fileid_t;

        struct mft_t;
        struct fdb_t;

        class bigfile_t
        {
        public:
            bigfile_t(alloc_t* allocator);

            static bigfile_t* instance;

            bool open(const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename);
            void close();

            bool exists(fileid_t id) const;                          ///< Return True if file exists in Archive
            bool isEqual(fileid_t firstId, fileid_t secondId) const; ///< Return True if both fileIds reference the same physical file
            bool isCompressed(fileid_t id) const;

            const char* filename(fileid_t id) const; ///< Return Filename associated with file id

            s32 size(fileid_t id) const;                                          ///< Return size of file
            s32 read(fileid_t id, void* destination) const;                       ///< Read whole file in destination
            s32 read(fileid_t id, s32 size, void* destination) const;             ///< Read part of file header in destination
            s32 read(fileid_t id, s32 offset, s32 size, void* destination) const; ///< Read part of file in destination

        protected:
            alloc_t*      mAlloc;
            void*         mBasePtr;
            mft_t*        mMFT;     ///< The .gdt file
            fdb_t*        mFDB;     ///< The .gdf file
            file_handle_t mBigfile; ///< The .gda file
        };

        // So the final bigfile will become something like this:
        // NOTE: Localization data should be packed into a specific Bigfile, and the language object
        //       should have a FileId to each defined language. If the language is not available it
        //       can redirect to English.
        class bigfile2_t
        {
        public:
            u32     mNumberOfTotalFileIds;
            u32     mNumberOfMFT;
            mft_t** mArrayOfMFT;
            fdb_t** mArrayOfFDB; // In DEBUG mode if you want to know the filename of a fileid_t
        };

    } // namespace ngd
} // namespace ncore

#endif /// __HWFILE_BIG_FILE_MANAGER_H__