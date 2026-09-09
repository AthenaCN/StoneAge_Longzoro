// Arminius' protocol utilities ver 0.1
//
// Any questions and bugs, mailto: arminius@mail.hwaei.com.tw

// -------------------------------------------------------------------
// The following definitions is to define game-dependent codes.
// Before compiling, remove the "//".
#define __STONEAGE
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>
#include "autil.h"

/* Tokyo-sa: 字段编解码固定密钥 */
char TokyoKey[256] = "www.longzoro.com";
#include "char.h"
#ifdef __STONEAGE
#include "lssproto_util.h"
#include "common.h"
#endif

// Nuke 0701 fix
char *MesgSlice[SLICE_MAX];
int SliceCount;

char PersonalKey[1024 * 4];

// -------------------------------------------------------------------
// Initialize utilities
//
BOOL util_Init(void)
{
	int i;

	for (i = 0; i < SLICE_MAX; i++) {
		MesgSlice[i] = (char *)calloc(1, SLICE_SIZE);
		if (MesgSlice[i] == NULL)
			return FALSE;
	}

	SliceCount = 0;
	strcpy(PersonalKey, _DEFAULT_PKEY);

	return TRUE;
}

// -------------------------------------------------------------------
// Split up a message into slices by spearator.  Store those slices
// into a global buffer "char **MesgSlice"
//
// arg: source=message string;  separator=message separator (1 byte)
// ret: (none)

// WON ADD
// void util_SplitMessage(char *source, char *separator)
BOOL util_SplitMessage(char *source, char *separator)
{
	if (source && separator) { // NULL input is invalid.
		char *ptr;
		char *head = source;

		// Nuke 1006 : Bug fix
		while ((ptr = (char *)strstr(head, separator)) && (SliceCount < SLICE_MAX) && (SliceCount >= 0)) {
			ptr[0] = '\0';
			if (strlen(head) < SLICE_SIZE) { // discard slices too large

				// Nuke 0701
				//		if (*MesgSlice != *dumb) {
				// print("Warning! Mem may be broken\n");
				//}
				/*
						if (MesgSlice[SliceCount]==0xffffffff) {
								print("MesgSlice[%d] broken\n",SliceCount);
								return FALSE;
						} else {
				*/
				strcpy(MesgSlice[SliceCount], head);
				SliceCount++;
				//}
			}
			head = ptr + 1;
		}
		strcpy(source, head); // remove splited slices
	}
	return TRUE;
}

// -------------------------------------------------------------------
// Encode the message
//
// arg: dst=output  src=input
// ret: (none)
void util_EncodeMessage(char *dst, char *src)
{
	//  strcpy(dst, src);
	//  util_xorstring(dst, src);

	int rn = rand() % 99;
	int t1, t2;
	char t3[1024 * 64], tz[1024 * 64];

	// encode seed
#ifdef _BACK_VERSION
	util_swapint(&t1, &rn, "3421");
#else
	util_swapint(&t1, &rn, "2413");
#endif
	t2 = t1 ^ 0xffffffff;
	util_256to64(tz, (char *)&t2, sizeof(int), DEFAULTTABLE);

	util_shlstring(t3, src, rn);
	strcat(tz, t3);
	util_xorstring(dst, tz);
}

// -------------------------------------------------------------------
// Decode the message
//
// arg: dst=output  src=input
// ret: (none)
void util_DecodeMessage(char *dst, char *src)
{
	//  strcpy(dst, src);
	//  util_xorstring(dst, src);

#define INTCODESIZE (sizeof(int) * 8 + 5) / 6

	int rn;
	int *t1, t2;
	char t3[1024 * 4], t4[1024 * 4]; // This buffer is enough for an integer.
	char tz[1024 * 64];

	if (src[strlen(src) - 1] == '\n')
		src[strlen(src) - 1] = '\0';
	util_xorstring(tz, src);

	// get seed
	strncpy(t4, tz, INTCODESIZE);
	t4[INTCODESIZE] = '\0';
	util_64to256(t3, t4, DEFAULTTABLE);
	t1 = (int *)t3;
	t2 = *t1 ^ 0xffffffff;
#ifdef _BACK_VERSION
	util_swapint(&rn, &t2, "4312");
#else
	util_swapint(&rn, &t2, "3142");
#endif

	util_shrstring(dst, tz + INTCODESIZE, rn);
}

