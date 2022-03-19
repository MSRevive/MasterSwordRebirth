#if !(defined(lint) || defined(_lint) || defined(RC_INVOKED))
#if (_MSC_VER >= 800) || defined(_PUSHPOP_SUPPORTED)
#pragma warning(disable : 4103)
#if !(defined(MIDL_PASS)) || defined(__midl)
#pragma pack(pop)
#else
#pragma pack()
#endif
#else
#pragma pack()
#endif
#endif