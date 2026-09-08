#include "version.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "util.h"
#include "configfile.h"

#define IS_2BYTEWORD(_a_) ((char)(0x80) <= (_a_) && (_a_) <= (char)(0xFF))

/*-----------------------------------------
  行末の改行を取り除く (去除行末换行)
  引数は 元の 文字列 を 渡される。 (参数：传入原始字符串)

  windows , macintosh , unixの行の改行を削除する。 (删除windows、macintosh、unix的行尾换行)

  windows : \r\n
  macintosh : \n\r
  unix : \n

  ただし (但是)
  macintosh : \r
  である。 (即)
  それゆえの場合、行末の 文字がなくても (因此，即使没有行尾字符)
  改行があればそれでよい (只要有换行符即可)
  -----------------------------------------*/
void chompex(char *buf)
{
	while (*buf) {
		if (*buf == '\r' || *buf == '\n') {
			*buf = '\0';
		}
		buf++;
	}
}

/*----------------------------------------
 * 行末の1 文字を取り除く (去除行尾的1个字符)
 * 引数
 *  src           渡される 文字
 ----------------------------------------*/
void chop(char *src)
{
	/*
	 * ただ最後の 文字を取り除く (只去掉最后的字符)
	 */
	int length = strlen(src);
	if (length == 0)
		return;
	src[length - 1] = '\0';
}

/*----------------------------------------
 * 行末の 文字を取る。その 文字を 削除する。 (取行尾字符并删除)
 * 引数
 *  src     元の 文字
 *  delim   削除する 文字列 ［  はNULL 文字で代用される (要删除的字符 [为NULL时用\0代替])
 ----------------------------------------*/
void dchop(char *src, char *del)
{
	int dellen, srclen;
	int i;
	BOOL delete = FALSE;

	/*
	 * srcの 末尾を調べる。1 文字ずつ del すべてを比較する (检查src末尾，与del各字符逐一比较)
	 * 一致するものがあれば、その後の 文字にしない。 (若匹配则去除该字符)
	 */

	srclen = strlen(src);
	dellen = strlen(del);
	if (srclen == 0 || dellen == 0)
		return;

	for (i = 0; i < dellen; i++) {
		if (src[srclen - 1] == del[i]) {
			delete = TRUE;
			break;
		}
	}

	/*  最後を削除 (删除末尾)    */
	if (delete)
		src[srclen - 1] = '\0';
}

/*----------------------------------------
 * dchopの逆、先頭の 文字を取る。その 文字を 削除する。 (与dchop相反，取开头字符并删除)
 * 引数
 *  src     元の 文字
 *  delim   削除する 文字列 ［  はNULL 文字で代用される (要删除的字符 [为NULL时用\0代替])
 ----------------------------------------*/
void pohcd(char *src, char *del)
{
	int dellen, srclen;
	int i;
	BOOL delete = FALSE;

	/*
	 * srcの 先頭を調べる。1 文字ずつ del すべてを比較する (检查src开头，与del各字符逐一比较)
	 * 一致するものがあれば、その後の 文字にしない。 (若匹配则去除该字符)
	 */

	srclen = strlen(src);
	dellen = strlen(del);
	if (srclen == 0 || dellen == 0)
		return;

	for (i = 0; i < dellen; i++) {
		if (src[0] == del[i]) {
			delete = TRUE;
			break;
		}
	}

	/*  削除 (删除)    */
	if (delete)
		/*  これの後に NULL 文字をコピーする (其后复制NULL字符) */
		for (i = 0; i < srclen; i++)
			src[i] = src[i + 1];
}

/*----------------------------------------
 *   文字 に 文字が含まれているかどうかを調べる。 (检查是否包含某字符)
 * 引数
 *  src     char*   調べる 文字
 *  include char*   含まれているかどうかの 文字 ［ (要查找的字符)
 * 戻り値
 *  真        先頭に含まれている場合srcの位置 (若包含则返回src位置)
 *  偽      -1 (假 -1)
 ----------------------------------------*/
int charInclude(char *src, char *include)
{
	int i, j;
	int srclen, includelen;

	srclen = strlen(src);
	includelen = strlen(include);

	for (i = 0; i < srclen; i++) {
		for (j = 0; j < includelen; j++)
			if (src[i] == include[j])
				return i;
	}
	return -1;
}

/*------------------------------------------------------------
 * EUCの2バイト 文字をコピーしないstrncpy (不截断EUC双字节字符的strncpy)
 * freeBSD のライブラリから strncpy を取り出して、 (从freeBSD的库中取出strncpy)
 * それをもとに改造。*dest++ とか使いたいけど、他の用途に没頭が (以此为基础改造，本想用*dest++)
 * ないので、使っていない。かわりに だけど、ポインタも使ってて (故未使用，但指针仍在使用)
 * かなり やっかいなプログラム (相当麻烦的程序)
 * 引数
 *  dest        char*   コピー先 (复制目标)
 *  src         char*   ソース (来源)
 *  n           size_t  長さ (长度)
 * 戻り値
 *  dest
 ------------------------------------------------------------*/
char *strncpy2(char *dest, const char *src, size_t n)
{
	if (n > 0) {
		char *d = dest;
		const char *s = src;
		int i;
		for (i = 0; i < n; i++) {
			if (*(s + i) == 0) {
				/*  コピーしていたら NULL 文字を 入れる (复制完则写入NULL字符)   */
				*(d + i) = '\0';
				return dest;
			}
			if (*(s + i) & 0x80) {
				*(d + i) = *(s + i);
				i++;
				if (i >= n) {
					*(d + i - 1) = '\0';
					break;
				}
				*(d + i) = *(s + i);
			} else
				*(d + i) = *(s + i);
		}
	}
	return dest;
}

/*----------------------------------------
 * src から dest にコピーする。 コピー先の大きさを調べる。 (将src复制到dest，检查目标大小)
 * 引数
 *      dest        コピー先 (目标)
 *      n           大きさ (大小)
 *      src         元の 文字
 ----------------------------------------*/
void strcpysafe(char *dest, size_t n, const char *src)
{
	/*
	 * src から dest へコピーする. (将src复制到dest)
	 * strcpy, strncpy では dest 側に コピーする挙動 (strcpy、strncpy向dest侧复制)
	 *   長い場合に効率が、パフォーマンスが悪くなる. (长度较长时性能下降)
	 * これを 防ぐため、 strncpy を用いるが strlen( src ) が n より (为防止此问题使用strncpy，但strlen(src)比n大时)
	 *   長い場合には、 dest の 末尾が NULL 文字にはならない. (较长时dest末尾不会自动补NULL)
	 *
	 * したがって dest の 大きさより src の長さが長い場合は (因此，当src比dest长时)
	 * n-1 で strncpy を用いる. 足りない場合はそのままコピーする (用n-1做strncpy，不足则直接复制)
	 *
	 * n が 大きい場合を考えないので その場合 止まらない。 (不考虑n很大的情况)
	 *
	 */
	// Nuke +1 (08/25): Danger if src=0
	if (!src) {
		*dest = '\0';
		return;
	}
	if (n <= 0) /* 何もしない (不处理)   */
		return;

	/*  この場合は、 n >= 1 番目が確保  (此情况n>=1已确保)  */
	/*  NULL 文字を含めて 入れる (包含NULL字符写入)  */
	else if (n < strlen(src) + 1) {
		/*
		 * 末尾を確保しないので n - 1(NULL 文字) (不预留末尾，n-1不含NULL)
		 * で strncpy を利用 (使用strncpy)
		 */
		strncpy2(dest, src, n - 1);
		dest[n - 1] = '\0';
	} else
		strcpy(dest, src);
}