// -------------------------------------------------------------------
// Get a function information from MesgSlice.  A function is a complete
// and identifiable message received, beginned at DEFAULTFUNCBEGIN and
// ended at DEFAULTFUNCEND.  This routine will return the function ID
// (Action ID) and how many fields this function have.
//
// arg: func=return function ID    fieldcount=return fields of the function
// ret: 1=success  0=failed (function not complete)
int util_GetFunctionFromSlice(int *func, int *fieldcount)
{
	char t1[1024 * 16];
	int i;

	//  if (strcmp(MesgSlice[0], DEFAULTFUNCBEGIN)!=0) util_DiscardMessage();

	strcpy(t1, MesgSlice[1]);
	// Robin adjust
	//*func=atoi(t1);
	*func = atoi(t1) - 13;
	for (i = 0; i < SLICE_MAX; i++)
		if (strcmp(MesgSlice[i], DEFAULTFUNCEND) == 0) {
			*fieldcount = i - 2; // - "&" - "#" - "func" 3 fields
			return 1;
		}

	return 0; // failed: message not complete
}

void util_DiscardMessage(void)
{
	SliceCount = 0;
}

#ifdef _CHECK_BATTLE_IO
extern int InBattleLoop;
extern int battle_write;
extern int other_write;
extern int battle_write_cnt;
extern int other_write_cnt;
#endif

void _util_SendMesg(char *file, int line, int fd, int func, char *buffer)
{
	// char t1[1024*16], t2[1024*16];
	char t1[1024 * 64], t2[1024 * 64];

	// WON ADD
	if (fd < 0) {
		print("\n SendMesg fd err %s:%d!! ==> func(%d)\n", file, line, func);
		return;
	}

	// Robin adjust
	// sprintf(t1, "&;%d%s;#;", func, buffer);
	sprintf(t1, "&;%d%s;#;", func + 23, buffer);
	print("[SEND-MESG] fd=%d func=%d(+23=%d) key='%s' plain='%s'\n", fd, func, func+23, PersonalKey, t1);
	util_EncodeMessage(t2, t1);
	print("[SEND-MESG] encrypted len=%d hex=", (int)strlen(t2));
	{ int i; for (i = 0; i < (int)strlen(t2) && i < 40; i++) print("%02x", (unsigned char)t2[i]); }
	print("\n");
#ifdef _CHECK_BATTLE_IO
	if (InBattleLoop) {
		battle_write_cnt++;
		battle_write += strlen(buffer);
	} else {
		other_write += strlen(buffer);
		other_write_cnt++;
	}
#endif
#ifdef __STONEAGE
	lssproto_Send(fd, t2);
#endif
}

int util_256to64(char *dst, char *src, int len, char *table)
{
	unsigned int dw, dwcounter, i;

	if (!dst || !src || !table)
		return 0;
	dw = 0;
	dwcounter = 0;
	for (i = 0; i < len; i++) {
		dw = (((unsigned int)src[i] & 0xff) << ((i % 3) * 2)) | dw;
		dst[dwcounter++] = table[dw & 0x3f];
		dw = (dw >> 6);
		if (i % 3 == 2) {
			dst[dwcounter++] = table[dw & 0x3f];
			dw = 0;
		}
	}
	if (dw)
		dst[dwcounter++] = table[dw];
	dst[dwcounter] = '\0';
	return dwcounter;
}

