#include "cgamedata\c_object.h"

#if 0

#include "HwNumber\HwFx16.h"
#include "HwNumber\HwFx32.h"
#include "HwFile\HwFile.h"
#include "HwFile\HwFileSystem.h"
#include "HwHdObject\HwHdObject.h"
#include "HwHdObject\HwHdStringTable.h"

namespace ncore
{
#define TEST_RESOURCE_DATA
#ifdef TEST_RESOURCE_DATA

    void gTestData(const HdObject* root, const HdStringTable* stringTable, const HdStringTable* typeTable)
    {
        // root->print(*stringTable, *typeTable);

        // Some tests
        const HdObject* test = root->getHdObject(HdMemberName("test"));
        // test->print(*stringTable, *typeTable);

        bool        testBool     = test->getBool(HdMemberName("testbool"));
        int         testInt      = test->getInt32(HdMemberName("testint"));
        Fx16        testTestfx16 = test->getFx16(HdMemberName("testfx16"));
        Fx32        testTestfx32 = test->getFx32(HdMemberName("testfx32"));
        const char* testStr      = test->getString(HdMemberName("teststring"));

        assert(testBool == true);
        assert(testInt == 2992);
        assert(testTestfx16.binary().value() == 0x2000);
        assert(testTestfx32.binary().value() == 0x2000);
        assert(StringTools::stringCompare(testStr, "This is a test") == 0);

        bool testBool2     = test->getBoolArray(HdMemberName("testboolarray"))[1] != 0;
        int  testInt2      = test->getInt32Array(HdMemberName("testinHdarray"))[1];
        Fx16 testTestfx162 = (Fx16&)test->getFx16Array(HdMemberName("testfx16array"))[1];
        Fx32 testTestfx322 = (Fx32&)test->getFx32Array(HdMemberName("testfx32array"))[1];

        const int loc_id1        = test->getTLocStr(HdMemberName("testlocid"));
        const int loc_id_array_1 = test->getTLocStrArray(HdMemberName("testlocidarray"))[0];
        const int loc_id_array_2 = test->getTLocStrArray(HdMemberName("testlocidarray"))[1];

        assert(testBool2 == false);
        assert(testInt2 == 222222);
        assert(testTestfx162.binary().value() == 0x2000);
        assert(testTestfx322.binary().value() == 0x2000);

        HdArray<const HdObject*> tesHdArray = test->getHdObjecHdArray(HdMemberName("tesHdarray"));

        const HdObject* tesHdArrayEl0 = tesHdArray[0];
        const HdObject* tesHdArrayEl1 = tesHdArray[1];
        const HdObject* tesHdArrayEl2 = tesHdArray[2];

        const HdObject* testInternal = test->getHdObject(HdMemberName("test"));
        testBool                     = testInternal->getBool(HdMemberName("testbool"));
        testInt                      = testInternal->getInt32(HdMemberName("testint"));
        testTestfx16                 = testInternal->getFx16(HdMemberName("testfx16"));
        testTestfx32                 = testInternal->getFx32(HdMemberName("testfx32"));
        testStr                      = testInternal->getString(HdMemberName("teststring"));

        ct_assert(sizeof(bool) == 1);

        testBool = testInternal->getBoolArray(HdMemberName("testbools"))[0] != 0;
        assert(testBool == true);
        testBool = testInternal->getBoolArray(HdMemberName("testbools"))[1] != 0;
        assert(testBool == false);
        testBool = testInternal->getBoolArray(HdMemberName("testbools"))[2] != 0;
        assert(testBool == false);
        testBool = testInternal->getBoolArray(HdMemberName("testbools"))[3] != 0;
        assert(testBool == true);

        testInt2 = testInternal->getInt32Array(HdMemberName("tesHdarray"))[0];
        assert(testInt2 == 88228833);

        assert(testInt == 123444);
        assert(testTestfx16.binary().value() == 0x2800);
        assert(testTestfx32.binary().value() == 0x2800);
        assert(StringTools::stringCompare(testStr, "This is a test2") == 0);

        // End
    }

#endif

    HdRoot::HdRoot()
        : mAllocatedDataSize(0)
        , mTypeTable(NULL)
        , mData(NULL)
    {
    }

    void HdRoot::load(const char* dataFilename, const char* relocFilename)
    {
        unload();

        File* resourceDataFile = FileSystem::open(dataFilename, EFileMode::READ);
        if (resourceDataFile->size() > 0)
        {
            File* resourceAllocFile = FileSystem::open(relocFilename, EFileMode::READ);
            if (resourceAllocFile->size() > 0)
            {
                // Resource data
                const unsigned int resourceDataSizeExtra         = 1024;
                const unsigned int resourceDataFileSize          = (unsigned int)(resourceDataFile->size());
                const unsigned int resourceDataAllocatedDataSize = resourceDataFileSize + resourceDataSizeExtra;
                void*              data                          = (void*)new char[(unsigned int)resourceDataAllocatedDataSize];

                // Reallocation table
                const unsigned int resourceReallocDataSizeExtra         = 1024;
                const unsigned int resourceReallocDataFileSize          = (unsigned int)(resourceAllocFile->size());
                const unsigned int resourceReallocDataAllocatedDataSize = resourceReallocDataFileSize + resourceReallocDataSizeExtra;
                int*               reallocData                          = (int*)new char[(unsigned int)resourceReallocDataAllocatedDataSize];

                resourceDataFile->read(data, resourceDataFileSize);
                resourceDataFile->close();

                resourceAllocFile->read(reallocData, resourceReallocDataFileSize);
                resourceAllocFile->close();

                // Reallocate the pointers in the data (including StringTable)
                int count = reallocData[0];
                for (int i = 0; i < count; i++)
                {
                    int* p = (int*)((int)data + reallocData[i + 1]);
                    *p     = *p + (int)data;
                }
                delete[] reallocData;

                mAllocatedDataSize = resourceDataAllocatedDataSize;
                mData              = (DataHeader*)data;

#ifdef TEST_RESOURCE_DATA
                gTestData(mData->mRoot, mData->mStringTable, mTypeTable);
#endif
            }
            delete resourceAllocFile;
        }
        delete resourceDataFile;
    }

    void HdRoot::reload(const char* dataFilename, const char* relocFilename) {}

    void HdRoot::unload()
    {
        if (mData != NULL)
        {
            // gMemFree(mData);
            delete[]((char*)mData);

            mData              = NULL;
            mAllocatedDataSize = 0;
        }
    }

} // namespace ncore

#endif