/*----------------------------------------
 * src から length 分だけ。 コピー先の 大きさを調べる。 (从src复制length长度，检查目标大小)
 * 引数
 *          dest        コピー先 (目标)
 *          n           コピー先の大きさ (目标大小)
 *          src         コピー元 (来源)
 *          length      何 文字コピーするか (要复制的字符数)
 ----------------------------------------*/
void strncpysafe(char *dest, const size_t n,
				 const char *src, const int length)
{
	/*
	 * src から dest に length コピーする (从src复制length到dest)
	 * strcpy, strncpy では dest 側に コピーする挙動 (strcpy、strncpy向dest侧复制)
	 *   長い場合に効率が、パフォーマンスが悪くなる. (较长时性能下降)
	 * この関数では、strlen( src ) と length の小さい方 (此函数取strlen(src)与length中较小者)
	 * ( 端にコピーする挙動) と dest の大きさを 確認、 (复制到末尾的行为与dest大小)
	 * strcpysafe と 同じを使う。 (与strcpysafe相同)
	 */

	int Short;
	Short = min(strlen(src), length);

	/* NULL 文字 を含めて 入れる (写入时包含NULL) */
	if (n < Short + 1) {
		/*
		 * 末尾を確保しないので n - 1(NULL 文字) (不预留末尾，n-1不含NULL)
		 * で strncpy を利用 (使用strncpy)
		 */
		strncpy2(dest, src, n - 1);
		dest[n - 1] = '\0';

	} else if (n <= 0) {
		return;
	} else {
		/*
		 * 末尾は最後なので Short でstrncpyを利用 (末尾在最后，用Short做strncpy)
		 * また src には Short の長さ に NULL が入らないので、 (且src在Short长度内不会写入NULL)
		 * dest には 必ず入れている。 (dest则必定写入)
		 */

		strncpy2(dest, src, Short);
		dest[Short] = '\0';
	}
}

/*------------------------------------------------------------
 * strcatを安全に 実装する。危いので、 いろんな場合を処理して使わないと (安全地实现strcat，需处理各种情况)
 * 引数
 *  src     char*       元になる 文字
 *  size    int         src のサイズ( 後ろにどれだけ残っているかでは (src的大小，非剩余空间)
 *                          なくてバッファ全体  ) (而是整个缓冲区)
 *  ap      char*       付け加える (追加)
 * 戻り値
 *  src     (  使用後) (src 使用后)
 ------------------------------------------------------------*/
char *strcatsafe(char *src, int size, char *ap)
{
	strcpysafe(src + strlen(src), size - strlen(src), ap);
	return src;
}

/*----------------------------------------
 *   文字 の 末尾 を 比較する。 同じかどうかしか 判定しない by ringo (比较字符串尾部，只判定是否相同)
 * char *s1 , *s2 :   比較する 文字  (比较的字符串)
 * 戻り値
 * 0 : 同じ (0:相同)
 * 1 : 異なる (1:不同)
 * これはNPCの  チャット に使われてる。 (用于NPC聊天)
 ----------------------------------------*/
int strcmptail(char *s1, char *s2)
{
	int i;
	int len1 = strlen(s1);
	int len2 = strlen(s2);

	for (i = 0;; i++) {
		int ind1 = len1 - 1 - i;
		int ind2 = len2 - 1 - i;
		if (ind1 < 0 || ind2 < 0)
			return 0;
		if (s1[ind1] != s2[ind2])
			return 1;
	}
}

/*----------------------------------------
 *   文字 の 中 に書かれている数字をゲットする。 (取出字符串中的数字)
 * NPCの チャット に使われてる。 (用于NPC聊天)
 * ただし 中 にいらん 文字がある場合 無視する。 (但忽略其中的无关字符)
 * 性能が 高くないアルゴリズムに 変更 (改成性能较差的算法)
 * 引数
 *  hoge    char*   調べる 文字
 * 戻り値
 *  調べた 数字。 (查出的数字)
 *
 *   例 (示例)
 * "ringo ( 吧噗吧噗星人 ) ： 100"
 *      の場合なら100を返す。 (这种情况返回100)
 * "ringo ( 吧噗吧噗星人 ) ： 是100"
 *      は100になる。数字がなければNPCには出さない (为100，无数字则不输出给NPC)
 ----------------------------------------*/
#define DECIMALCHARNUM 10
#define FIGURECHARNUM 3
#define SPECIALCHARNUM 2
char *UTIL_decimalchar[DECIMALCHARNUM] = {"0", "1", "2",
										  "3", "4", "5",
										  "6", "7", "8", "9"};
char *UTIL_figurechar[FIGURECHARNUM] = {"", "", ""};
char *UTIL_specialchar[SPECIALCHARNUM] = {"", ""};

/*
 *   文字 から、数字を 含むと 思われる 部分を 探し出す。先 のほうから (从字符串中找出包含数字的部分)
 * 変換して 探し出す。euc用。 (转换后查找，用于EUC)
 *
 *    : in "这本书七万册300美金吗？"
 *      out "7万"
 * 引数
 * char *in:      文字  (输入字符串)
 * char *out:探した 文字  (输出字符串)
 * int outsiz:探した 文字 の末尾の大きさ (输出缓冲区大小)
 * 戻り値
 * 数字を 含む 部分があ見つたらその 文字 の長さ、な見つたら０ (找到数字部分返回长度，否则0)
 */
static int findNumberString(char *in, char *out, int outsiz)
{
	int len, i, j, k, findflag, numstrflag;
	char tmp[3];

	len = strlen(in);
	if (len == 0)
		return 0;

	numstrflag = 0;
	findflag = 0;
	for (i = 0, j = 0; in[i] != '\0' && j < outsiz - 2; i++) {
		findflag = 0;
		if (in[i] & 0x80) {
			/* EUCの場合 */
			tmp[0] = in[i];
			tmp[1] = in[i + 1];
			tmp[2] = '\0';
		} else {
			/* ASCIIの場合 */
			tmp[0] = in[i];
			tmp[1] = '\0';
		}
		for (k = 0; k < DECIMALCHARNUM; k++) {
			if (strstr(UTIL_decimalchar[k], tmp)) {
				numstrflag = 1;
				findflag = 1;
			}
		}
		for (k = 0; k < FIGURECHARNUM; k++) {
			if (strstr(UTIL_figurechar[k], tmp)) {
				numstrflag = 1;
				findflag = 1;
			}
		}
		for (k = 0; k < SPECIALCHARNUM; k++) {
			if (strstr(UTIL_specialchar[k], tmp)) {
				numstrflag = 1;
				findflag = 1;
			}
		}
		if (findflag) {
			if (in[i] & 0x80) {
				out[j] = tmp[0];
				out[j + 1] = tmp[1];
				j += 2;
				i++;
			} else {
				out[j] = tmp[0];
				j += 1;
			}
			continue;
		}

		if (numstrflag) {
			out[j] = '\0';
			return j;
		}
	}

	if (findflag) {
		out[j] = '\0';
		return j;
	} else {
		return 0;
	}
}

/*
 *   文字 をintに 変換。 (将字符串转为int)
 *
 */
