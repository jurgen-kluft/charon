#ifndef __CGAMEDATA_BIG_FILE_MANAGER_H__
#define __CGAMEDATA_BIG_FILE_MANAGER_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cfile/c_file.h"

namespace ncore
{
    namespace ngd
    {
        typedef s32 fileid_t;

        struct MFT;
        struct FDB;
        struct BigFile;

        class BigFileManager
        {
        public:
            bool open(const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename);
            void close();

            bool exists(fileid_t id) const;                          ///< Return True if file exists in Archive
            bool isEqual(fileid_t firstId, fileid_t secondId) const; ///< Return True if both fileIds reference the same physical file
            bool isCompressed(fileid_t id) const;

            const char* filename(fileid_t id) const; ///< Return Filename associated with @hash

            int size(fileid_t id) const;                                    ///< Return size of file
            int read(fileid_t id, void* destination);                       ///< Read whole file in destination
            int read(fileid_t id, int size, void* destination);             ///< Read part of file header in destination
            int read(fileid_t id, int offset, int size, void* destination); ///< Read part of file in destination

        protected:
            MFT*         mMFT;     ///< The .gdt file
            FDB*         mFDB;     ///< The .gdf file
            filehandle_t mBigfile; ///< The .gda file
        };
    } // namespace ngd
} // namespace ncore

#endif /// __HWFILE_BIG_FILE_MANAGER_H__