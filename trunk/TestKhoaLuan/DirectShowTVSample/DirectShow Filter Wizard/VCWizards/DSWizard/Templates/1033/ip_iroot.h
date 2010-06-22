#pragma once

// [!output FILTER_GUID_REG]
DEFINE_GUID(CLSID_[!output PROJECT_NAME], 
[!output FILTER_GUID]);

// [!output FILTER_PROPERTY_PAGE_GUID_REG]
DEFINE_GUID(CLSID_[!output PROJECT_NAME]PropertyPage, 
[!output FILTER_PROPERTY_PAGE_GUID]);

struct [!output PROJECT_NAME]Parameters {
	// TODO: insert your own transform parameters here
	int param1;
	int param2;
};

#ifdef __cplusplus
extern "C" {
#endif

// [!output FILTER_INTERFACE_GUID_REG]
DEFINE_GUID(IID_I[!output PROJECT_NAME], 
[!output FILTER_INTERFACE_GUID]);

DECLARE_INTERFACE_(I[!output PROJECT_NAME], IUnknown)
{
    STDMETHOD(get_[!output PROJECT_NAME]) (THIS_
                [!output PROJECT_NAME]Parameters *irp
             ) PURE;

    STDMETHOD(put_[!output PROJECT_NAME]) (THIS_
                [!output PROJECT_NAME]Parameters irp
             ) PURE;
};

#ifdef __cplusplus
}
#endif