int numstrIsKanji(char *str);
int decimalstrToInt(char *str);
int kanjistrToInt(char *str);

int textToInt(char *str)
{
	char numstr[256];
	if (!findNumberString(str, numstr, sizeof(numstr)))
		return 0;

	if (numstrIsKanji(numstr))
		return kanjistrToInt(numstr);
	else
		return decimalstrToInt(numstr);
}

/*
 * 数字を 含む 文字 が、 万 の位なのか億 の位なのかを (判断数字串是万位还是亿位)
 * 引数とする 文字 ［数字を 含む 文字 数字以外は含んではいけない。 (参数：字符串，不能包含数字以外的内容)
 * 戻り値     万の位なら１、億の位なら０ (万位返回1，亿位返回0)
 *    str が"一亿四千万。"のときは戻り値1 (str为"一亿四千万"时返回1)
 *    str が"123"のときは戻り値0 (str为"123"时返回0)
 *
 */
int numstrIsKanji(char *str)
{
	int i, j;
	char tmp[3];

	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] & 0x80) {
			tmp[0] = str[i];
			tmp[1] = str[i + 1];
			tmp[2] = '\0';
			i++;
		} else {
			tmp[0] = str[i];
			tmp[1] = '\0';
		}
		for (j = 0; j < FIGURECHARNUM; j++) {
			if (strstr(UTIL_figurechar[j], tmp)) {
				return 1;
			}
		}
		for (j = 0; j < SPECIALCHARNUM; j++) {
			if (strstr(UTIL_specialchar[j], tmp)) {
				return 1;
			}
		}
	}
	return 0;
}

/*
 * 十進数で数字を した 文字 をintに 変換 (将十进制数字字符串转为int)
 * char *str   文字  (字符串)
 * 戻り値   文字 が 含む数字 (字符串中包含的数字)
 */
int decimalstrToInt(char *str)
{

	double val;
	char tmp[3];
	int i, j;

	val = 0;
	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] & 0x80) {
			tmp[0] = str[i];
			tmp[1] = str[i + 1];
			tmp[2] = '\0';
			i++;
		} else {
			tmp[0] = str[i];
			tmp[1] = '\0';
		}
		for (j = 0; j < DECIMALCHARNUM; j++) {
			if (strstr(UTIL_decimalchar[j], tmp)) {
				val = val * 10 + j;
			}
		}
	}

	if (val > 0x7fffffff)
		return 0x7fffffff;
	else
		return (int)val;
}

/*
 *     万で した数字を整数に 変換 (将万位数字转为整数)
 * char *str 数字を 含む 文字  (包含数字的字符串)
 * 戻り値 数字 (返回值 数字)
 *
 * strが"一亿4千万？"のときは戻り値 140000000 (str为"一亿4千万？"时返回140000000)
 */
int kanjistrToInt(char *str)
{
	double val, tmpval;
	char tmp[3];
	int num, i, j, numflag;

	numflag = 0;
	num = 1;
	tmpval = 0;
	val = 0;
	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] & 0x80) {
			tmp[0] = str[i];
			tmp[1] = str[i + 1];
			tmp[2] = '\0';
			i++;
		} else {
			tmp[0] = str[i];
			tmp[1] = '\0';
		}
		for (j = 0; j < SPECIALCHARNUM; j++) {
			if (strstr(UTIL_specialchar[j], tmp)) {
				if (numflag == 1)
					tmpval += num;
				if (j == 0)
					val += tmpval * 10000;
				else if (j == 1)
					val += tmpval * 100000000;
				num = 1;
				numflag = 0;
				tmpval = 0;
				goto nextchar;
			}
		}
		for (j = 0; j < FIGURECHARNUM; j++) {
			if (strstr(UTIL_figurechar[j], tmp)) {
				if (j == 0)
					tmpval += num * 10;
				else if (j == 1)
					tmpval += num * 100;
				else if (j == 2)
					tmpval += num * 1000;
				num = 1;
				numflag = 0;
				goto nextchar;
			}
		}
		for (j = 0; j < DECIMALCHARNUM; j++) {
			if (strstr(UTIL_decimalchar[j], tmp)) {
				num = j;
				numflag = 1;
				goto nextchar;
			}
		}
	nextchar:
		continue;
	}
	if (numflag == 1)
		tmpval += num;
	val += tmpval;

	if (val > 0x7fffffff)
		return 0x7fffffff;
	else
		return (int)val;
}

/*----------------------------------------
 *   文字 の 中 に含まれている数字を検索する。 (在字符串中查找数字)
 * NPCの チャット に使われてる。 (用于NPC聊天)
 * ただし 中 に当てはま 文字がある場合 無視する。 (但忽略符合情况的字符)
 * 性能を 高くないパフォーマンスに 書い (为性能不高而写)
 * 引数
 *  hoge    char*   調べる 文字
 * 戻り値
 *  調べた 数字。 (查出的数字)
 *
 *   例 (示例)
 * "ringo ( 吧噗吧噗星人 ) ： 100"
 *      の場合なら100を返す。 (这种情况返回100)
 * "ringo ( 吧噗吧噗星人 ) ： 是100"
 *      は100になる。数字がなければNPCには出さない (为100，无数字则不输出给NPC)
 ----------------------------------------*/
int texttoiTail(char *hoge)
{
	return 0; /* バグってたのでこうした。そのうち関数ごと消す。 (因有bug而这样处理，将来整个函数删除) */

#if 0
    int len = strlen(hoge);

    if( len == 0 )return atoi(hoge);
    /*    から変換して 先頭に数字を探す (从开头查找数字)    */
    while(--len && len >= 0){
        if( isdigit(hoge[len]) )
            break;
    }
    while( --len && len >= 0 ){
        if( !isdigit(hoge[len]) )
            break;
        if( hoge[len]=='+' || hoge[len]=='-')
            break;
    }
    return atoi(hoge+len+1);
#endif
#if 0
    int f = 0;
    for(i=len-1;i>=0;i--){
        if( isdigit( hoge[i] ) ) f = 1;
        if( (f == 1)
            && !( isdigit(hoge[i]) || hoge[i]=='+' || hoge[i]=='-') ){
            return atoi( hoge + i + 1 );
        }
    }
    return atoi(hoge);
#endif
}

/*----------------------------------------
 *   文字 を数字かどうかチェックして、数字だ見つたら (检查字符串是否为数字，若是数字则)
 *  引数に 入れて返す (存入参数并返回)
 * 引数
 *      arg               変換元 (要转换的值)
 *      number            変換 を受 ける所 (接收转换结果的位置)
 *      base            基数 (基数)
 *      type            引数の型( CTYPE は common.h に定義されている ) (参数类型)
 * 戻り値
 *      TRUE(1)    真 (真)
 *      FALSE(0)   strtolが完璧には真 しな見つた (strtol未能完全转换)
 *                  この場合のnumberはstrtolの戻り値である (此时number是strtol的返回值)
 ----------------------------------------*/
BOOL strtolchecknum(char *arg, void *number, int base, CTYPE type)
{
	char *buf;
	int num;

	num = strtol(arg, &buf, base);

	switch (type) {
	case CHAR:
		*(char *)number = (char)num;
		break;
	case SHORT:
		*(short *)number = (short)num;
		break;
	case INT:
		*(int *)number = num;
		break;
	case DOUBLE:
		*(double *)number = (double)num;
		break;
	default:
		break;
	}

	if (strlen(buf) >= 1)
		/*
		 * 残り 文字があるという事は 変換しきれてないという事なのだ。 (还有剩余字符说明未转换完全)
		 */
		return FALSE;

	return TRUE;
}

