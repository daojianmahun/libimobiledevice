//
//  main.c
//  convert
//
//  Created by mabingtao on 24/09/2020.
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// #c---
/*****************************************************************************
 * 将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码.
 *
 * 参数:
 *    unic     字符的Unicode编码值
 *    pOutput  指向输出的用于存储UTF8编码值的缓冲区的指针
 *    outsize  pOutput缓冲的大小
 *
 * 返回值:
 *    返回转换后的字符的UTF8编码所占的字节数, 如果出错则返回 0 .
 *
 * 注意:
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求;
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种;
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位)
 *     2. 请保证 pOutput 缓冲区有最少有 6 字节的空间大小!
 ****************************************************************************/
int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput, int outSize)
{
	assert(pOutput != NULL);
	assert(outSize >= 6);

	if (unic <= 0x0000007F)
	{
		// * U-00000000 - U-0000007F:  0xxxxxxx
		*pOutput = (unic & 0x7F);
		return 1;
	}
	else if (unic >= 0x00000080 && unic <= 0x000007FF)
	{
		// * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
		*(pOutput + 1) = (unic & 0x3F) | 0x80;
		*pOutput = ((unic >> 6) & 0x1F) | 0xC0;
		return 2;
	}
	else if (unic >= 0x00000800 && unic <= 0x0000FFFF)
	{
		// * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
		*(pOutput + 2) = (unic & 0x3F) | 0x80;
		*(pOutput + 1) = ((unic >> 6) & 0x3F) | 0x80;
		*pOutput = ((unic >> 12) & 0x0F) | 0xE0;
		return 3;
	}
	else if (unic >= 0x00010000 && unic <= 0x001FFFFF)
	{
		// * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		*(pOutput + 3) = (unic & 0x3F) | 0x80;
		*(pOutput + 2) = ((unic >> 6) & 0x3F) | 0x80;
		*(pOutput + 1) = ((unic >> 12) & 0x3F) | 0x80;
		*pOutput = ((unic >> 18) & 0x07) | 0xF0;
		return 4;
	}
	else if (unic >= 0x00200000 && unic <= 0x03FFFFFF)
	{
		// * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		*(pOutput + 4) = (unic & 0x3F) | 0x80;
		*(pOutput + 3) = ((unic >> 6) & 0x3F) | 0x80;
		*(pOutput + 2) = ((unic >> 12) & 0x3F) | 0x80;
		*(pOutput + 1) = ((unic >> 18) & 0x3F) | 0x80;
		*pOutput = ((unic >> 24) & 0x03) | 0xF8;
		return 5;
	}
	else if (unic >= 0x04000000 && unic <= 0x7FFFFFFF)
	{
		// * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		*(pOutput + 5) = (unic & 0x3F) | 0x80;
		*(pOutput + 4) = ((unic >> 6) & 0x3F) | 0x80;
		*(pOutput + 3) = ((unic >> 12) & 0x3F) | 0x80;
		*(pOutput + 2) = ((unic >> 18) & 0x3F) | 0x80;
		*(pOutput + 1) = ((unic >> 24) & 0x3F) | 0x80;
		*pOutput = ((unic >> 30) & 0x01) | 0xFC;
		return 6;
	}

	return 0;
}
// #c---end