// -------------------------------------------------------------------
// Convert 6-bit strings into 8-bit strings, buffers that store these strings
// must have enough space.
//
// arg: dst=6-bit string;  src=8-bit string;  table=mapping table
// ret: 0=failed  >0=bytes converted
int util_64to256(char *dst, char *src, char *table)
{
	unsigned int dw, dwcounter, i;
	char *ptr = NULL;

	dw = 0;
	dwcounter = 0;
	if (!dst || !src || !table)
		return 0;
	for (i = 0; i < strlen(src); i++) {
		ptr = (char *)index(table, src[i]);
		if (!ptr)
			return 0;
		if (i % 4) {
			dw = ((unsigned int)(ptr - table) & 0x3f) << ((4 - (i % 4)) * 2) | dw;
			dst[dwcounter++] = dw & 0xff;
			dw = dw >> 8;
		} else {
			dw = (unsigned int)(ptr - table) & 0x3f;
		}
	}
	if (dw)
		dst[dwcounter++] = dw & 0xff;
	dst[dwcounter] = '\0';
	return dwcounter;
}

// -------------------------------------------------------------------
// This basically is a 256to64 encoder.  But it shifts the result by key.
//
// arg: dst=6-bit string;  src=8-bit string;  len=src strlen;
//      table=mapping table;  key=rotate key;
// ret: 0=failed  >0=bytes converted
int util_256to64_shr(char *dst, char *src, int len, char *table, char *key)
{
	unsigned int dw, dwcounter, i, j;

	if (!dst || !src || !table || !key)
		return 0;
	if (strlen(key) < 1)
		return 0; // key can't be empty.
	dw = 0;
	dwcounter = 0;
	j = 0;
	for (i = 0; i < len; i++) {
		dw = (((unsigned int)src[i] & 0xff) << ((i % 3) * 2)) | dw;
		dst[dwcounter++] = table[((dw & 0x3f) + key[j]) % 64]; // check!
		j++;
		if (!key[j])
			j = 0;
		dw = (dw >> 6);
		if (i % 3 == 2) {
			dst[dwcounter++] = table[((dw & 0x3f) + key[j]) % 64]; // check!
			j++;
			if (!key[j])
				j = 0;
			dw = 0;
		}
	}
	if (dw)
		dst[dwcounter++] = table[(dw + key[j]) % 64]; // check!
	dst[dwcounter] = '\0';
	return dwcounter;
}

// -------------------------------------------------------------------
// Decoding function of util_256to64_shr.
//
// arg: dst=8-bit string;  src=6-bit string;  table=mapping table;
//      key=rotate key;
// ret: 0=failed  >0=bytes converted
int util_shl_64to256(char *dst, char *src, char *table, char *key)
{
	unsigned int dw, dwcounter, i, j;
	char *ptr = NULL;

	if (!key || (strlen(key) < 1))
		return 0; // must have key

	dw = 0;
	dwcounter = 0;
	j = 0;
	if (!dst || !src || !table)
		return 0;
	for (i = 0; i < strlen(src); i++) {
		ptr = (char *)index(table, src[i]);
		if (!ptr)
			return 0;
		if (i % 4) {
			// check!
			dw = ((((unsigned int)(ptr - table) & 0x3f) + 640 - key[j]) % 64)
					 << ((4 - (i % 4)) * 2)
				 | dw;
			j++;
			if (!key[j])
				j = 0;
			dst[dwcounter++] = dw & 0xff;
			dw = dw >> 8;
		} else {
			// check!
			dw = (((unsigned int)(ptr - table) & 0x3f) + 640 - key[j]) % 64;
			j++;
			if (!key[j])
				j = 0;
		}
	}
	if (dw)
		dst[dwcounter++] = dw & 0xff;
	dst[dwcounter] = '\0';
	return dwcounter;
}

