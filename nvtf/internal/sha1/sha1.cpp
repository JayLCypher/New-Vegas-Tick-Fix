/* sha1.cpp

Copyright (c) 2005 Michael D. Leonhard

http://tamale.net/

This file is licensed under the terms described in the
accompanying LICENSE file.
*/

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "sha1.h"

// print out memory in hexadecimal
void SHA1::hexPrinter( unsigned char* c, int l ) {
	assert( c );
	assert( l > 0 );
	while( l > 0 ) {
		printf( " %02x", *c );
		l--;
		c++;
	}
}

// circular left bit rotation.  MSB wraps around to LSB
UINT32 SHA1::lrot(const UINT32 x, const int bits) { return (x<<bits) | (x>>(32 - bits)); };

// Save a 32-bit unsigned integer to memory, in big-endian order
void SHA1::storeBigEndianUint32( unsigned char* byte, const UINT32 num ) {
	assert( byte );
	byte[0] = static_cast<unsigned char>(num>>24);
	byte[1] = static_cast<unsigned char>(num>>16);
	byte[2] = static_cast<unsigned char>(num>>8);
	byte[3] = static_cast<unsigned char>(num);
}

// process ***********************************************************
void SHA1::process() {
	assert( unprocessedBytes == 64 );
	//printf( "process: " ); hexPrinter( bytes, 64 ); printf( "\n" );
	int t;
	UINT32 K, f, W[80] = { };

	// starting values
	UINT32 a = H0;
	UINT32 b = H1;
	UINT32 c = H2;
	UINT32 d = H3;
	UINT32 e = H4;

	// copy and expand the message block
	for( t = 0; t < 16; t++ ) W[t] = (bytes[t*4] << 24)
									+(bytes[t*4 + 1] << 16)
									+(bytes[t*4 + 2] << 8)
									+ bytes[t*4 + 3];
	for(; t < 80; t++ ) W[t] = lrot( W[t-3]^W[t-8]^W[t-14]^W[t-16], 1 );
	
	/* main loop */
	for( t = 0; t < 80; t++ )
	{
		if (t < 20) {
			K = 0x5a827999;
			f = (b & c) | ((b ^ 0xFFFFFFFF) & d); //TODO: try using ~
		}
		else if (t < 40) {
			K = 0x6ed9eba1;
			f = b ^ c ^ d;
		}
		else if (t < 60) {
			K = 0x8f1bbcdc;
			f = (b & c) | (b & d) | (c & d);
		}
		else {
			K = 0xca62c1d6;
			f = b ^ c ^ d;
		}
		const UINT32 temp = lrot(a,5) + f + e + W[t] + K;
		e = d;
		d = c;
		c = lrot(b,30);
		b = a;
		a = temp;
		//printf( "t=%d %08x %08x %08x %08x %08x\n",t,a,b,c,d,e );
	}

	/* add variables */
	H0 += a;
	H1 += b;
	H2 += c;
	H3 += d;
	H4 += e;
	//printf( "Current: %08x %08x %08x %08x %08x\n",H0,H1,H2,H3,H4 );
	
	unprocessedBytes = 0; /* all bytes have been processed */
}

// addBytes **********************************************************
void SHA1::addBytes( const char* data, unsigned int num ) {
	assert( data );
	assert( num > 0 );
	
	size += num; // add these bytes to the running total
	
	while( num > 0 ) { // repeat until all data is processed
		const int needed = 64 - unprocessedBytes; // number of bytes required to complete block
		assert( needed > 0 );
		
		const unsigned int toCopy = (num < needed) ? num : needed; // number of bytes to copy (use smaller of two)
		memcpy( bytes + unprocessedBytes, data, toCopy ); // Copy the bytes
		num -= toCopy; // Bytes have been copied 
		data += toCopy;
		unprocessedBytes += toCopy;

		if( unprocessedBytes == 64 ) process(); // there is a full block
	}
}

// digest ************************************************************
unsigned char* SHA1::getDigest()
{
	// save the message size
	const UINT32 totalBitsL = size << 3;
	const UINT32 totalBitsH = size >> 29;
	// add 0x80 to the message
	addBytes( "\x80", 1 );
	
	unsigned char footer[64] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	if (unprocessedBytes > 56) { addBytes(reinterpret_cast<char *>(footer), 64 - unprocessedBytes); } // block has no room for 8-byte filesize, so finish it
	assert( unprocessedBytes <= 56 );

	const int neededZeros = 56 - unprocessedBytes; // how many zeros do we need
	storeBigEndianUint32( footer + neededZeros    , totalBitsH ); // store file size (in bits) in big-endian format
	storeBigEndianUint32( footer + neededZeros + 4, totalBitsL );
	
	addBytes(reinterpret_cast<char *>(footer), neededZeros + 8); // finish the final block
	
	const auto digest = static_cast<unsigned char *>(malloc(20)); // allocate memory for the digest bytes
	storeBigEndianUint32( digest, H0 ); // copy the digest bytes
	storeBigEndianUint32( digest + 4, H1 );
	storeBigEndianUint32( digest + 8, H2 );
	storeBigEndianUint32( digest + 12, H3 );
	storeBigEndianUint32( digest + 16, H4 );

	return digest; // return the digest
}
