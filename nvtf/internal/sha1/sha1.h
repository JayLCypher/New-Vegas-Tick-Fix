#ifndef SHA1_HEADER
#define SHA1_HEADER

/* sha1.h

Copyright (c) 2005 Michael D. Leonhard

http://tamale.net/

This file is licensed under the terms described in the
accompanying LICENSE file.
*/

class SHA1 {
public:
	SHA1() = default;
	~SHA1() = default;
	void addBytes( const char* data, unsigned int num );
	unsigned char* getDigest();
	// utility methods
	static UINT32 lrot( UINT32 x, int bits );
	static void storeBigEndianUint32( unsigned char* byte, const UINT32 num );
	static void hexPrinter( unsigned char* c, int l );

private:
	// fields
	UINT32 H0, H1, H2, H3, H4;
	unsigned char bytes[64] = {};
	unsigned int unprocessedBytes;
	UINT32 size;
	void process();
};

#endif
