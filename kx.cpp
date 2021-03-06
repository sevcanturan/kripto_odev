// kx.cpp : Bu dosya 'main' işlevi içeriyor. Program yürütme orada başlayıp biter.
//

#define _WIN32_DCOM
#include "pch.h"
#include <iostream>
#include <fstream>
#include <comdef.h>
#include <Wbemidl.h>

#include "aes256.hpp"
using namespace std;
#pragma comment(lib, "wbemuuid.lib")


string bul2() {
	HRESULT hres;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		cout << "Failed to initialize COM library. Error code = 0x" << hex << hres << endl;
		return "1";                  // Program has failed.
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
	);


	if (FAILED(hres))
	{
		cout << "Failed to initialize security. Error code = 0x"
			<< hex << hres << endl;
		CoUninitialize();
		return "1";                    // Program has failed.
	}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres))
	{
		cout << "Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< hex << hres << endl;
		CoUninitialize();
		return "1";                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
	);

	if (FAILED(hres))
	{
		cout << "Could not connect. Error code = 0x"
			<< hex << hres << endl;
		pLoc->Release();
		CoUninitialize();
		return "1";                // Program has failed.
	}

	cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);

	if (FAILED(hres))
	{
		cout << "Could not set proxy blanket. Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "1";               // Program has failed.
	}

	IEnumWbemClassObject* pEnumerator = NULL;


	IWbemClassObject *pclsObj;
	ULONG uReturn = 0;

	//Win32_DiskDrive

	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_DiskDrive"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		std::cout << "Query for operating system name failed."
			<< " Error code = 0x"
			<< std::hex << hres << std::endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "1"; // Program has failed.
	}


	uReturn = 0;
	string returnValue = "";
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;

		std::cout << "----------------------------------" << std::endl;
		std::cout << "Retrieve DISK Info" << std::endl;
		std::cout << "----------------------------------" << std::endl;
		// Get the value of the Name property
		hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
		std::wcout << " Disk Name : " << vtProp.bstrVal << std::endl;
		VariantClear(&vtProp);

		hr = pclsObj->Get(L"Model", 0, &vtProp, 0, 0);
		std::wcout << " Disk Model : " << vtProp.bstrVal << std::endl;
		VariantClear(&vtProp);

		hr = pclsObj->Get(L"Status", 0, &vtProp, 0, 0);
		std::wcout << " Status : " << vtProp.bstrVal << std::endl;
		VariantClear(&vtProp);

		hr = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
		std::wcout << " Device ID : " << vtProp.bstrVal << std::endl;
		VariantClear(&vtProp);


		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		std::wcout << " SerialNumber : " << vtProp.bstrVal << std::endl;
		returnValue += _bstr_t(vtProp.bstrVal);
		VariantClear(&vtProp);
		break;
	}
	//Win32_baseboard
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_baseboard"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		std::cout << "Query for operating system name failed."
			<< " Error code = 0x"
			<< std::hex << hres << std::endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "1"; // Program has failed.
	}


	uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;

		std::cout << "----------------------------------" << std::endl;
		std::cout << "Retrieve DISK Info" << std::endl;
		std::cout << "----------------------------------" << std::endl;
		// Get the value of the Name property
		hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
		std::wcout << " Disk Name : " << vtProp.bstrVal << std::endl;
		VariantClear(&vtProp);


		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		std::wcout << " SerialNumber : " << vtProp.bstrVal << std::endl;
		returnValue += _bstr_t(vtProp.bstrVal);
		VariantClear(&vtProp);

	}
	return returnValue;
}


#define KEY_LEN    32
#define TEXT_SIZE 100
#define ENC_SIZE  113

unsigned char test_key[KEY_LEN] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
									  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };



byte sifreli[17] = { 0,173,152,20,51,154,174,168,33,9,199,195,37,56,56,126,211 };
// Test vectors
#define TEST_VECTOR_NUM   4
#define TEST_VECTOR_SIZE 16

void init_key(ByteArray& key) {
	for (unsigned char i = 0; i < KEY_LEN; i++)
		key.push_back(test_key[i]);

}


void init_txt(ByteArray& txt, string plain) {
	for (unsigned char i = 0; i < sizeof(txt); ++i)
		txt.push_back(plain.at(i));
}


int main()
{
	string original = "";
	original = original + bul2();

	std::cout << "Starting test encrypts char..." << std::endl;

	ByteArray key;
	init_key(key);
	std::cout << " Key initialized..." << std::endl;

	//unsigned char txt[TEXT_SIZE];
	ByteArray enc;
	init_txt(enc, original);
	std::cout << " Text initialized..." << std::endl;
	ByteArray sonuc;
	ByteArray::size_type enc_len = Aes256::encrypt(key, enc, sonuc);
	std::cout << " Encrypted..." << std::endl;

	bool ayni = true;
	for (int i = 0; i < sizeof(sifreli); i++) {
		if (sifreli[i] != (byte)sonuc[i]) {
			ayni = false;
			break;
		}
	}
	if (!ayni) {
		cout << "hata";
		return -1;
	}
	std::cout << " Done!" << std::endl;
	int a;
	int i;
	int	toplam = 1;
	cout << "****FAKTORIYEL PROGRAMI****" << endl;
	cout << endl;
baslat:
	cout << "Faktoriyeli Alinacak Sayiyi Giriniz: ";
	cin >> a;
	cout << endl;
	if (a >= 0)
		for (i = 1; i <= a; i++)
		{
			toplam = toplam * i;
		}
	else
	{
		cout << "Negatif sayi girdiniz. Lutfen pozitif sayi giriniz." << endl;
		cout << endl;
		cout << endl;
		goto baslat;
	}
	cout << endl;
	cout << a << "!=" << toplam << endl;
	cout << endl;
	cin >> a;

}