// -------------------------------------------------------------------
// This basically is a 256to64 encoder.  But it shifts the result by key.
//
// arg: dst=6-bit string;  src=8-bit string;  len=src strlen;
//      table=mapping table;  key=rotate key;
// ret: 0=failed  >0=bytes converted
int util_256to64_shl(char *dst, char *src, int len, char *table, char *key)
{
	unsigned int dw, dwcounter, i, j;

	if (!dst || !src || !table || !key)
		return 0;
	if (strlen(key) < 1)
		return 0; // key can't be empty.
	dw = 0;
	dwcounter = 0;
	j = 0;
	for (i = 0; i < len; i++) {
		dw = (((unsigned int)src[i] & 0xff) << ((i % 3) * 2)) | dw;
		dst[dwcounter++] = table[((dw & 0x3f) + 640 - key[j]) % 64];
		j++;
		if (!key[j])
			j = 0;
		dw = (dw >> 6);
		if (i % 3 == 2) {
			dst[dwcounter++] = table[((dw & 0x3f) + 640 - key[j]) % 64];
			j++;
			if (!key[j])
				j = 0;
			dw = 0;
		}
	}
	if (dw)
		dst[dwcounter++] = table[(dw + 640 - key[j]) % 64];
	dst[dwcounter] = '\0';
	return dwcounter;
}

// -------------------------------------------------------------------
// Decoding function of util_256to64_shl.
//
// arg: dst=8-bit string;  src=6-bit string;  table=mapping table;
//      key=rotate key;
// ret: 0=failed  >0=bytes converted
int util_shr_64to256(char *dst, char *src, char *table, char *key)
{
	unsigned int dw, dwcounter, i, j;
	char *ptr = NULL;

	if (!key || (strlen(key) < 1))
		return 0; // must have key

	dw = 0;
	dwcounter = 0;
	j = 0;
	if (!dst || !src || !table)
		return 0;
	for (i = 0; i < strlen(src); i++) {
		ptr = (char *)index(table, src[i]);
		if (!ptr)
			return 0;
		if (i % 4) {
			// check!
			dw = ((((unsigned int)(ptr - table) & 0x3f) + key[j]) % 64)
					 << ((4 - (i % 4)) * 2)
				 | dw;
			j++;
			if (!key[j])
				j = 0;
			dst[dwcounter++] = dw & 0xff;
			dw = dw >> 8;
		} else {
			// check!
			dw = (((unsigned int)(ptr - table) & 0x3f) + key[j]) % 64;
			j++;
			if (!key[j])
				j = 0;
		}
	}
	if (dw)
		dst[dwcounter++] = dw & 0xff;
	dst[dwcounter] = '\0';
	return dwcounter;
}

// -------------------------------------------------------------------
// Swap a integer (4 byte).
// The value "rule" indicates the swaping rule.  It's a 4 byte string
// such as "1324" or "2431".
//
void util_swapint(int *dst, int *src, char *rule)
{
	char *ptr, *qtr;
	int i;

	ptr = (char *)src;
	qtr = (char *)dst;
	for (i = 0; i < 4; i++)
		qtr[rule[i] - '1'] = ptr[i];
}

// -------------------------------------------------------------------
// Xor a string.  Be careful that your string contains '0xff'.  Your
// data may lose.
//
void util_xorstring(char *dst, char *src)
{
	int i;
	if (strlen(src) > 1024 * 64)
		return;

	DebugPoint = 100000;

	for (i = 0; i < strlen(src); i++) {
		DebugPoint = 100000 + i;
		dst[i] = src[i] ^ 255;
	}
	dst[i] = '\0';
	DebugPoint = 1000;
}

// -------------------------------------------------------------------
// Shift the string right.
//
void util_shrstring(char *dst, char *src, int offs)
{
	char *ptr;
	if (!dst || !src || (strlen(src) < 1))
		return;

	offs = strlen(src) - (offs % strlen(src));
	ptr = src + offs;
	strcpy(dst, ptr);
	strncat(dst, src, offs);
	dst[strlen(src)] = '\0';
}

// -------------------------------------------------------------------
// Shift the string left.
//
void util_shlstring(char *dst, char *src, int offs)
{
	char *ptr;
	if (!dst || !src || (strlen(src) < 1))
		return;

	offs = offs % strlen(src);
	ptr = src + offs;
	strcpy(dst, ptr);
	strncat(dst, src, offs);
	dst[strlen(src)] = '\0';
}