void convertUnicodeBytes(const char *src, size_t srcLen, char *des, size_t *desLen)
{
	int j = 0;

	for (int i = 0; i < srcLen;)
	{
		char byte = src[i];

		if ((byte == 0x5c) && i + 8 < srcLen) //5c == '\'
		{
			char nb1 = src[i + 1];
			char nb2 = src[i + 2];
			char nb3 = src[i + 3];
			char nb4 = src[i + 4];

			if (nb1 == 0x31 && nb2 == 0x33 && nb3 == 0x34 && (nb4 == 0x55 || nb4 == 0x75))
			{
				//将Unicode字符串转为Uncode编码
				char tmp[5] = {src[i + 5], src[i + 6], src[i + 7], src[i + 8], '\0'};

				long unicode = strtol(tmp, NULL, 16);

				//将Unicode编码转为UTF-8编码
				unsigned char *utf8Bytes = (unsigned char *)malloc(sizeof(unsigned char) * 6);

				int outLen = enc_unicode_to_utf8_one(unicode, utf8Bytes, 6);

				for (int k = 0; k < outLen; k++)
				{
					des[j] = utf8Bytes[k];
					j++;
				}
				i = i + 9;
			}
			else
			{
				des[j] = byte;
				i++;
				j++;
			}
		}
		else
		{
			des[j] = byte;
			i++;
			j++;
		}
	}
	des[j] = '\0';
	*desLen = j;
}
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
			// out.add(bytes[i++]);
			out[ol] = src[i++];
			ol++;
		}
		else
		{
			// Mapped byte: decode next 4 bytes.
			if (src[i + 1] == kM && src[i + 2] == kCaret)
			{
				// \M^x form: bytes in range 0x80 to 0x9f.
				//   out.add((bytes[i + 3] & 0x7f) + 0x40);
				out[ol] = (src[i + 3] & 0x7f) + 0x40;
				ol++;
			}
			else if (src[i + 1] == kM && src[i + 2] == kDash)
			{
				// \M-x form: bytes in range 0xa0 to 0xf7.
				// out.add(bytes[i + 3] | 0x80);
				out[ol] = (src[i + 3] | 0x80);
				ol++;
			}
			else if (isDigit(src[i + 1]) && isDigit(src[i + 2]))
			{
				//(src.getRange(i + 1, i + 3).every(isDigit)) {
				// \ddd form: octal representation (only used for \134 and \240).
				//out.add(decodeOctal(bytes[i + 1], bytes[i + 2], bytes[i + 3]));
				out[ol] = decodeOctal(src[i + 1], src[i + 2], src[i + 3]);
				ol++;
			}
			else
			{
				// Unknown form: copy as-is.
				//   out.addAll(bytes.getRange(0, 4));
				// void    *memcpy(void *__dst, const void *__src, size_t __n);
				memcpy(out[ol], src[i], 4);
				ol += 4;
			}
			i += 4;
		}
	}
	*outLen = ol;
}
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

/*
String decodeSyslog(String line) {
  // UTF-8 values for \, M, -, ^.
  const int kBackslash = 0x5c;
  const int kM = 0x4d;
  const int kDash = 0x2d;
  const int kCaret = 0x5e;

  // Mask for the UTF-8 digit range.
  const int kNum = 0x30;

  // Returns true when `byte` is within the UTF-8 7-bit digit range (0x30 to 0x39).
  bool isDigit(int byte) => (byte & 0xf0) == kNum;

  // Converts a three-digit ASCII (UTF-8) representation of an octal number `xyz` to an integer.
  int decodeOctal(int x, int y, int z) => (x & 0x3) << 6 | (y & 0x7) << 3 | z & 0x7;

  try {
    final List<int> bytes = utf8.encode(line);
    final List<int> out = <int>[];
    for (int i = 0; i < bytes.length;) {
      if (bytes[i] != kBackslash || i > bytes.length - 4) {
        // Unmapped byte: copy as-is.
        out.add(bytes[i++]);
      } else {
        // Mapped byte: decode next 4 bytes.
        if (bytes[i + 1] == kM && bytes[i + 2] == kCaret) {
          // \M^x form: bytes in range 0x80 to 0x9f.
          out.add((bytes[i + 3] & 0x7f) + 0x40);
        } else if (bytes[i + 1] == kM && bytes[i + 2] == kDash) {
          // \M-x form: bytes in range 0xa0 to 0xf7.
          out.add(bytes[i + 3] | 0x80);
        } else if (bytes.getRange(i + 1, i + 3).every(isDigit)) {
          // \ddd form: octal representation (only used for \134 and \240).
          out.add(decodeOctal(bytes[i + 1], bytes[i + 2], bytes[i + 3]));
        } else {
          // Unknown form: copy as-is.
          out.addAll(bytes.getRange(0, 4));
        }
        i += 4;
      }
    }
    return utf8.decode(out);
  } on Exception {
    // Unable to decode line: return as-is.
    return line;
  }
}
*/

// int main(int argc, const char *argv[])
// {
// 	char linep[] = {0x20, 0x74, 0x65, 0x73, 0x74, 0x20, 0x63, 0x6F, 0x64, 0x65, 0x0A};
// 	size_t lp = 11;

// 	char out[lp];
// 	size_t outsize = 0;							   //                       memset(void *s,   int ch,   size_t n);
// 	void *res = memset(out, 0, sizeof(char) * lp); // void    *memset(void *__b, int __c, size_t __len);
// 	decodeSyslog(linep, lp, out, &outsize);
// 	for (int l = 0; l < outsize; l++)
// 	{
// 		printf("%02X ", out[l]);
// 	}
// 	return 0;
// }