/*----------------------------------------
 * src から dels で指定した 文字を取り除く (从src中删除dels指定的字符)
 * バックスラッシュはエスケープである事に注意。 (注意反斜杠是转义符)
 * つまりバックスラッシュは削除できない。 (即反斜杠不能删除)
 * 引数
 *  src      渡される 文字
 *  char*  削除する 文字列(  文字 で削除対象) (要删除的字符)
 * 戻り値
 *  なし
 ----------------------------------------*/
void deleteCharFromString(char *src, char *dels)
{
	int index = 0; /* できあがりの 文字 の index (输出字符的索引) */
	int delength;  /* dels の長さを保存する(元は速度アップの為 (保存dels长度，其实是为了提速) */
	int i = 0, j;  /* i は src をループする 数 j は dels をループする 数 (i遍历src，j遍历dels) */

	delength = strlen(dels);

	while (src[i] != '\0') {
		if (src[i] == BACKSLASH)
			/*
			 * バックスラッシュなのでエスケープする (是反斜杠则转义)
			 * つまり後の 文字は そのままに移 する (后续字符原样保留)
			 */
			src[index++] = src[++i]; /* substitute next char */

		else {
			for (j = 0; j < delength; j++) {
				if (src[i] == dels[j])
					/*
					 * 削除する 文字だ。よって i に 1 をたす (是删除字符则i加1)
					 */
					goto incrementi;
			}
			/*
			 * 消す 指定されていないのでコピーする。 (未指定删除则复制)
			 */
			src[index++] = src[i];
		}
	incrementi:
		i++;
	}

	src[index] = '\0';
}

/*----------------------------------------
 * src から dels で指定した 文字を取り除く (从src中删除dels指定的字符)
 * エスケープはない (不转义)
 * 引数
 *  src      渡される 文字
 *  char*  削除する 文字列(  文字 で削除対象) (要删除的字符)
 * 戻り値
 *  なし
 ----------------------------------------*/
void deleteCharFromStringNoEscape(char *src, char *dels)
{
	int index = 0; /* できあがりの 文字 の index (输出字符的索引) */
	int delength;  /* dels の長さを保存する(元は速度アップの為 (保存dels长度，其实是为了提速) */
	int i = 0, j;  /* i は src をループする 数 j は dels をループする 数 (i遍历src，j遍历dels) */

	delength = strlen(dels);

	while (src[i] != '\0') {
		for (j = 0; j < delength; j++) {
			if (src[i] == dels[j])
				/*
				 * 削除する 文字だ。よって i に 1 をたす (是删除字符则i加1)
				 */
				goto incrementi;
		}
		/*
		 * 消す 指定されていないのでコピーする。 (未指定删除则复制)
		 */
		src[index++] = src[i];
	incrementi:
		i++;
	}

	src[index] = '\0';
}

/*------------------------------------------------------------
 * 指定された 文字を指定された 文字に 置き換える (将指定字符替换为另一字符)
 * 引数
 *  src     char*         渡される 文字
 *  oldc    char          置き換える 文字 (被替换的字符)
 *  newc    char          置く 文字 (替换成的字符)
 * 戻り値
 *  src
 ------------------------------------------------------------*/
char *replaceString(char *src, char oldc, char newc)
{
	char *cp = src;

	do {
		if (*cp == oldc)
			*cp = newc;
	} while (*cp++);
	return src;
}

typedef struct tagEscapeChar
{
	char escapechar;
	char escapedchar;
} EscapeChar;
static EscapeChar escapeChar[] =
	{
		{'\n', 'n'},
		{',', 'c'},
		{'|', 'z'},
		{'\\', 'y'},
};

char makeCharFromEscaped(char c) // add this function,because the second had it
{
	int i;

	for (i = 0; i < sizeof(escapeChar) / sizeof(escapeChar[0]); i++) {
		if (escapeChar[i].escapedchar == c) {
			c = escapeChar[i].escapechar;
			break;
		}
	}
	return c;
}
/*----------------------------------------
 * makeEscapeStringで変換された 文字 を元に す (将makeEscapeString转换的字符串还原)
 * 引数
 *  src             char*       元になる 文字 ［これを持ち帰る［ (原始字符串，使用后带回)
 * 戻り値
 *  src を返す。(関数を 呼れこにしやすいように) (返回src，便于调用函数)
 ----------------------------------------*/
char *makeStringFromEscaped(char *src)
{ // ttom this function all change,beside copy from the second

	int i;
	// CoolFish: Fix bug 2001/10/13
	// int     srclen = strlen( src );
	int srclen = 0;
	int searchindex = 0;

	// CoolFish: Fix bug 2001/10/13
	if (!src)
		return NULL;
	srclen = strlen(src);

	for (i = 0; i < srclen; i++) {
		// for 2Byte Word
		if (IS_2BYTEWORD(src[i])) {
			src[searchindex++] = src[i++];
			src[searchindex++] = src[i];
		} else if (src[i] == '\\') {
			// 後の 文字に変える (替换为后续字符)
			i++;
			src[searchindex++] = makeCharFromEscaped(src[i]);
		} else {
			src[searchindex++] = src[i];
		}
	}
	src[searchindex] = '\0';

	return src;
}

char *makeEscapeString(char *src, char *dest, int sizeofdest)
{ // ttom this function all change, copy from the second
	int i;
	int srclen = 0;
	int destindex = 0;

	// CoolFish: Fix bug 2001/10/13
	if (!src)
		return NULL;
	srclen = strlen(src);

	for (i = 0; i < srclen; i++) {
		BOOL dirty = FALSE;
		int j;
		char escapechar = '\0';
		if (destindex + 1 >= sizeofdest)
			break;
		if (IS_2BYTEWORD(src[i])) {
			if (destindex + 2 >= sizeofdest)
				break;

			dest[destindex] = src[i];
			dest[destindex + 1] = src[i + 1];
			destindex += 2;
			i++;
			continue;
		}
		for (j = 0; j < sizeof(escapeChar) / sizeof(escapeChar[0]); j++) {
			if (src[i] == escapeChar[j].escapechar) {
				dirty = TRUE;
				escapechar = escapeChar[j].escapedchar;
				break;
			}
		}
		if (dirty == TRUE) {
			if (destindex + 2 < sizeofdest) {
				dest[destindex] = '\\';
				dest[destindex + 1] = escapechar;
				destindex += 2;
				dirty = TRUE;
				continue;
			} else {
				dest[destindex] = '\0';
				return dest;
			}
		} else {
			dest[destindex] = src[i];
			destindex++;
		}
	}
	dest[destindex] = '\0';
	return dest;
}

// this function copy all from the second
char *ScanOneByte(char *src, char delim)
{
	// Nuke
	if (!src)
		return NULL;

	//   文字 がなくなるまで走査 (扫描至字符串末尾)
	for (; src[0] != '\0'; src++) {
		// 最後の１バイト かどうかをチェック (检查是否为最后的单字节)
		if (IS_2BYTEWORD(src[0])) {
			// 最後だ。その場合は１バイト余分に進ませる。 (若是则额外推进1字节)
			// ただし１バイトしかない場合はそうしない (但只剩1字节则不这样)
			if (src[1] != 0) {
				src++;
			}
			continue;
		}
		//   最後だ見つた。ここで対象の 文字と 比較 (到末尾则在此与目标字符比较)
		if (src[0] == delim) {
			return src;
		}
	}
	// ループ 終えたら見つからな見つた。 (循环结束仍未找到)
	return NULL;
}

