/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sun Nov 26 12:44:16 2000
 */
/* Compiler settings for C:\SimpleDelay\SimpleDelay.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __SimpleDelay_h__
#define __SimpleDelay_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ISimpleDelay_FWD_DEFINED__
#define __ISimpleDelay_FWD_DEFINED__
typedef interface ISimpleDelay ISimpleDelay;
#endif 	/* __ISimpleDelay_FWD_DEFINED__ */


#ifndef __Delay_FWD_DEFINED__
#define __Delay_FWD_DEFINED__

#ifdef __cplusplus
typedef class Delay Delay;
#else
typedef struct Delay Delay;
#endif /* __cplusplus */

#endif 	/* __Delay_FWD_DEFINED__ */


#ifndef __DelayProp_FWD_DEFINED__
#define __DelayProp_FWD_DEFINED__

#ifdef __cplusplus
typedef class DelayProp DelayProp;
#else
typedef struct DelayProp DelayProp;
#endif /* __cplusplus */

#endif 	/* __DelayProp_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ISimpleDelay_INTERFACE_DEFINED__
#define __ISimpleDelay_INTERFACE_DEFINED__

/* interface ISimpleDelay */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ISimpleDelay;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("616E54EC-065D-48D5-9217-DB1D206728CA")
    ISimpleDelay : public IUnknown
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Wet( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_Wet( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Delay( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_Delay( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_MinDelay( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_MaxDelay( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISimpleDelayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISimpleDelay __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISimpleDelay __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISimpleDelay __RPC_FAR * This);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Wet )( 
            ISimpleDelay __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Wet )( 
            ISimpleDelay __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Delay )( 
            ISimpleDelay __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Delay )( 
            ISimpleDelay __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MinDelay )( 
            ISimpleDelay __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MaxDelay )( 
            ISimpleDelay __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        END_INTERFACE
    } ISimpleDelayVtbl;

    interface ISimpleDelay
    {
        CONST_VTBL struct ISimpleDelayVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISimpleDelay_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISimpleDelay_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISimpleDelay_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISimpleDelay_get_Wet(This,pVal)	\
    (This)->lpVtbl -> get_Wet(This,pVal)

#define ISimpleDelay_put_Wet(This,newVal)	\
    (This)->lpVtbl -> put_Wet(This,newVal)

#define ISimpleDelay_get_Delay(This,pVal)	\
    (This)->lpVtbl -> get_Delay(This,pVal)

#define ISimpleDelay_put_Delay(This,newVal)	\
    (This)->lpVtbl -> put_Delay(This,newVal)

#define ISimpleDelay_get_MinDelay(This,pVal)	\
    (This)->lpVtbl -> get_MinDelay(This,pVal)

#define ISimpleDelay_get_MaxDelay(This,pVal)	\
    (This)->lpVtbl -> get_MaxDelay(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISimpleDelay_get_Wet_Proxy( 
    ISimpleDelay __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB ISimpleDelay_get_Wet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE ISimpleDelay_put_Wet_Proxy( 
    ISimpleDelay __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB ISimpleDelay_put_Wet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISimpleDelay_get_Delay_Proxy( 
    ISimpleDelay __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB ISimpleDelay_get_Delay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE ISimpleDelay_put_Delay_Proxy( 
    ISimpleDelay __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB ISimpleDelay_put_Delay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISimpleDelay_get_MinDelay_Proxy( 
    ISimpleDelay __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB ISimpleDelay_get_MinDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISimpleDelay_get_MaxDelay_Proxy( 
    ISimpleDelay __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB ISimpleDelay_get_MaxDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISimpleDelay_INTERFACE_DEFINED__ */



#ifndef __SIMPLEDELAYLib_LIBRARY_DEFINED__
#define __SIMPLEDELAYLib_LIBRARY_DEFINED__

/* library SIMPLEDELAYLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_SIMPLEDELAYLib;

EXTERN_C const CLSID CLSID_Delay;

#ifdef __cplusplus

class DECLSPEC_UUID("AD90F22E-C51D-4F1C-B440-C1F2A01D5F1F")
Delay;
#endif

EXTERN_C const CLSID CLSID_DelayProp;

#ifdef __cplusplus

class DECLSPEC_UUID("B6D53BB1-89F2-4BE2-843A-0171A20CEFB1")
DelayProp;
#endif
#endif /* __SIMPLEDELAYLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
