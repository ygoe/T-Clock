// -------------------------------------------------------------------------
/* Modified by Mike Caetano 1-7-2002

	added GUID_NULL definition
	corrected IsEqualGUID
// ---------------------------------------------------------------------------
	Potential Conflicts
	c:\lcc\include\objbase.h contains a different macro definition
		for InlineIsEqualGUID macro, the psdk version of objbase.h does not

// ------------------------------------------------------------------------
*/

// Scope Filter Found Below
// ---------------------------------------------------------------------------
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID {
unsigned long Data1;
unsigned short Data2;
unsigned short Data3;
unsigned char Data4[ 8 ];
} GUID;
#endif

#define DECLSPEC_SELECTANY
#define EXTERN_C extern

// ---------------------------------------------------------------------------
// from ksguid.h

#ifndef STATICGUIDOF
#define STATICGUIDOF(guid) STATIC_##guid
#endif

#ifdef DEFINE_GUIDEX
#undef DEFINE_GUIDEX
#endif
#define DEFINE_GUIDEX(name) \
	EXTERN_C const CDECL GUID DECLSPEC_SELECTANY name = { STATICGUIDOF(name) }

// ---------------------------------------------------------------------------
// from ks.h

#define DEFINE_GUIDSTRUCT(g, n) DEFINE_GUIDEX(n)
#define DEFINE_GUIDNAMED(n) n

#define GUID_NULL DEFINE_GUIDNAMED(GUID_NULL)

#define STATIC_GUID_NULL \
	0x00000000L, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

// notable means for defining a variable
DEFINE_GUIDEX(GUID_NULL);

// ---------------------------------------------------------------------------

#ifdef DEFINE_GUID
#undef DEFINE_GUID
#endif

#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
			EXTERN_C const GUID DECLSPEC_SELECTANY name \
				={ l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
			EXTERN_C const GUID name
#endif

#undef DEFINE_OLEGUID
#define DEFINE_OLEGUID(name, l, w1, w2) \
	DEFINE_GUID(name, l, w1, w2, 192,0,0,0,0,0,0,70)

// ---------------------------------------------------------------------------
// Scope Filter
// ---------------------------------------------------------------------------
#ifndef __GUIDDEF_H__
#define __GUIDDEF_H__

// ---------------------------------------------------------------------------

#ifndef __LPGUID_DEFINED__
#define __LPGUID_DEFINED__
typedef GUID *LPGUID;
#endif

#ifndef __LPCGUID_DEFINED__
#define __LPCGUID_DEFINED__
typedef const GUID *LPCGUID;
#endif

// ---------------------------------------------------------------------------

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef GUID IID;
typedef IID *LPIID;
#define IID_NULL GUID_NULL
#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)

typedef GUID CLSID;
typedef CLSID *LPCLSID;
#define CLSID_NULL GUID_NULL
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)

typedef GUID FMTID;
typedef FMTID *LPFMTID;
#define FMTID_NULL GUID_NULL
#define IsEqualFMTID(rfmtid1, rfmtid2) IsEqualGUID(rfmtid1, rfmtid2)

// ---------------------------------------------------------------------------

#ifdef __midl_proxy
#define __MIDL_CONST
#else
#define __MIDL_CONST const
#endif

#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID const GUID * __MIDL_CONST
#endif

#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#define REFIID const IID * __MIDL_CONST
#endif

#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#define REFCLSID const IID * __MIDL_CONST
#endif

#ifndef _REFFMTID_DEFINED
#define _REFFMTID_DEFINED
#define REFFMTID const IID * __MIDL_CONST
#endif

#endif // __IID_DEFINED__

// ---------------------------------------------------------------------------

#ifndef __midl
#ifndef _SYS_GUID_OPERATORS_
#define _SYS_GUID_OPERATORS_

#define InlineIsEqualGUID(rguid1, rguid2)  \
        ( ((unsigned long *) rguid1)[0] == ((unsigned long *) rguid2)[0] &&   \
          ((unsigned long *) rguid1)[1] == ((unsigned long *) rguid2)[1] &&   \
          ((unsigned long *) rguid1)[2] == ((unsigned long *) rguid2)[2] &&   \
          ((unsigned long *) rguid1)[3] == ((unsigned long *) rguid2)[3] )

#define IsEqualGUID(rguid1, rguid2) \
	( !memcmp( &(rguid1), &(rguid2), sizeof(GUID)) )

#ifdef __INLINE_ISEQUAL_GUID
#undef IsEqualGUID
#define IsEqualGUID(rguid1, rguid2) InlineIsEqualGUID(rguid1, rguid2)
#endif

#endif // _SYS_GUID_OPERATORS_
#endif // __midl

// ---------------------------------------------------------------------------
#endif //  __GUIDDEF_H__