/*----------------------------------------
 * delim で指定された 文字 を区切り 文字として (以delim作为分隔符)
 * index     の 値 を 得る。indexは1ではじまる。 (取得索引，index从1开始)
 *   文字 注意 。 (注意字符串)
 * 引数
 *  src     元の 文字
 *  delim   デリミタとなる 文字 ［ はNULL 文字で終わっている事 (分隔符，NULL表示到结尾)
 *  index   何 番目 の 値 を取るか (取第几个)
 *  buf     結果 文字 を入 する場所へのポインター (存放结果的指针)
 *  buflen  結果 文字 を入 する場所のサイズ (缓冲区大小)
 * 戻り値
 *  見つかった TRUE(1); (找到返回TRUE)
 *  見つからなかった FALSE(0); (未找到返回FALSE)
 *  ex
 *      getStringFromIndexWithDelim( "aho=hoge","=",1,buf,sizeof(buf) );
 *      buf ... aho
 *
 *      getStringFromIndexWithDelim( "aho=hoge","=",2,buf,sizeof(buf) );
 *      buf ... hoge
 *      これも 説明される。 (同理)
 *
 *      getStringFromIndexWithDelim( "aho=hoge","=",3,buf,sizeof(buf) );
 *      戻り値 FALSE (返回FALSE)
 ----------------------------------------*/
BOOL getStringFromIndexWithDelim_body(char *src, char *delim, int index,
									  char *buf, int buflen,
									  char *file, int line)
{						 // ttom this function all change,copy from the second
	int i;				 /* ループ 数 (循环计数) */
	int length = 0;		 /* 取り出した 文字 の長さ (取出的字符串长度) */
	int addlen = 0;		 /* 加える長さ (增加的长度) */
	int oneByteMode = 0; /* １バイトモードかどうか (是否单字节模式) */

	if (strlen(delim) == 1) { // 走査が１バイトなら１バイトモードにする (若分隔符为单字节则用单字节模式)
		oneByteMode = 1;	  // その場合2バイト 文字は変換しない (此时不转换双字节字符)
	}
	for (i = 0; i < index; i++) {
		char *last;
		src += addlen; /* 見つかった長さを加え (加上找到的长度) */

		if (oneByteMode) {
			// ２バイトモードかどうかこれで走査 (按双字节模式扫描)
			last = ScanOneByte(src, delim[0]);
		} else {
			last = strstr(src, delim); /* 見つける (查找) */
		}
		if (last == NULL) {
			/*
			 * みつからなかったのですべてコピーして return。 (未找到则全部复制并返回)
			 */
			strcpysafe(buf, buflen, src);

			if (i == index - 1)
				/*ちょうどみつかった (正好找到)*/
				return TRUE;

			/*見つからなかった (未找到)*/
			return FALSE;
		}

		/*
		 * みつかった所と 先頭の位置 の差を求める (计算找到位置与开头的差)
		 * ただし抽出された 文字 の長さ (但为取出的字符串长度)
		 */
		length = last - src;

		/*
		 * 後のループの為にみつかった長さと delim の長さを加えておく (为后续循环加上找到长度与delim长度)
		 */
		addlen = length + strlen(delim);
	}
	strncpysafe(buf, buflen, src, length);

	return TRUE;
}

/*------------------------------------------------------------
 *   文字 "a,b,c,d" を引数に受 ける。デフォルトは 0 にする。 (接收a,b,c,d，无分隔为0)
 * 参考の 関数、実際は最後が使われてる。 (参考函数，实际用最后部分)
 * 引数
 *  src         char*   元の 文字
 *  int1        int*    intのポインター。(aを受 ける) (int指针，接收a)
 *  int2        int*    intのポインター。(bを受 ける) (int指针，接收b)
 *  int3        int*    intのポインター。(cを受 ける) (int指针，接收c)
 *  int4        int*    intのポインター。(dを受 ける) (int指针，接收d)
 * 戻り値
 *  なし
 ------------------------------------------------------------*/
void getFourIntsFromString(char *src, int *int1, int *int2, int *int3,
						   int *int4)
{
	int ret;
	char string[128];

	ret = getStringFromIndexWithDelim(src, ",", 1, string, sizeof(string));
	if (ret == FALSE)
		*int1 = 0;
	else
		*int1 = atoi(string);

	ret = getStringFromIndexWithDelim(src, ",", 2, string, sizeof(string));
	if (ret == FALSE)
		*int2 = 0;
	else
		*int2 = atoi(string);

	ret = getStringFromIndexWithDelim(src, ",", 3, string, sizeof(string));
	if (ret == FALSE)
		*int3 = 0;
	else
		*int3 = atoi(string);

	ret = getStringFromIndexWithDelim(src, ",", 4, string, sizeof(string));
	if (ret == FALSE)
		*int4 = 0;
	else
		*int4 = atoi(string);
}

/*----------------------------------------------
 * src の 中でdelsで指定した 文字が 続していたら (src中连续出现dels指定字符时)
 * 1つにまとめる。 (合并为一个)
 * バックスラッシュはエスケープシーケンスとなる。 (反斜杠为转义序列)
 * したがって 続したバックスラッシュを消す事はできない。 (因此不能删除连续的反斜杠)
 * 引数
 *  src      変換する 文字
 *  dels   残す 文字列( 文字 で削除対象) (要删除的字符)
 * 戻り値
 *  なし
 ---------------------------------------------*/
void deleteSequentChar(char *src, char *dels)
{
	int length;			  /* src の長さ (src长度) */
	int delength;		  /* dels の長さ (dels长度) */
	int i, j;			  /* 比較 数 (循环计数) */
	int index = 0;		  /* 確保 文字 の index (输出字符索引) */
	char backchar = '\0'; /* 削除する 文字 (要删除的字符) */

	length = strlen(src);
	delength = strlen(dels);

	/*
	 * ループするごとにコピーする (每次循环复制)
	 */
	for (i = 0; i < length; i++) {
		if (src[i] == BACKSLASH) {
			/*
			 * バックスラッシュだ見つたら (若是反斜杠)
			 * 強制読に 置く( BACKSLASH続きと、後の 文字 ) (强制放置，即反斜杠及其后字符)
			 * 進める (推进)
			 */
			src[index++] = src[i++];
			if (i >= length) {
				print("*\n");
				break;
			}
			src[index++] = src[i];
			/*   続きが止まった事にする (视为连续结束) */
			backchar = '\0';
			continue;
		}
		if (src[i] == backchar)
			/*
			 * 削除する文字だ見つたら何もしない。つまり削除になる。 (若是删除字符则不处理，即删除)
			 */
			continue;

		/*
		 *   続きの削除する 文字がなくな見つたので backchar は NULL 文字にする (后续无删除字符则backchar设为NULL)
		 * これがかんじん (这是关键)
		 */
		backchar = '\0';
		for (j = 0; j < delength; j++) {
			if (src[i] == dels[j]) {
				/*
				 * 削除する 文字 だ。 ここで continue しない事により (是删除字符，此处不continue则)
				 * 一 文字は削除しない事が保存できる。 (可确保连续只保留一个)
				 */
				backchar = src[i];
				break;
			}
		}
		src[index++] = src[i];
	}
	src[index++] = '\0';
}

