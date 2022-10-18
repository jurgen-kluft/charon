#include "cbase/c_base.h"
#include "cbase/c_allocator.h"
#include "cbase/c_console.h"

#include "cunittest/cunittest.h"
#include "cunittest/private/ut_ReportAssert.h"


UNITTEST_SUITE_LIST(cUnitTest);
UNITTEST_SUITE_DECLARE(cUnitTest, gamedata);

namespace ncore
{
	// Our own assert handler
	class UnitTestAssertHandler : public ncore::asserthandler_t
	{
	public:
		UnitTestAssertHandler()
		{
			NumberOfAsserts = 0;
		}

		virtual bool	handle_assert(u32& flags, const char* fileName, s32 lineNumber, const char* exprString, const char* messageString)
		{
			UnitTest::reportAssert(exprString, fileName, lineNumber);
			NumberOfAsserts++;
			return false;
		}


		ncore::s32		NumberOfAsserts;
	};

	class UnitTestAllocator : public UnitTest::Allocator
	{
		ncore::alloc_t*	mAllocator;
	public:
						UnitTestAllocator(ncore::alloc_t* allocator)	{ mAllocator = allocator; }
		virtual void*	Allocate(size_t size)								{ return mAllocator->allocate((u32)size, sizeof(void*)); }
		virtual size_t	Deallocate(void* ptr)								{ return mAllocator->deallocate(ptr); }
	};

	class TestAllocator : public alloc_t
	{
		alloc_t*		mAllocator;
	public:
							TestAllocator(alloc_t* allocator) : mAllocator(allocator) { }

		virtual void* v_allocate(u32 size, u32 alignment)
		{
			UnitTest::IncNumAllocations();
			return mAllocator->allocate(size, alignment);
		}

		virtual u32 	v_deallocate(void* mem)
		{
			UnitTest::DecNumAllocations();
			return mAllocator->deallocate(mem);
		}

		virtual void v_release()
		{
			mAllocator->release();
			mAllocator = NULL;
		}
	};
}

ncore::alloc_t* gTestAllocator = NULL;
ncore::UnitTestAssertHandler gAssertHandler;


bool gRunUnitTest(UnitTest::TestReporter& reporter)
{
	cbase::init();

#ifdef TARGET_DEBUG
    ncore::context_t::set_assert_handler(&gAssertHandler);
#endif

	ncore::alloc_t* systemAllocator = ncore::alloc_t::get_system();
	ncore::UnitTestAllocator unittestAllocator( systemAllocator );
	UnitTest::SetAllocator(&unittestAllocator);

	ncore::console->write("Configuration: ");
	ncore::console->writeLine(TARGET_FULL_DESCR_STR);

	ncore::TestAllocator testAllocator(systemAllocator);
	gTestAllocator = &testAllocator;

	int r = UNITTEST_SUITE_RUN(reporter, cUnitTest);
	if (UnitTest::GetNumAllocations()!=0)
	{
		reporter.reportFailure(__FILE__, __LINE__, "cunittest", "memory leaks detected!");
		r = -1;
	}

	gTestAllocator->release();

	UnitTest::SetAllocator(NULL);

	cbase::exit();
	return r==0;
}

