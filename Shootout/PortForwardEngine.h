// PortForwardEngine.h: interface for the CPortForwardEngine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PORTFORWARDENGINE_H__003E19B2_EC21_4097_8A62_D28641F61CC8__INCLUDED_)
#define AFX_PORTFORWARDENGINE_H__003E19B2_EC21_4097_8A62_D28641F61CC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Natupnp.h>
#include <UPnP.h>

#include <vector>

using namespace std;

class CPortForwardChangeCallbacks  
{
public:
	CPortForwardChangeCallbacks();
	virtual ~CPortForwardChangeCallbacks();
	
	virtual HRESULT OnNewNumberOfEntries( long lNewNumberOfEntries );
	virtual HRESULT OnNewExternalIPAddress( BSTR bstrNewExternalIPAddress );
	
};



class CPortForwardEngine  
{
public:
	
	// forward declarations
	
protected:
	interface IDerivedNATExternalIPAddressCallback;
	interface IDerivedNATNumberOfEntriesCallback;
	
public:
	class PortMappingContainer;
	class DeviceInformationContainer;
	
	
public:
	
	// public functions -- there are only a few
	
	CPortForwardEngine();
	virtual ~CPortForwardEngine();
	
	HRESULT ListenForUpnpChanges(CPortForwardChangeCallbacks *pCallbacks = NULL);  // NULL==default object; if you provide your own pointer to a CPortForwardChangeCallbacks-derived object it is deleted for you automatically
	HRESULT StopListeningForUpnpChanges( );  // Stops listenting for UPnP change events on the router and deletes any CPortForwardChangeCallbacks-derived objects
	
	BOOL GetDeviceInformationUsingThread( HWND hWnd );  // starts a thread that will get IGD (router) device information; the thread posts a UWM_PORT_FORWARD_ENGINE_THREAD_NOTIFICATION message to hWnd when it's done
	BOOL GetMappingsUsingThread( HWND hWnd );  // starts a thread that will get all mappings; the thread posts a UWM_PORT_FORWARD_ENGINE_THREAD_NOTIFICATION message to hWnd when it's done
	BOOL EditMappingUsingThread( PortMappingContainer& oldMapping, PortMappingContainer& newMapping, HWND hWnd );  // starts a thread that will edit one specific mapping; the thread posts a UWM_PORT_FORWARD_ENGINE_THREAD_NOTIFICATION message to hWnd when it's done
	BOOL AddMappingUsingThread( PortMappingContainer& newMapping, HWND hWnd );  // starts a thread that will add one new mapping; the thread posts a UWM_PORT_FORWARD_ENGINE_THREAD_NOTIFICATION message to hWnd when it's done
	BOOL DeleteMappingUsingThread( PortMappingContainer& oldMapping, HWND hWnd );  // starts a thread that will delete one specific mapping; the thread posts a UWM_PORT_FORWARD_ENGINE_THREAD_NOTIFICATION message to hWnd when it's done
	
	std::vector<PortMappingContainer> GetPortMappingVector() const;  // gets a copy of currently-known port mappings
	DeviceInformationContainer GetDeviceInformationContainer() const;  // gets a copy of currently-know device information
	
	BOOL IsAnyThreadRunning() const;  // returns TRUE if there is any thread currently running
	
	
protected:
	
	// protected functions used internally by PortMappingEngine
	
	void InitializeMembersToNull();
	void DeinitializeCom();
	HRESULT PopulateDeviceInfoContainer( IUPnPDevice* piDevice, DeviceInformationContainer& deviceInfo, HWND hWnd=NULL );
	HRESULT GetNextMapping( IEnumVARIANT* piEnumerator, PortMappingContainer& mappingContainer );
	HRESULT SetChangeEventCallbackPointer(CPortForwardChangeCallbacks *pCallbacks);
	
	static UINT ThreadForPortRetrieval( LPVOID pVoid );
	static UINT ThreadForDeviceInformationRetrieval( LPVOID pVoid );
	static UINT ThreadToEditMapping( LPVOID pVoid );
	static UINT ThreadToAddMapping( LPVOID pVoid );
	static UINT ThreadToDeleteMapping( LPVOID pVoid );
	
	
	
protected:
		
	// protected members
	
	IUPnPNAT*								m_piNAT;					
	IDerivedNATExternalIPAddressCallback*	m_piExternalIPAddressCallback;
	IDerivedNATNumberOfEntriesCallback*		m_piNumberOfEntriesCallback;
	
	INATEventManager*						m_piEventManager;
	CPortForwardChangeCallbacks*			m_pChangeCallbackFunctions;
	
	BOOL m_bListeningForUpnpChanges;
	
	CWinThread* m_pPortMappingThread;
	CWinThread* m_pDeviceInfoThread;
	CWinThread* m_pAddMappingThread;
	CWinThread* m_pEditMappingThread;
	CWinThread* m_pDeleteMappingThread;
	
	std::vector<PortMappingContainer> m_MappingContainer;
	
	CRITICAL_SECTION m_cs;
	
	
protected:
	
	// protected interfaces, which were forward-declared above, and which are used for event notifications from COM
	// most of the code is here in this .h file, except for the QueryInterface method which is in the .cpp file

	interface IDerivedNATExternalIPAddressCallback : public INATExternalIPAddressCallback
	{
		IDerivedNATExternalIPAddressCallback( CPortForwardChangeCallbacks* p ) : m_pointer( p ), m_dwRef( 0 ) { };
		
		HRESULT STDMETHODCALLTYPE NewExternalIPAddress( BSTR bstrNewExternalIPAddress )
		{
			ASSERT( m_pointer != NULL );			
			return	m_pointer->OnNewExternalIPAddress( bstrNewExternalIPAddress );
		}
		
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
		
