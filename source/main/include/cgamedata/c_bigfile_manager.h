#ifndef __CGAMEDATA_BIG_FILE_MANAGER_H__
#define __CGAMEDATA_BIG_FILE_MANAGER_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "cfile/c_file.h"

namespace ncore
{
    class alloc_t;

    namespace ngd
    {
        typedef u64 fileid_t;

        struct MFT;
        struct FDB;
        struct BigFile;

        class BigFileManager
        {
        public:
            BigFileManager(alloc_t* allocator);

            bool open(const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename);
            void close();

            bool exists(fileid_t id) const;                          ///< Return True if file exists in Archive
            bool isEqual(fileid_t firstId, fileid_t secondId) const; ///< Return True if both fileIds reference the same physical file
            bool isCompressed(fileid_t id) const;

            const char* filename(fileid_t id) const; ///< Return Filename associated with @hash

            s32 size(fileid_t id) const;                                          ///< Return size of file
            s32 read(fileid_t id, void* destination) const;                       ///< Read whole file in destination
            s32 read(fileid_t id, s32 size, void* destination) const;             ///< Read part of file header in destination
            s32 read(fileid_t id, s32 offset, s32 size, void* destination) const; ///< Read part of file in destination

        protected:
            alloc_t*      mAlloc;
            void*         mBasePtr;
            MFT*          mMFT;     ///< The .gdt file
            FDB*          mFDB;     ///< The .gdf file
            file_handle_t mBigfile; ///< The .gda file
        };

        // So the final bigfile will become something like this:
        class NewBigFileManager
        {
        public:
            u32   mNumberOfTotalFileIds;
            u32   mNumberOfMFT;
            MFT** mArrayOfMFT;
            FDB** mArrayOfFDB; // In DEBUG mode if you want to know the filename of a fileid_t
        };

    } // namespace ngd
} // namespace ncore

#endif /// __HWFILE_BIG_FILE_MANAGER_H__