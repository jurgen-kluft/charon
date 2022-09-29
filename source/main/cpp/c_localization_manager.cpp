#include "cbase/c_target.h"
#include "cgamedata/c_localization_manager.h"



#if 0



using namespace ncore;

s32				LocalizationManager::sLanguageFileSize = 0;
s32*			LocalizationManager::sOffsets = nullptr;
u64*			LocalizationManager::sHashOffsets = nullptr;
char*			LocalizationManager::sLanguageData = nullptr;
s32				LocalizationManager::sNumStrings = 0;
const char*		LocalizationManager::sStrings = nullptr;
ELanguage		LocalizationManager::sCurrentLanguage = LANGUAGE_ENGLISH;

#define XPRINT(txt)

void LocalizationManager::sInit()
{
	XPRINT("Start LocalizationManager init...\n");
	if (sLanguageData == nullptr)
	{
		XPRINT("Start get file id...\n");
		SArray<FileId>	locFiles = SResources::sRoot()->getResource(MemberName("Localization"))->getFileIdList(MemberName("FileId"));
		XPRINT("The file id list is init success.\n");
		sLanguageFileSize = 0;
		
		const s32 n = locFiles.size();
		ncore::x_printf("file id list : %d\n", x_va_list(n));
		for (s32 i=0; i<n; i++)
		{
			FileId fileId = locFiles[i];
			const s32 fileSize = (fileId != FileId::INVALID_ID) ? ArcFileManager::size(fileId) : 0;
			if (fileSize > sLanguageFileSize)
				sLanguageFileSize = fileSize;
		}
		XPRINT("Language file size init success.\n");
		sLanguageFileSize = x_Align(sLanguageFileSize, 4);
		XPRINT("Language file size alloc success.\n");
		sNumStrings = 0;
		sOffsets    = nullptr;
		sStrings    = nullptr;
		sCurrentLanguage = LANGUAGE_ENGLISH;
		
		sLanguageData = (char*)x_malloc(sizeof(char), sLanguageFileSize, ncore::XMEM_FLAG_ALIGN_32B);//new char[sLanguageFileSize];
		XPRINT("Language data alloc success.\n");
	}
	XPRINT("Start setting currrent language...\n");
	// set the current language and load the data
#ifdef TARGET_FINAL_A
	sSetCurrentLanguageForVersionA(ncore::xsystem::GetLanguage());
#elif defined TARGET_FINAL_E
	sSetCurrentLanguageForVersionE(ncore::xsystem::GetLanguage());
#else
	sSetCurrentLanguageForVersionE(ncore::xsystem::GetLanguage());
#endif
	sLoad();
	XPRINT("End LocalizationManager init...\n");
}


ELanguage LocalizationManager::sGetCurrentLanguage()
{
	return sCurrentLanguage;
}
void LocalizationManager::sLoad()
{
	SArray<FileId>	locFiles = SResources::sRoot()->getResource(MemberName("Localization"))->getFileIdList(MemberName("FileId"));
	FileId fileId = locFiles[(s32)sCurrentLanguage];
	const s32 fileSize = (fileId != FileId::INVALID_ID) ? ArcFileManager::size(fileId) : 0;
	const char* fileName	=	ArcFileManager::filename(fileId);

	MEMORY_STATISTIC(ResourceManager::RESOURCE_LOCALIZATION, fileName, fileSize);
	
	xbool is_ok = fileSize > 0;
	if (is_ok)
	{
		is_ok = ArcFileManager::read(fileId, (void*)sLanguageData) /*== fileSize*/;

		if (is_ok)
		{
			//-------------- Localization file format------------------
			// Magic number(identifier), type : s64
			// Count of strings , type : s32
			// unused 4 Byte
			// hashArray[count] , type : s64
			// offsetArray[count], type : s32
			// String : .......
			//-----------------------------------------------------------
			// First 2 integers are the identifier
			// sLanguageData : start of the file
			sNumStrings = *((s32*)(sLanguageData + 2*4));// Count
			sHashOffsets = (u64*)(sLanguageData + 3*4 + 4);
			sOffsets    = (s32*)(sLanguageData + 3*4 + sNumStrings /*use as count*/ * 8 + 4);
			sStrings    = (const char*)(sOffsets + sNumStrings /*use as count*/ * 1);
		}
		else
		{
			sNumStrings = 0;
			sOffsets    = nullptr;
			sStrings    = nullptr;
		}
	}

	if (!is_ok)
	{
		Tracer::Trace("Could not load localization data for language");
	}
}


void		LocalizationManager::sExit()
{
	sLanguageFileSize = 0;
	delete[] sLanguageData;
	sLanguageData = nullptr;
	
	sNumStrings = 0;
	sOffsets    = nullptr;
	sStrings    = nullptr;
	sCurrentLanguage = LANGUAGE_ENGLISH;
}


const char*  LocalizationManager::sGetText(s32 id)
{
	if ( id < 0 || id >= sNumStrings )
	{
		return "WRONG LOC ID";
	}
	return sStrings + sOffsets[id];
}

const ncore::s32 LocalizationManager::sHashToId(lstring_t localizationHash)
{
 	// Binary search
   	ncore::s32 current = 0;
   	ncore::s32 start = 0;
   	ncore::s32 end  = sNumStrings - 1;
	while (end >= start)
	{
	current = (start + end)/2;
	if (sHashOffsets[current] == localizationHash.getId())
		return current;
	else if (sHashOffsets[current] < localizationHash.getId())
		start = current + 1;
	else
		end = current - 1;
	}
	return -1;
}

const ncore::u64 LocalizationManager::sGetStringHash64( const char* str )
{
	const ncore::u64 m = 0xc6a4a7935bd1e995LL;
	const ncore::s32 r = 47;
	ncore::u32 length = ncore::strlen( str );

	ncore::u64 h = length * m;

	const ncore::u8* data = (const ncore::u8*)str;

	while( length >= 8 )
	{
		ncore::u64 k;		

		k  = ncore::u64( data[0] );
		k |= ncore::u64( data[1] ) << 8;
		k |= ncore::u64( data[2] ) << 16;
		k |= ncore::u64( data[3] ) << 24;
		k |= ncore::u64( data[4] ) << 32;
		k |= ncore::u64( data[5] ) << 40;
		k |= ncore::u64( data[6] ) << 48;
		k |= ncore::u64( data[7] ) << 56;

		k *= m; 
		k ^= k >> r; 
		k *= m; 

		h ^= k;
		h *= m;

		data += 8;
		length-= 8;
	}


	switch( length )
	{
	case 7: h ^= ncore::u64(data[6]) << 48;
	case 6: h ^= ncore::u64(data[5]) << 40;
	case 5: h ^= ncore::u64(data[4]) << 32;
	case 4: h ^= ncore::u64(data[3]) << 24;
	case 3: h ^= ncore::u64(data[2]) << 16;
	case 2: h ^= ncore::u64(data[1]) << 8;
	case 1: h ^= ncore::u64(data[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

const char* LocalizationManager::sGetTextByLocId( const char* locId )
{
	ncore::u64 locIdHash;
	ncore::s32 id;
	const char* result;
	locIdHash = sGetStringHash64(locId);
	id = sHashToId(locIdHash);
	result = sGetText(id);
	return result;
}

const char* LocalizationManager::sGetTextByLString(lstring_t lstr)
{
	return sGetText( sHashToId(lstr) );
}


#endif