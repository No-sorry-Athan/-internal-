#pragma once
#include <new>


typedef __int16					int16;
typedef unsigned __int16		uint16;
typedef __int32					int32;
typedef unsigned __int32		uint32;
typedef __int64					int64;
typedef unsigned __int64		uint64;

//-----------------------------------------------------------------------------
// Stack-based allocation related helpers
//-----------------------------------------------------------------------------
#if defined( COMPILER_GCC )

#define stackalloc( _size )		alloca( ALIGN_VALUE( _size, 16 ) )

#ifdef PLATFORM_OSX
#define mallocsize( _p )	( malloc_size( _p ) )
#else
#define mallocsize( _p )	( malloc_usable_size( _p ) )
#endif

#else
// for when we don't care about how many bits we use
typedef unsigned int uint;
#define FORCEINLINE_TEMPLATE		__forceinline
#define stackalloc( _size )		_alloca( ALIGN_VALUE( _size, 16 ) )
#define mallocsize( _p )		( _msize( _p ) )

//Little endian things
#define LittleDWord( val )			( val )

//Floating point
#define LittleFloat( pOut, pIn )	( *pOut = *pIn )

//#define WordSwap  WordSwapAsm
#pragma warning(push)
#pragma warning (disable:4035) // no return value

template <typename T>
inline T WordSwapAsm(T w)
{
	__asm
	{
		mov ax, w
		xchg al, ah
	}
}

//#define DWordSwap DWordSwapAsm
//#define BigLong( val )				DWordSwap( val )
//#define BigShort( val )				WordSwap( val )
//template <typename T>
//inline T DWordSwapAsm(T dw)
//{
//	__asm
//	{
//		mov eax, dw
//		bswap eax
//	}
//}



//dbg
#ifdef _DEBUG
#define COMPILE_TIME_ASSERT( pred )	switch(0){case 0:case pred:;}
#define ASSERT_INVARIANT( pred )	static void UNIQUE_ID() { COMPILE_TIME_ASSERT( pred ) }
#else
#define COMPILE_TIME_ASSERT( pred )
#define ASSERT_INVARIANT( pred )
#endif
#endif

#define stackfree( _p ) 0