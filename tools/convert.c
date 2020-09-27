//
//  main.c
//  convert
//
//  Created by mabingtao on 24/09/2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/// Decodes a vis-encoded syslog string to a UTF-8 representation.
///
/// Apple's syslog logs are encoded in 7-bit form. Input bytes are encoded as follows:
/// 1. 0x00 to 0x19: non-printing range. Some ignored, some encoded as <...>.
/// 2. 0x20 to 0x7f: as-is, with the exception of 0x5c (backslash).
/// 3. 0x5c (backslash): octal representation \134.
/// 4. 0x80 to 0x9f: \M^x (using control-character notation for range 0x00 to 0x40).
/// 5. 0xa0: octal representation \240.
/// 6. 0xa1 to 0xf7: \M-x (where x is the input byte stripped of its high-order bit).
/// 7. 0xf8 to 0xff: unused in 4-byte UTF-8.
///
/// See: [vis(3) manpage](https://www.freebsd.org/cgi/man.cgi?query=vis&sektion=3)
/// 
/// https://github.com/flutter/flutter/commit/6a42ed3f5502f590b16eb712edc8b28f1bad9fd7

// UTF-8 values for \, M, -, ^.
const int kBackslash = 0x5c;
const int kM = 0x4d;
const int kDash = 0x2d;
const int kCaret = 0x5e;

// Mask for the UTF-8 digit range.
const int kNum = 0x30;

bool isDigit(char byte)
{
	return (byte & 0xf0) == kNum;
}

int decodeOctal(int x, int y, int z)
{
	return (x & 0x3) << 6 | (y & 0x7) << 3 | z & 0x7;
}

void decodeSyslog(const char *src, size_t srcLen, char *out, size_t *outLen)
{
	size_t ol = 0;

	for (int i = 0; i < srcLen;)
	{
		if (src[i] != kBackslash || i > srcLen - 4)
		{
			// Unmapped byte: copy as-is.
			out[ol] = src[i++];
			ol++;
		}
		else
		{
			// Mapped byte: decode next 4 bytes.
			if (src[i + 1] == kM && src[i + 2] == kCaret)
			{
				// \M^x form: bytes in range 0x80 to 0x9f.
				out[ol] = (src[i + 3] & 0x7f) + 0x40;
				ol++;
			}
			else if (src[i + 1] == kM && src[i + 2] == kDash)
			{
				// \M-x form: bytes in range 0xa0 to 0xf7.
				out[ol] = (src[i + 3] | 0x80);
				ol++;
			}
			else if (isDigit(src[i + 1]) && isDigit(src[i + 2]))
			{
				//(src.getRange(i + 1, i + 3).every(isDigit)) {
				// \ddd form: octal representation (only used for \134 and \240).
				out[ol] = decodeOctal(src[i + 1], src[i + 2], src[i + 3]);
				ol++;
			}
			else
			{
				// Unknown form: copy as-is.
				memcpy(out[ol], src[i], 4);
				ol += 4;
			}
			i += 4;
		}
	}
	*outLen = ol;
}


// int main(int argc, const char *argv[])
// {
// 	char linep[] = {0x20, 0x74, 0x65, 0x73, 0x74, 0x20, 0x63, 0x6F, 0x64, 0x65, 0x0A};
// 	size_t lp = 11;
// 	char out[lp];
// 	size_t outsize = 0;							   
// 	decodeSyslog(linep, lp, out, &outsize);
// 	for (int l = 0; l < outsize; l++)
// 	{
// 		printf("%02X ", out[l]);
// 	}
// 	return 0;
// }