		ULONG STDMETHODCALLTYPE AddRef()  {	return ++m_dwRef; }
		
		ULONG STDMETHODCALLTYPE Release()  
		{
			if ( --m_dwRef == 0 )
				delete this;
			
			return m_dwRef;
		}
		
		DWORD		m_dwRef;
		CPortForwardChangeCallbacks*	m_pointer;
	};
	
	interface IDerivedNATNumberOfEntriesCallback : public INATNumberOfEntriesCallback
	{
		IDerivedNATNumberOfEntriesCallback( CPortForwardChangeCallbacks* p ) : m_pointer( p ), m_dwRef( 0 ) { };
		
		HRESULT STDMETHODCALLTYPE NewNumberOfEntries( long lNewNumberOfEntries )
		{
			ASSERT( m_pointer != NULL );			
			return m_pointer->OnNewNumberOfEntries( lNewNumberOfEntries );
		}
		
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject); 
		
		ULONG STDMETHODCALLTYPE AddRef()  { return ++m_dwRef; }
		
		ULONG STDMETHODCALLTYPE Release()
		{
			if ( --m_dwRef == 0 )
				delete this;
			
			return m_dwRef;
		}
		
		DWORD		m_dwRef;
		CPortForwardChangeCallbacks*	m_pointer;
	};
	
	
public:
	
	// public nested classes, which were forward-declared above
	// these are public because they are needed by classes that call the 
	// GetPortMappingVector() and GetDeviceInformationContainer() methods
	
	class PortMappingContainer
	{
	public:
		PortMappingContainer() : ExternalIPAddress(_T("")), ExternalPort(_T("")), 
			InternalPort(_T("")), Protocol(_T("")), InternalClient(_T("")), 
			Enabled(_T("")), Description(_T(""))   { };
		CString ExternalIPAddress;
		CString ExternalPort;
		CString InternalPort;
		CString Protocol;
		CString InternalClient;	
		CString Enabled;	
		CString Description;
	};
		
	class DeviceInformationContainer
	{
	public:
		DeviceInformationContainer() : Children(_T("")), Description(_T("")),
			FriendlyName(_T("")), HasChildren(_T("")), IconURL(_T("")), IsRootDevice(_T("")), 
			ManufacturerName(_T("")), ManufacturerURL(_T("")), ModelName(_T("")), 
			ModelNumber(_T("")), ModelURL(_T("")), ParentDevice(_T("")), 
			PresentationURL(_T("")), RootDevice(_T("")), SerialNumber(_T("")), 
			Services(_T("")), Type(_T("")), UniqueDeviceName(_T("")), UPC(_T(""))   { };
		
		// see http://msdn.microsoft.com/library/en-us/upnp/upnp/iupnpdevice.asp
		
		CString Children;			// Child devices of the device. 
		CString Description;		// Human-readable form of the summary of a device's functionality. 
		CString FriendlyName;		// Device display name. 
		CString HasChildren;		// Indicates whether the device has any child devices. 
		CString IconURL;			// URL of icon
		CString IsRootDevice;		// Indicates whether the device is the top-most device in the device tree. 
		CString ManufacturerName;	// Human-readable form of the manufacturer name. 
		CString ManufacturerURL;	// URL for the manufacturer's Web site. 
		CString ModelName;			// Human-readable form of the model name. 
		CString ModelNumber;		// Human-readable form of the model number. 
		CString ModelURL;			// URL for a Web page that contains model-specific information. 
		CString ParentDevice;		// Parent of the device. 
		CString PresentationURL;	// Presentation URL for a Web page that can be used to control the device. 
		CString RootDevice;			// Top-most device in the device tree. 
		CString SerialNumber;		// Human-readable form of the serial number. 
		CString Services;			// List of services provided by the device. 
		CString Type;				// Uniform resource identifier (URI) for the device type. 
		CString UniqueDeviceName;	// Unique device name (UDN) of the device. 
		CString UPC;				// Human-readable form of the product code. 
	};
	
	
	
protected:
	
	// some more protected members, most of which could not be declared until after full-declaration
	// of the DeviceInformationContainer and PortMappingContainer nested classes
	
	DeviceInformationContainer m_DeviceInfo;
	
	// good-enough method for inter-thread transfer of information, since only
	// one thread of each type is ever permitted at one time
	
	PortMappingContainer m_scratchpadOldMapping;
	PortMappingContainer m_scratchpadNewMapping;
	PortMappingContainer m_scratchpadAddedMapping;
	PortMappingContainer m_scratchpadDeletedMapping;
	
	HWND m_hWndForPortMappingThread;
	HWND m_hWndForDeviceInfoThread;
	HWND m_hWndForAddMappingThread;
	HWND m_hWndForEditMappingThread;
	HWND m_hWndForDeleteMappingThread;
	
	
public:
	
	// a public enum which is used by classes that respond to the registered window message
	// UWM_PORT_FORWARD_ENGINE_THREAD_NOTIFICATION, so they can decode the wParam and lParam values
	
	enum THREAD_STATUS {
		EnumPortRetrieveInterval	= 0x0001,
		EnumPortRetrieveDone	= 0x0002,
		EnumDeviceInfoInterval	= 0x0011,
		EnumDeviceInfoDone		= 0x0012,
		EnumAddMappingInterval	= 0x0021,
		EnumAddMappingDone		= 0x0022,
		EnumEditMappingInterval	= 0x0041,
		EnumEditMappingDone		= 0x0042,
		EnumDeleteMappingInterval	= 0x0081,
		EnumDeleteMappingDone		= 0x0082
	};
			
};



#endif // !defined(AFX_PORTFORWARDENGINE_H__003E19B2_EC21_4097_8A62_D28641F61CC8__INCLUDED_)
