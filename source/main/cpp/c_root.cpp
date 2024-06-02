#include "charon/c_object.h"
#include "charon/c_root.h"
#include "cfile/c_file.h"
#include "cbase/c_allocator.h"

namespace ncore
{
    namespace charon
    {
        // #define TEST_RESOURCE_DATA

#ifdef TEST_RESOURCE_DATA

        void gTestData(const object_t* root, const strtable_t* stringTable, const strtable_t* typeTable)
        {
            // root->print(*stringTable, *typeTable);

            // Some tests
            const object_t* test = root->getHdObject(membername_t("test"));
            // test->print(*stringTable, *typeTable);

            bool        testBool     = test->getBool(membername_t("testbool"));
            int         testInt      = test->getInt32(membername_t("testint"));
            Fx16        testTestfx16 = test->getFx16(membername_t("testfx16"));
            Fx32        testTestfx32 = test->getFx32(membername_t("testfx32"));
            const char* testStr      = test->getString(membername_t("teststring"));

            assert(testBool == true);
            assert(testInt == 2992);
            assert(testTestfx16.binary().value() == 0x2000);
            assert(testTestfx32.binary().value() == 0x2000);
            assert(StringTools::stringCompare(testStr, "This is a test") == 0);

            bool testBool2     = test->getBoolArray(membername_t("testboolarray"))[1] != 0;
            int  testInt2      = test->getInt32Array(membername_t("testinHdarray"))[1];
            Fx16 testTestfx162 = (Fx16&)test->getFx16Array(membername_t("testfx16array"))[1];
            Fx32 testTestfx322 = (Fx32&)test->getFx32Array(membername_t("testfx32array"))[1];

            const int loc_id1        = test->getTLocStr(membername_t("testlocid"));
            const int loc_id_array_1 = test->getTLocStrArray(membername_t("testlocidarray"))[0];
            const int loc_id_array_2 = test->getTLocStrArray(membername_t("testlocidarray"))[1];

            assert(testBool2 == false);
            assert(testInt2 == 222222);
            assert(testTestfx162.binary().value() == 0x2000);
            assert(testTestfx322.binary().value() == 0x2000);

            HdArray<const object_t*> tesHdArray = test->getHdObjecHdArray(membername_t("tesHdarray"));

            const object_t* tesHdArrayEl0 = tesHdArray[0];
            const object_t* tesHdArrayEl1 = tesHdArray[1];
            const object_t* tesHdArrayEl2 = tesHdArray[2];

            const object_t* testInternal = test->getHdObject(membername_t("test"));
            testBool                     = testInternal->getBool(membername_t("testbool"));
            testInt                      = testInternal->getInt32(membername_t("testint"));
            testTestfx16                 = testInternal->getFx16(membername_t("testfx16"));
            testTestfx32                 = testInternal->getFx32(membername_t("testfx32"));
            testStr                      = testInternal->getString(membername_t("teststring"));

            ct_assert(sizeof(bool) == 1);

            testBool = testInternal->getBoolArray(membername_t("testbools"))[0] != 0;
            assert(testBool == true);
            testBool = testInternal->getBoolArray(membername_t("testbools"))[1] != 0;
            assert(testBool == false);
            testBool = testInternal->getBoolArray(membername_t("testbools"))[2] != 0;
            assert(testBool == false);
            testBool = testInternal->getBoolArray(membername_t("testbools"))[3] != 0;
            assert(testBool == true);

            testInt2 = testInternal->getInt32Array(membername_t("tesHdarray"))[0];
            assert(testInt2 == 88228833);

            assert(testInt == 123444);
            assert(testTestfx16.binary().value() == 0x2800);
            assert(testTestfx32.binary().value() == 0x2800);
            assert(StringTools::stringCompare(testStr, "This is a test2") == 0);

            // End
        }
#endif

        root_t::root_t(alloc_t* allocator)
            : m_Allocator(allocator)
            , m_typeTable(nullptr)
            , m_dataSize(0)
            , m_data(nullptr)
        {
        }

        void root_t::load(const char* dataFilename, const char* relocFilename)
        {
            unload();

            nfile::file_handle_t resourceDataFile     = nfile::file_open(dataFilename, nfile::FILE_MODE_READ);
            const s64            resourceDataFileSize = nfile::file_size(resourceDataFile);
            if (resourceDataFileSize > 0)
            {
                nfile::file_handle_t resourceAllocFile     = nfile::file_open(relocFilename, nfile::FILE_MODE_READ);
                const s64            resourceAllocFileSize = nfile::file_size(resourceAllocFile);
                if (resourceAllocFileSize > 0)
                {
                    // Resource data
                    const unsigned int resourceDataSizeExtra         = 1024;
                    const unsigned int resourceDataFileSize          = (unsigned int)(resourceDataFileSize);
                    const unsigned int resourceDataAllocatedDataSize = resourceDataFileSize + resourceDataSizeExtra;
                    u8*                data                          = (u8*)m_Allocator->allocate(resourceDataAllocatedDataSize);

                    // Reallocation table
                    const unsigned int resourceReallocDataSizeExtra         = 1024;
                    const unsigned int resourceReallocDataFileSize          = (unsigned int)(resourceAllocFileSize);
                    const unsigned int resourceReallocDataAllocatedDataSize = resourceReallocDataFileSize + resourceReallocDataSizeExtra;
                    u32*               reallocData                          = (u32*)m_Allocator->allocate(resourceReallocDataFileSize);

                    nfile::file_read(resourceDataFile, data, resourceDataFileSize);
                    nfile::file_close(resourceDataFile);

                    nfile::file_read(resourceAllocFile, (u8*)reallocData, resourceReallocDataFileSize);
                    nfile::file_close(resourceAllocFile);

                    // Reallocate the pointers in the data (including StringTable)
                    int count = reallocData[0];
                    for (int i = 0; i < count; i++)
                    {
                        int* p = (int*)((int_t)data + reallocData[i + 1]);
                        *p     = *p + (int_t)data;
                    }
                    m_Allocator->deallocate(reallocData);

                    m_data = (dataheader_t*)data;

#ifdef TEST_RESOURCE_DATA
                    gTestData(mData->mRoot, mData->mStringTable, mTypeTable);
#endif
                }
            }
        }

        void root_t::reload(const char* dataFilename, const char* relocFilename) {}

        void root_t::unload()
        {
            if (m_data != nullptr)
            {
                m_Allocator->deallocate(m_data);
                m_data = nullptr;
            }
        }
    }  // namespace ngd
}  // namespace ncore
