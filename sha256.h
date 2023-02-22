#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <stdint.h>
#include <strsafe.h>

#define H0 0x6A09E667
#define H1 0xBB67AE85
#define H2 0x3C6EF372
#define H3 0xA54FF53A
#define H4 0x510E527F
#define H5 0x9B05688C
#define H6 0x1F83D9AB
#define H7 0x5BE0CD19

#define BUFFER_SIZE 4096 * 4096
#define PADDING_SIZE 512

#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))

struct SHA256DigestStruct {
	LPWSTR fileName;
	uint32_t hashValues[8];
};

typedef struct SHA256DigestStruct SHA256Digest;

SHA256Digest initializeDigest(LPWSTR pathToFile);
SHA256Digest calculateHash(LPWSTR pathToFile);
HANDLE openFile(LPWSTR pathToFile);
void padMessage(uint8_t*, uint64_t, uint32_t*);
void padWithOneBit(uint8_t*, uint32_t*);
void padWithZeroes(uint8_t*, uint32_t*, uint64_t);
void padWithInitialLength(uint8_t*, uint64_t, uint32_t*);
void performRounds(SHA256Digest*, uint8_t*, int32_t);

uint32_t K[] = {0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
				0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
				0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
				0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
				0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
				0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
				0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
				0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2};

SHA256Digest calculateHash(LPWSTR pathToFile)
{
	SHA256Digest digest = initializeDigest(pathToFile);
	HANDLE hFile = openFile(digest.fileName);
	uint8_t* readBuffer = (uint8_t*) malloc(BUFFER_SIZE);
	DWORD bytesRead = -1;
	while (bytesRead != 0)
	{
		ReadFile(hFile, readBuffer, BUFFER_SIZE - PADDING_SIZE - 1, &bytesRead, NULL);
		uint64_t initialMessageLength = bytesRead * 8;
		uint32_t bufferPointer = bytesRead;
		padMessage(readBuffer, initialMessageLength, &bufferPointer);
		for(int offset = 0; offset < bufferPointer; offset += 64)
		{
			performRounds(&digest, readBuffer, offset); 
		}
	}
	free(readBuffer);
	CloseHandle(hFile);
	return digest;
}

SHA256Digest initializeDigest(LPWSTR pathToFile)
{
	SHA256Digest digest;
	(digest.hashValues)[0] = H0;
	(digest.hashValues)[1] = H1;
	(digest.hashValues)[2] = H2;
	(digest.hashValues)[3] = H3;
	(digest.hashValues)[4] = H4;
	(digest.hashValues)[5] = H5;
	(digest.hashValues)[6] = H6;
	(digest.hashValues)[7] = H7;
	size_t maxLength = PATH_MAX;
	size_t valueLength;
	StringCchLengthW(pathToFile, maxLength, &valueLength);
	digest.fileName = (LPWSTR) malloc(sizeof(wchar_t) * maxLength + sizeof(wchar_t));
	StringCchCopyW(digest.fileName, valueLength + 1, pathToFile);
	return digest;
}

HANDLE openFile(LPWSTR pathToFile)
{
	HANDLE hFile = CreateFileW(pathToFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	return hFile; 
}

void padMessage(uint8_t* message, uint64_t initialMessageLength, uint32_t* bufferPointer)
{
	padWithOneBit(message, bufferPointer);
	padWithZeroes(message, bufferPointer, initialMessageLength);
	padWithInitialLength(message, initialMessageLength, bufferPointer);
}

void padWithOneBit(uint8_t* message, uint32_t* bufferPointer)
{
	message[(*bufferPointer)] = 0x80;
	(*bufferPointer) += 1;
}

void padWithZeroes(uint8_t* message, uint32_t* bufferPointer, uint64_t initialMessageLength)
{
	uint32_t k = 0;
	while((initialMessageLength + 8 + k * 8) % 512 != 448)
	{
		message[(*bufferPointer)++] = 0x00;
		k++;
	}
}

void padWithInitialLength(uint8_t* message, uint64_t initialMessageLength, uint32_t* bufferPointer)
{
	uint8_t messageLengthInBytes[8];
	for (int i = 0; i < 8; i++)
	{
		messageLengthInBytes[i] = initialMessageLength >> (7 - i) * 8 & 0xFF;
		message[(*bufferPointer)++] = messageLengthInBytes[i];
	}
}

void performRounds(SHA256Digest* digest, uint8_t* message, int32_t offset) 
{
	uint32_t W[64];
	for (int k = 0; k < 64; k++)
		W[k] = 0;
	int wPointer = 0;
	for (int k = offset; k < offset + 64; k += 4)
	{
		W[wPointer++] = ((uint32_t) (message[k]) << 24)| ((uint32_t) (message[k + 1]) << 16) | ((uint32_t) (message[k + 2]) << 8) | ((uint32_t) message[k + 3]);
	}
	for (; wPointer < 64; wPointer++)
	{
		uint32_t s0 = (ROTRIGHT(W[wPointer - 15], 7)) ^ (ROTRIGHT(W[wPointer - 15], 18)) ^ (W[wPointer - 15] >> 3);;
		uint32_t s1 = (ROTRIGHT(W[wPointer - 2], 17)) ^ (ROTRIGHT(W[wPointer - 2], 19)) ^ (W[wPointer - 2] >> 10);
		W[wPointer] = W[wPointer - 16] + s0 + W[wPointer - 7] + s1;
	} 
	uint32_t a = digest->hashValues[0];
	uint32_t b = digest->hashValues[1];
	uint32_t c = digest->hashValues[2];
	uint32_t d = digest->hashValues[3];
	uint32_t e = digest->hashValues[4];
	uint32_t f = digest->hashValues[5];
	uint32_t g = digest->hashValues[6];
	uint32_t h = digest->hashValues[7];
	for (int i = 0; i < 64; i++)
	{
		uint32_t t2 = ((ROTRIGHT(a, 2)) ^ (ROTRIGHT(a, 13)) ^ (ROTRIGHT(a, 22))) + ((a & b) ^ (a & c) ^ (b & c));
		uint32_t t1 = h + ((ROTRIGHT(e, 6)) ^ (ROTRIGHT(e, 11)) ^ (ROTRIGHT(e, 25))) + ((e & f) ^ ((~e) & g)) + K[i] + W[i];
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}
	digest->hashValues[0] = digest->hashValues[0] + a;
	digest->hashValues[1] = digest->hashValues[1] + b;
	digest->hashValues[2] = digest->hashValues[2] + c;
	digest->hashValues[3] = digest->hashValues[3] + d;
	digest->hashValues[4] = digest->hashValues[4] + e;
	digest->hashValues[5] = digest->hashValues[5] + f;
	digest->hashValues[6] = digest->hashValues[6] + g;
	digest->hashValues[7] = digest->hashValues[7] + h;	
}