// -------------------------------------------------------------------
// Convert a message slice into integer.  Return a checksum.
//
// arg: sliceno=slice index in MesgSlice    value=result
// ret: checksum, this value must match the one generated by util_mkint
int util_deint(int sliceno, int *value)
{
	int *t1, t2;
	char t3[1024 * 4]; // This buffer is enough for an integer.

	util_shl_64to256(t3, MesgSlice[sliceno], DEFAULTTABLE, TokyoKey);
	t1 = (int *)t3;
	t2 = *t1 ^ 0xffffffff;
#ifdef _BACK_VERSION
	util_swapint(value, &t2, "3421");
#else
	util_swapint(value, &t2, "2413");
#endif

	return *value;
}

int util_mkint(char *buffer, int value)
{
	int t1, t2;
	char t3[1024 * 4];
	int len, j;

#ifdef _BACK_VERSION
	util_swapint(&t1, &value, "4312");
#else
	util_swapint(&t1, &value, "3142");
#endif
	t2 = t1 ^ 0xffffffff;
	util_256to64_shr(t3, (char *)&t2, sizeof(int), DEFAULTTABLE, TokyoKey);
	/* Tokyo-sa: integer encoding is fixed 6 chars, pad with zero-encoding */
	len = strlen(t3);
	for (j = len; j < 6; j++) {
		t3[j] = DEFAULTTABLE[(TokyoKey[j % strlen(TokyoKey)]) % 64];
	}
	t3[j] = '\0';
	strcat(buffer, ";");
	strcat(buffer, t3);

	return value;
}

// -------------------------------------------------------------------
// Convert a message slice into string.  Return a checksum.
//
// arg: sliceno=slice index in MesgSlice    value=result
// ret: checksum, this value must match the one generated by util_mkstring
int util_destring(int sliceno, char *value)
{

	util_shr_64to256(value, MesgSlice[sliceno], DEFAULTTABLE, TokyoKey);

	return strlen(value);
}


// -------------------------------------------------------------------
// Tokyo-sa: convert a whole message from UTF-8 to GBK when the whole
// string is valid UTF-8; otherwise treat it as raw GBK data-file bytes
// and pass through unchanged.  Source tree string literals are UTF-8;
// data-file strings (maps, dialogues, items...) are GBK.  Byte-by-byte
// mixed conversion is wrong: GBK byte streams frequently contain
// accidental valid UTF-8 sequences (e.g. B4 E5 B3 A4 = "村长" contains
// E5 B3 A4 = UTF-8 U+54C9) and would be corrupted.
static int str_is_valid_utf8(const char *s)
{
	const unsigned char *p = (const unsigned char *)s;
	while (*p) {
		if (*p < 0x80) {
			p++;
			continue;
		}
		int need;
		unsigned int cp;
		if ((*p & 0xE0) == 0xC0) {
			need = 1; cp = *p & 0x1F;
		} else if ((*p & 0xF0) == 0xE0) {
			need = 2; cp = *p & 0x0F;
		} else if ((*p & 0xF8) == 0xF0) {
			need = 3; cp = *p & 0x07;
		} else {
			return 0; /* continuation or invalid lead */
		}
		if (need == 1 && cp < 2) return 0; /* overlong */
		if (need == 2 && cp == 0 && (p[1] & 0x20)) return 0; /* overlong */
		if (need == 3 && cp == 0) return 0; /* overlong */
		for (int i = 1; i <= need; i++) {
			if ((p[i] & 0xC0) != 0x80) return 0;
			cp = (cp << 6) | (p[i] & 0x3F);
		}
		if (need == 2 && cp < 0x800) return 0;
		if (need == 3 && cp < 0x10000) return 0;
		if (cp > 0x10FFFF) return 0;
		if (cp >= 0xD800 && cp <= 0xDFFF) return 0;
		p += need + 1;
	}
	return 1;
}