/*----------------------------------------
 * hash値を返す。 文字 にのみ使うこと (返回哈希值，仅用于字符串)
 * これは そのままなのでアルゴリズムはようわからんが (直接用，算法不明但)
 * いいアルゴリズムらしい。 (据说是不错的算法)
 * 引数
 *  s     文字  (字符串)
 * 戻り値
 *  int ハッシュ値 (哈希值)
 *----------------------------------------*/
#define PRIME 211
int hashpjw(char *s)
{
	char *p;
	unsigned int h = 0, g;
	for (p = s; *p; p++) {
		h = (h << 4) + (*p);
		if ((g = h & 0xf0000000) != 0) {
			h = h ^ (g >> 24);
			h = h ^ g;
		}
	}
	return h % PRIME;
}

/*----------------------------------------
 * ローカルにバインドしたソケットを返す( TCP ) (返回绑定到本地的socket)
 * 引数
 *          port        ポート番号 (端口号)
 * 戻り値
 *          -1      失敗 (失败)
 *                      1. socketシステムコールのエラー (1.socket系统调用错误)
 *                      2. bindシステムコールのエラー
 *                      3. listenシステムコールのエラー
 ----------------------------------------*/
int bindlocalhost(int port)
{
	struct sockaddr_in sin; /*ポートアドレス用 (端口地址用)*/
	int sfd;				/*ソケットディスクリプタ (socket描述符)*/
	int rc;					/*ポート結果コード (端口结果码)*/

	/*AF_INET の SOCK_STREAM  (使用AF_INET和SOCK_STREAM) */
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		print("%s\n", strerror(errno));
		return -1;
	}

	if (getReuseaddr()) {
		int sendbuff;
		/* ポートアドレスの設定用 (端口地址设置用) */
		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
				   (char *)&sendbuff, sizeof(sendbuff));
	}

	/*0クリアして値 を入  (清零并赋值)  */
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;

	rc = bind(sfd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
	if (rc == -1) {
		print("%s\n", strerror(errno));
		return -1;
	}

	rc = listen(sfd, 5);
	if (rc == -1) {
		print("%s\n", strerror(errno));
		return -1;
	}
	return sfd;
}

/*----------------------------------------
 * コネクトする。( TCP ) (建立连接)
 * 引数
 *      hostname    接続するホスト
 *                      (xx.xxx.xxx.xxxでもいいしxxx.co.jpとかでもいい) (可用IP或域名)
 *      port        接続するホストのポート
 * 戻り値
 *      ディスクリプタ( -1 の場合はエラー ) (描述符，-1为错误)
 ----------------------------------------*/
int connectHost(char *hostname, unsigned short port)
{
	struct sockaddr_in sock; /*connectの時に呼ばれ (连接时调用)*/
	struct hostent *hoste;	 /*hostnameのhostent*/
	int fd;					 /*コネクトしたソケットディスクリプタ (连接后的socket描述符)*/
	int lr;					 /*ポート結果コード*/

	memset(&sock, 0, sizeof(struct sockaddr_in));
	sock.sin_family = AF_INET;
	sock.sin_port = htons(port);

	/* dot notation 文字を調べる (按点分格式解析) */
	sock.sin_addr.s_addr = inet_addr(hostname);
	if (sock.sin_addr.s_addr == -1) {
		/*
		 *エラー dot notation ではない。したがって 名前の 前解決が 必要だ。 (非点分格式，需要先解析域名)
		 * dnsをひかなければならない場合はひきにいくのもここでやってくれる。 (需要DNS时此处会解析)
		 */
		hoste = gethostbyname(hostname);
		if (hoste == NULL) {
			print("获取主机名: %s\n", hostname);
			return -1;
		}

		memcpy((void *)&sock.sin_addr.s_addr,
			   hoste->h_addr, sizeof(struct in_addr));
	}

	/*ソケットを作る (创建socket)*/
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		print("Cannot Create Socket(%s errno:%d)\n", strerror(errno), errno);
		return -1;
	}
	/*コネクトする (连接)*/
	lr = connect(fd, (struct sockaddr *)&sock, sizeof(struct sockaddr_in));
	if (lr != 0) {
		print("Cannot connect. (%s errno:%d)\n", strerror(errno), errno);
		return -1;
	}

	return fd;
}

/*----------------------------------------
 * バッファの 後 に改行記号があるかどうか。 (缓冲区末尾是否有换行符)
 * 元しにいアルゴリズムに 変更 (改为慢速但简单的算法)
 * 引数
 *  char*   調べる 文字
 * 戻り値
 *    先頭に見つかった改行記号の位置       アクセスした場合にいい値。 (找到的第一个换行符位置)
 *  どういう事かというと、 (具体来说)
 *      existsNewLinwCharacter( "aho\nhoge\n" )
 *  だと、返り値は 3 になる。これに注意する事。 (则返回3，需注意)
 *  -1      見つからない (未找到)
 *
 ----------------------------------------*/
int existsNewLineCharacter(char *line)
{
#if 1
	char *old = line;
	do {
		if (*line == NEWLINE)
			return line - old;
	} while (*(line++));
#else
	int i;					   /*比較 数 (循环计数)*/
	int length = strlen(line); /*調べる 文字 の長さ (被检查字符串长度)*/
	for (i = 0; i < length; i++) {
		if (line[i] == NEWLINE)
			return i;
	}
#endif

	return -1;
}

/*----------------------------------------
 *    文字 中 に 文字が 見つかる場所をかえす。何 番目 かを決める。 (返回字符在字符串中的位置)
 *  nindex( string , c , 1 ) と index( string , c ) は equivalent である。 (与index等价)
 *  numberが 負 または0の場合は返り値は、stringと 同 じである。 (number为负或0时返回string本身)
 *
 *  引数
 *      string  char*   調べる 文字
 *      c       int     調べる 文字
 *      number  int       ある 番 (第几个)
 *  戻り値
 *      一致した 文字のポインター (匹配字符的指针)
 *      NULL    見つからなかった。 (未找到返回NULL)
 ----------------------------------------*/
char *nindex(char *string, int c, int number)
{
	int i; /*比較 数*/
	int num = 0;
	int length = strlen(string); /*調べる 文字 の長さ*/
	if (number <= 0)
		return string;
	for (i = 0; i < length; i++) {
		if (string[i] == c)
			num++;
		if (number == num)
			return &string[i];
	}
	return NULL;
}

BOOL rrd(char *dirname, STRING64 *buf, int bufsize, int *index)
{
	DIR *d;
	char dirn[1024];

	d = opendir(dirname);
	if (d == NULL)
		return FALSE;

	while (1) {
		struct dirent *dent;
		struct stat st;
		dent = readdir(d);
		if (dent == NULL) {
			if (errno == EBADF) {
				errorprint;
				closedir(d);
				return FALSE;
			} else
				/*  successful */
				break;
		}

		/*  . で始まるファイルは 含まない (不包含以.开头的文件)    */
		if (dent->d_name[0] == '.')
			continue;

		snprintf(dirn, sizeof(dirn), "%s/%s", dirname, dent->d_name);
		if (stat(dirn, &st) == -1)
			continue;
		if (S_ISDIR(st.st_mode)) {
			if (rrd(dirn, buf, bufsize, index) == FALSE) {
				closedir(d);
				return FALSE;
			}
		} else {
			if (*index >= bufsize)
				break;

			strcpysafe(buf[*index].string, sizeof(buf[*index].string), dirn);
			(*index)++;
		}
	}
	closedir(d);
	return TRUE;
}