static size_t str_utf8_to_gbk_mixed(char *out, size_t outlen, const char *in)
{
	static iconv_t cd = (iconv_t)-1;
	char *src = (char *)in;
	char *dst = out;
	size_t srclen = strlen(in);
	size_t dstlen = outlen - 1;

	if (!str_is_valid_utf8(in)) {
		/* raw GBK (data-file) string: copy verbatim */
		while (srclen > 0 && dstlen > 0) {
			*dst++ = *src++;
			srclen--;
			dstlen--;
		}
		*dst = 0;
		return (size_t)(dst - out);
	}
	if (cd == (iconv_t)-1)
		cd = iconv_open("GBK", "UTF-8");
	if (cd == (iconv_t)-1) {
		/* iconv unavailable: copy verbatim */
		while (srclen > 0 && dstlen > 0) {
			*dst++ = *src++;
			srclen--;
			dstlen--;
		}
		*dst = 0;
		return (size_t)(dst - out);
	}
	iconv(cd, NULL, NULL, NULL, NULL); /* reset state */
	while (srclen > 0 && dstlen > 0) {
		size_t r = iconv(cd, &src, &srclen, &dst, &dstlen);
		if (r == (size_t)-1) {
			if (errno == E2BIG && srclen > 0) {
				break; /* output buffer full: stop */
			}
			break;
		}
	}
	*dst = 0;
	return (size_t)(dst - out);
}

// -------------------------------------------------------------------
// Tokyo-sa: convert a single data-file string from GBK to UTF-8 so it
// can be safely concatenated with UTF-8 source-tree literals (e.g.
// "拾获 %s" in char_item.c).  The concatenated whole string is then a
// valid UTF-8 string and str_utf8_to_gbk_mixed() will convert it back
// to GBK at send time, keeping the client display clean.  Invalid GBK
// bytes are passed through one by one instead of aborting.
size_t str_gbk_to_utf8(char *out, size_t outlen, const char *in)
{
	static iconv_t cd = (iconv_t)-1;
	char *src = (char *)in;
	char *dst = out;
	size_t srclen = strlen(in);
	size_t dstlen = outlen - 1;

	if (str_is_valid_utf8(in)) {
		/* already valid UTF-8: copy verbatim */
		size_t l = srclen;
		if (l >= outlen)
			l = outlen - 1;
		memcpy(out, in, l);
		out[l] = 0;
		return l;
	}
	if (cd == (iconv_t)-1)
		cd = iconv_open("UTF-8", "GBK");
	if (cd == (iconv_t)-1) {
		/* iconv unavailable: copy verbatim */
		while (srclen > 0 && dstlen > 0) {
			*dst++ = *src++;
			srclen--;
			dstlen--;
		}
		*dst = 0;
		return (size_t)(dst - out);
	}
	iconv(cd, NULL, NULL, NULL, NULL); /* reset state */
	while (srclen > 0 && dstlen > 0) {
		size_t r = iconv(cd, &src, &srclen, &dst, &dstlen);
		if (r == (size_t)-1) {
			if (errno == EILSEQ && srclen > 0 && dstlen > 0) {
				/* invalid GBK byte: pass it through and resync */
				*dst++ = *src++;
				srclen--;
				dstlen--;
				iconv(cd, NULL, NULL, NULL, NULL);
				continue;
			}
			break;
		}
	}
	*dst = 0;
	return (size_t)(dst - out);
}

// -------------------------------------------------------------------
// Convert a string into buffer (a string).  Return a checksum.
//
// arg: buffer=output   value=data to pack
// ret: checksum, this value must match the one generated by util_destring
int util_mkstring(char *buffer, char *value)
{
	char t1[SLICE_SIZE];
	char conv[SLICE_SIZE];
	size_t clen;

	clen = str_utf8_to_gbk_mixed(conv, sizeof(conv), value);
	util_256to64_shl(t1, conv, clen, DEFAULTTABLE, TokyoKey);
	strcat(buffer, ";"); // It's important to append a SEPARATOR between fields
	strcat(buffer, t1);

	return clen;
}