int rgetFileName(char *dirname, STRING64 *string, int size)
{
	int index = 0;
	if (rrd(dirname, string, size, &index) == FALSE)
		return -1;
	else
		return index;
}

BOOL checkStringsUnique(char **strings, int num, int verbose)
{
	int i, j;
	for (i = 0; i < num - 1; i++) {
		for (j = i + 1; j < num; j++) {
			if (!strcmp(strings[i], strings[j])) {
				if (verbose)
					print("Overlapp string is %s\n", strings[i]);
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL PointInRect(RECT *rect, POINT *p)
{
	if (rect->x <= p->x && p->x <= rect->x + rect->width && rect->y <= p->y && p->y <= rect->y + rect->height)
		return TRUE;
	return FALSE;
}

BOOL CoordinateInRect(RECT *rect, int x, int y)
{
	POINT p = {x, y};
	return PointInRect(rect, &p);
}

int clipRect(RECT *rect1, RECT *rect2, RECT *ret)
{
	if (rect1->x > rect2->x + rect2->width - 1
		|| rect2->x > rect1->x + rect1->width - 1
		|| rect1->y > rect2->y + rect2->height - 1
		|| rect2->y > rect1->y + rect1->height - 1)
		return 0;
	ret->x = max(rect1->x, rect2->x);
	ret->y = max(rect1->y, rect2->y);
	ret->width = min(rect1->x + rect1->width, rect2->x + rect2->width) - ret->x;
	ret->height = min(rect1->y + rect1->height, rect2->y + rect2->height) - ret->y;
	return 1;
}

BOOL isstring1or0(char *string)
{
	if (strcasecmp(string, "TRUE") == 0)
		return TRUE;
	if (strcasecmp(string, "FALSE") == 0)
		return FALSE;
	if (strcasecmp(string, "1") == 0)
		return TRUE;
	if (strcasecmp(string, "0") == 0)
		return FALSE;
	if (strcasecmp(string, "ON") == 0)
		return TRUE;
	if (strcasecmp(string, "OFF") == 0)
		return FALSE;
	return FALSE;
}

void easyGetTokenFromString(char *src, int count, char *output, int len)
{
	int i;
	int counter = 0;

	if (len <= 0)
		return;

#define ISSPACETAB(c) ((c) == ' ' || (c) == '\t')

	for (i = 0;; i++) {
		if (src[i] == '\0') {
			output[0] = '\0';
			return;
		}
		if (i > 0 && !ISSPACETAB(src[i - 1]) && !ISSPACETAB(src[i])) {
			continue;
		}

		if (!ISSPACETAB(src[i])) {
			counter++;
			if (counter == count) {
				/* copy it */
				int j;
				for (j = 0; j < len - 1; j++) {
					if (src[i + j] == '\0' || ISSPACETAB(src[i + j])) {
						break;
					}
					output[j] = src[i + j];
				}
				output[j] = '\0';
				return;
			}
		}
	}
}

/*------------------------------------------------------------
 * 線型で 補間をもとめる。 by ringo (线性插值)
 * 計算の用  (计算用)
 *
 * double val1 , val2 : この値のあいだをとる (取两值之间)
 * double d :   補間 (插值系数)
 *
 *
 *  -d<0----- val1 ---0<d<1------- val2 ------d>1----
 *
 *
 ------------------------------------------------------------*/
float linearDiv(float val1, float val2, float d)
{
	return val1 + (val2 - val1) * (d);
}

/*------------------------------------------------------------
 *   文字 のなかから 任意な 文字をlen 文字とりだしてランダムな (从字符串中取len个随机字符)
 *   文字 をつくる。NPCのランダムパスワード表示につかってる (生成随机字符串，用于NPC随机密码显示)
 *
 * char *cand : ここから選ぶ。候補 の 文字。 (候选字符)
 * char *out : 結果 バッファ (输出缓冲区)
 * int len : 結果 の長さ (输出长度)
 ------------------------------------------------------------*/
void makeRandomString(char *cand, char *out, int len)
{
	int i;
	int l = strlen(cand);

	for (i = 0; i < len; i++) {
		out[i] = cand[rand() % l];
	}
	out[i] = '\0';
}

/*------------------------------------------------------------
 * 引数に指定されたファイルがファイルかどうを調べる (检查参数指定的文件是否存在)
 * 引数
 *  filename        char*        ファイル (文件名)
 * 戻り値
 *  ファイル        TRUE(1) (是文件返回TRUE)
 *  ファイルではない  FALSE(0) (不是文件返回FALSE)
 ------------------------------------------------------------*/
BOOL isExistFile(char *filename)
{
	/*  fopen で調べる (用fopen检查)  */
	FILE *fp;
	fp = fopen(filename, "w");
	if (fp) {
		fclose(fp);
		return TRUE;
	} else
		return FALSE;
}
/*------------------------------------------------------------
 * 10進  62進 変換 (十进制转62进制)
 * 引数
 * a        int     元の１０進 (原始十进制数)
 * out      char *  出力 バッファー (输出缓冲区)
 * outlen   int     出力 バッファの 大きさ (缓冲区大小)
 *   戻り値 (返回值)
 *          真      outのアドレス (out的地址)
 *          偽      NULL (假 NULL)
 ------------------------------------------------------------*/
char *cnv10to62(int a, char *out, int outlen)
{
#if 1
	int i, j;
	char base[] = {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
	int tmp[64];
	int src;
	int minus;
	int baselen = sizeof(base) - 1;
	if (a < 0) {
		minus = 1;
		a *= -1;
	} else {
		minus = 0;
	}
	/* special case */
	if (a < baselen) {
		if (minus) {
			*(out) = '-';
			*(out + 1) = base[a];
			*(out + 2) = '\0';
			return (out);
		} else {
			*out = base[a];
			*(out + 1) = '\0';
			return (out);
		}
	}
	src = a;
	for (i = 0; src >= baselen; i++) {
		tmp[i] = src % baselen;
		src /= baselen;
	}
	i--;
	if (minus) {
		*out = '-';
		*(out + 1) = base[src];
		for (j = 2; i >= 0; i--, j++) {
			if (j > outlen - 2)
				return NULL;
			*(out + j) = base[tmp[i]];
		}
	} else {
		*out = base[src];
		for (j = 1; i >= 0; i--, j++) {
			if (j > outlen - 2)
				return NULL;
			*(out + j) = base[tmp[i]];
		}
	}
	*(out + j) = '\0';
	return (out);
#else
/* 古い  (旧版)*/
#define CNV_NUMBER 62
	int i, j;
	char base[] = {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
	int tmp[64];
	int src;
	if (a < 0)
		return (NULL);
	if (a < CNV_NUMBER) {
		*out = base[a];
		*(out + 1) = '\0';
		return (out);
	}
	src = a;
	for (i = 0; src >= CNV_NUMBER; i++) {
		tmp[i] = src % CNV_NUMBER;
		src /= CNV_NUMBER;
	}
	i--;
	*out = base[src];
	for (j = 1; i >= 0; i--, j++) {
		if (j > outlen - 2)
			return NULL;
		*(out + j) = base[tmp[i]];
	}
	*(out + j) = '\0';
	return (out);
#undef CNV_NUMBER
#endif
}
/*
 * 与えられた 配列 の 中で、数字が重 複しているか調べる関数 (检查数组中数字是否重复)
 * -1 は対象外という 特殊な関数 (-1为排除对象的特殊函数)
 *
 * 引数
 * src		*int		調べる元の配列
 * srclen	int			src の 大きさ
 *
 *   戻り値   重複 あり	TRUE
 *				なし	FALSE
 */
BOOL checkRedundancy(int *src, int srclen)
{
	int i, j;
	int ret = FALSE;

	for (i = 0; i < srclen; i++) {
		if (*(src + i) != -1) {
			for (j = i + 1; j < srclen; j++) {
				if (*(src + i) == *(src + j)) {
					ret = TRUE;
					break;
				}
			}
		}
	}
	return ret;
}
void shuffle_ints(int num, int *a, int t)
{
	int i;
	for (i = 0; i < t; i++) {
		int x = random() % num;
		int y = random() % num;
		int s;
		s = a[x];
		a[x] = a[y];
		a[y] = s;
	}
}

static unsigned char BitTable[] = /*	ビットの並び順を逆にする テーブル (反转位顺序的表)	*/
	{
		0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
		0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
		0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
		0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
		0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
		0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
		0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
		0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
		0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
		0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
		0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
		0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
		0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
		0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
		0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
		0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
		0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
		0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
		0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
		0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
		0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
		0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
		0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
		0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
		0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
		0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
		0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
		0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
		0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
		0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
		0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
		0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};
static unsigned short crctab16[] = /*	crc計算 テーブル (crc计算用表)		*/
	{
		0x0000,
		0x1021,
		0x2042,
		0x3063,
		0x4084,
		0x50a5,
		0x60c6,
		0x70e7,
		0x8108,
		0x9129,
		0xa14a,
		0xb16b,
		0xc18c,
		0xd1ad,
		0xe1ce,
		0xf1ef,
		0x1231,
		0x0210,
		0x3273,
		0x2252,
		0x52b5,
		0x4294,
		0x72f7,
		0x62d6,
		0x9339,
		0x8318,
		0xb37b,
		0xa35a,
		0xd3bd,
		0xc39c,
		0xf3ff,
		0xe3de,
		0x2462,
		0x3443,
		0x0420,
		0x1401,
		0x64e6,
		0x74c7,
		0x44a4,
		0x5485,
		0xa56a,
		0xb54b,
		0x8528,
		0x9509,
		0xe5ee,
		0xf5cf,
		0xc5ac,
		0xd58d,
		0x3653,
		0x2672,
		0x1611,
		0x0630,
		0x76d7,
		0x66f6,
		0x5695,
		0x46b4,
		0xb75b,
		0xa77a,
		0x9719,
		0x8738,
		0xf7df,
		0xe7fe,
		0xd79d,
		0xc7bc,
		0x48c4,
		0x58e5,
		0x6886,
		0x78a7,
		0x0840,
		0x1861,
		0x2802,
		0x3823,
		0xc9cc,
		0xd9ed,
		0xe98e,
		0xf9af,
		0x8948,
		0x9969,
		0xa90a,
		0xb92b,
		0x5af5,
		0x4ad4,
		0x7ab7,
		0x6a96,
		0x1a71,
		0x0a50,
		0x3a33,
		0x2a12,
		0xdbfd,
		0xcbdc,
		0xfbbf,
		0xeb9e,
		0x9b79,
		0x8b58,
		0xbb3b,
		0xab1a,
		0x6ca6,
		0x7c87,
		0x4ce4,
		0x5cc5,
		0x2c22,
		0x3c03,
		0x0c60,
		0x1c41,
		0xedae,
		0xfd8f,
		0xcdec,
		0xddcd,
		0xad2a,
		0xbd0b,
		0x8d68,
		0x9d49,
		0x7e97,
		0x6eb6,
		0x5ed5,
		0x4ef4,
		0x3e13,
		0x2e32,
		0x1e51,
		0x0e70,
		0xff9f,
		0xefbe,
		0xdfdd,
		0xcffc,
		0xbf1b,
		0xaf3a,
		0x9f59,
		0x8f78,
		0x9188,
		0x81a9,
		0xb1ca,
		0xa1eb,
		0xd10c,
		0xc12d,
		0xf14e,
		0xe16f,
		0x1080,
		0x00a1,
		0x30c2,
		0x20e3,
		0x5004,
		0x4025,
		0x7046,
		0x6067,
		0x83b9,
		0x9398,
		0xa3fb,
		0xb3da,
		0xc33d,
		0xd31c,
		0xe37f,
		0xf35e,
		0x02b1,
		0x1290,
		0x22f3,
		0x32d2,
		0x4235,
		0x5214,
		0x6277,
		0x7256,
		0xb5ea,
		0xa5cb,
		0x95a8,
		0x8589,
		0xf56e,
		0xe54f,
		0xd52c,
		0xc50d,
		0x34e2,
		0x24c3,
		0x14a0,
		0x0481,
		0x7466,
		0x6447,
		0x5424,
		0x4405,
		0xa7db,
		0xb7fa,
		0x8799,
		0x97b8,
		0xe75f,
		0xf77e,
		0xc71d,
		0xd73c,
		0x26d3,
		0x36f2,
		0x0691,
		0x16b0,
		0x6657,
		0x7676,
		0x4615,
		0x5634,
		0xd94c,
		0xc96d,
		0xf90e,
		0xe92f,
		0x99c8,
		0x89e9,
		0xb98a,
		0xa9ab,
		0x5844,
		0x4865,
		0x7806,
		0x6827,
		0x18c0,
		0x08e1,
		0x3882,
		0x28a3,
		0xcb7d,
		0xdb5c,
		0xeb3f,
		0xfb1e,
		0x8bf9,
		0x9bd8,
		0xabbb,
		0xbb9a,
		0x4a75,
		0x5a54,
		0x6a37,
		0x7a16,
		0x0af1,
		0x1ad0,
		0x2ab3,
		0x3a92,
		0xfd2e,
		0xed0f,
		0xdd6c,
		0xcd4d,
		0xbdaa,
		0xad8b,
		0x9de8,
		0x8dc9,
		0x7c26,
		0x6c07,
		0x5c64,
		0x4c45,
		0x3ca2,
		0x2c83,
		0x1ce0,
		0x0cc1,
		0xef1f,
		0xff3e,
		0xcf5d,
		0xdf7c,
		0xaf9b,
		0xbfba,
		0x8fd9,
		0x9ff8,
		0x6e17,
		0x7e36,
		0x4e55,
		0x5e74,
		0x2e93,
		0x3eb2,
		0x0ed1,
		0x1ef0,
};
unsigned short CheckCRC(unsigned char *p, int size)
{
	unsigned short crc = 0;
	int i;

	for (i = 0; i < size; i++) {
		crc = (crctab16[(crc >> 8) & 0xFF]
			   ^ (crc << 8) ^ BitTable[p[i]]);
	}
	return crc;
}

// Add Code By Shan  2001.06.16
INLINE double time_diff(struct timeval subtrahend,
						struct timeval subtractor)
{
	return ((subtrahend.tv_sec - subtractor.tv_sec)
			+ (subtrahend.tv_usec
			   - subtractor.tv_usec)
				  / (double)1E6);
}
