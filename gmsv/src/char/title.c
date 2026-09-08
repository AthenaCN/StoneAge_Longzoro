#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _REDHAT_V9
#include <errno.h>
#endif

#include "title.h"
#include "char.h"
#include "char_base.h"
#include "item.h"
#include "skill.h"
#include "buf.h"
#include "util.h"
#include "configfile.h"

/*====================惫寞  ====================*/
static int TITLE_IntCheck(int charaindex, int elem, int *data, int flg);
static int TITLE_WorkIntCheck(int charaindex, int elem, int *data, int flg);
static int TITLE_ItemCheck(int charaindex, int elem, int *data, int flg);
static int TITLE_ItemEquipCheck(int charaindex, int elem, int *data, int flg);
static int TITLE_SkillCheck(int charaindex, int elem, int *data, int flg);
static int TITLE_SexCheck(int charaindex, int elem, int *data, int flg);

typedef enum {
	TITLE_FUNCTYPENONE,		/*  関数なし  (无函数)  */
	TITLE_FUNCTYPEUSERFUNC, /* definefunction を呼び出して自分を取得する  (调用definefunction获取自身)
							 * 関数の戻り値は  (函数返回值为)
							 *  int     キャラクタインデックス  (角色索引)
							 *  buf       自分用の文字列  (自己的字符串)
							 *  buflen  その文字列の長さ  (该字符串长度)
							 */
	TITLE_USEFUNCTYPENUM
} TITLE_USEFUNCTYPE;

typedef struct tagTITLE_Table {
	int index; /* データ値の識別番号を取得するため［  (为获取数据识别编号)
				* この  番号でaddtitle呼ばれる  (用此编号调用addtitle)
				*/
	char name[32];
	TITLE_USEFUNCTYPE functype;
	void (*definefunction)(int, char *buf, int buflen);
} TITLE_Table;

/* 呼び出せるパラメータ  (可调用的参数)
 * STR,TGH,MAXMP］ATK,DEF
 * LEVEL,CLASS,SKILL,ITEM,FIREREG,ICEREG,THUNDERREG
 * KANJILV,TALKCNT,WALKCNT,DEADCNT,LOGINCNT,BASEIMAGENUMBER
 * GOLD
 */
typedef struct tagTITLE_Compare {
	char compare[8];
	;
} TITLE_COMPARE;

TITLE_COMPARE TITLE_compare[] = {{"<="}, {">="}, {"<>"}, {">"}, {"<"}, {"="}};

typedef struct tagTITLE_PARAM {
	int element;
	int (*checkfunc)(int charaindex, int elem, int *data, int flg);
	char *paramname;
} TITLE_PARAM;

TITLE_PARAM TITLE_param[] =
	{
		/**** ここから  (从这里开始) ****/
		{-1, TITLE_ItemCheck, "ITEM"},
		{-1, TITLE_ItemEquipCheck, "EQUIPITEM"},
		{-1, NULL, "EQUIPEVENT"},
		/**** それ以外はここではないかも  (其他则可能不在此处) ****/
		{-1, TITLE_SkillCheck, "SKILL"},
		{CHAR_STR, TITLE_IntCheck, "STR"},
		{CHAR_TOUGH, TITLE_IntCheck, "TGH"},
		{CHAR_MAXMP, TITLE_IntCheck, "MAXMP"},
		{CHAR_WORKFIXSTR, TITLE_WorkIntCheck, "ATK"},
		{CHAR_WORKFIXTOUGH, TITLE_WorkIntCheck, "DEF"},
		{CHAR_LV, TITLE_IntCheck, "LEVEL"},
		{CHAR_TALKCOUNT, TITLE_IntCheck, "TALKCNT"},
		{CHAR_WALKCOUNT, TITLE_IntCheck, "WALKCNT"},
		{CHAR_DEADCOUNT, TITLE_IntCheck, "DEADCNT"},
		{CHAR_LOGINCOUNT, TITLE_IntCheck, "LOGINCNT"},
		{CHAR_BASEBASEIMAGENUMBER, TITLE_IntCheck, "BASEIMAGENUMBER"},
		{CHAR_GOLD, TITLE_IntCheck, "GOLD"},
		{-1, TITLE_SexCheck, "SEX"},
};

#define TITLE_PARAMSIZE 20
typedef struct tagTITLE_configTable {
	int paramindex[20];				/* TITLE_paramの検索  (TITLE_param检索) */
	int param[20][TITLE_PARAMSIZE]; /*   値  (值) */
	int compareflg[20];				/* ><=判断するか  (是否用><=判断) */
	int title;
	BOOL equipcheckflg; /*
						 *  アイテム系インターフェースならこのキャラが持っているか  (若是物品类则检查是否持有)
						 *  手に入れていない  (未入手)
						 */
} TITLE_CONFIGTABLE;

typedef struct tagTITLE_configbuf {
	int title;
	int flg;
} TITLE_CONFIGBUF;

static TITLE_CONFIGTABLE *TITLE_ConfigTable;
static TITLE_Table *TITLE_table;
static TITLE_CONFIGBUF *TITLE_configbuf;
static int TITLE_titlenum;
static int TITLE_titlecfgnum;

/*------------------------------------------------------------
  * index  番号からTITLE_tableの検索を返す  (从编号查TITLE_table)
 ------------------------------------------------------------*/
int TITLE_getTitleIndex(int index) {
	int i;
	if (index < 0)
		return -1;
	for (i = 0; i < TITLE_titlenum; i++) {
		if (TITLE_table[i].index == index) {
			return (i);
		}
	}
	return -1;
}

/*  文字列のサイズ  (字符串大小)  */
#define TITLESTRINGBUFSIZ 256
	/*  スキルインターフェースで使うテンポラリデータ用の文字列  (技能接口用的临时数据字符串)    */
	static char TITLE_statusStringBuffer[TITLESTRINGBUFSIZ];
/*------------------------------------------------------------
  * スキルインターフェースで使うタイトルのデータを取得  (获取技能接口用的称号数据)
 * 戻り値  (返回值)
  *  title       Title*      テンポラリ  (临时)
  *  charaindex  int         このタイトルを持っているキャラのインデックス  (持有该称号的角色索引)
 * 戻り値  (返回值)
 *  char*
 ------------------------------------------------------------*/
char *TITLE_makeTitleStatusString(int charaindex, int havetitleindex) {
	int attach;
	int index;
	/*  関数用のインデックスからデータを取得する  (从函数用索引获取数据)  */
	index = CHAR_getCharHaveTitle(charaindex, havetitleindex);
#if 0
	if( TITLE_CHECKTABLEINDEX( index ) == FALSE ){
		TITLE_statusStringBuffer[0] = '\0';
		return TITLE_statusStringBuffer;
	}
#endif
	attach = TITLE_getTitleIndex(index);
	if (attach == -1) {
		TITLE_statusStringBuffer[0] = '\0';
		return TITLE_statusStringBuffer;
	}
	switch (TITLE_table[attach].functype) {
	case TITLE_FUNCTYPENONE:
		snprintf(TITLE_statusStringBuffer,
				 sizeof(TITLE_statusStringBuffer), "%s",
				 TITLE_table[attach].name);
		break;

	case TITLE_FUNCTYPEUSERFUNC: {
		char string[256] = {""};
		void (*function)(int, char *buf, int buflen);
		function = TITLE_table[attach].definefunction;
		if (function)
			function(charaindex, string, sizeof(string));

		strcpysafe(TITLE_statusStringBuffer,
				   sizeof(TITLE_statusStringBuffer), string);
	} break;
	default:
		TITLE_statusStringBuffer[0] = '\0';
		return TITLE_statusStringBuffer;
		break;
	}
	return TITLE_statusStringBuffer;
}

/*------------------------------------------------------------
/*------------------------------------------------------------
 * なしタイトルのデータ値を返す  (返回无称号的数据值)
  *  戻り値  (返回值)
 *  戻り値  (返回值)
 * 戻り値  (返回值)
 ------------------------------------------------------------*/
char *TITLE_makeSkillFalseString(void) {
	TITLE_statusStringBuffer[0] = '\0';
	return TITLE_statusStringBuffer;
}

/*------------------------------------------------------------
 * 指定された番号のタイトルを登録する［既に持っていると｝登録しない  (注册指定编号称号，已拥有则不注册)
 * 戻り値  (返回值)
  *  charaindex      int     キャラクタインデックス  (角色索引)
  *  titleindex      int     タイトルインデックス  (称号索引)
 * 戻り値  (返回值)
  *  登録したindex
  *  登録しなかった      FALSE(0)  (未注册)
 ------------------------------------------------------------*/
BOOL TITLE_addtitle(int charaindex, int titleindex) {
	int i;
	int firstfindempty = -1;

	if (CHAR_CHECKINDEX(charaindex) == FALSE)
		return FALSE;
	/*if( TITLE_CHECKTABLEINDEX(titleindex) == FALSE )return FALSE;*/
	if (TITLE_getTitleIndex(titleindex) == -1)
		return FALSE;

	for (i = 0; i < CHAR_TITLEMAXHAVE; i++) {
		if (CHAR_getCharHaveTitle(charaindex, i) == titleindex)
			/*  ここで持っているので入れない  (已拥有故不加入)  */
			return FALSE;
		if (firstfindempty == -1 && CHAR_getCharHaveTitle(charaindex, i) == -1) {
			firstfindempty = i;
		}
	}
	return CHAR_setCharHaveTitle(charaindex, firstfindempty, titleindex);
}

/*------------------------------------------------------------
 * 指定された番号のタイトルを外す［持っていなければ外さない［  (移除指定编号称号，未拥有则不移除)
  *   装備していれば外す［  (若装备中则移除)
 * 戻り値  (返回值)
  *  charaindex      int     キャラクタインデックス  (角色索引)
  *  titleindex      int     タイトルインデックス  (称号索引)
 * 戻り値  (返回值)
  *  外した            TRUE(1)  (已移除)
  *  外さなかった      FALSE(0)  (未移除)
 ------------------------------------------------------------*/
BOOL TITLE_deltitle(int charaindex, int titleindex) {
	int i;
	BOOL del = FALSE;
	int index;
	if (CHAR_CHECKINDEX(charaindex) == FALSE)
		return FALSE;
	/*if( TITLE_CHECKTABLEINDEX(titleindex) == FALSE )return FALSE;*/
	index = TITLE_getTitleIndex(titleindex);
	if (index == -1)
		return FALSE;

	for (i = 0; i < CHAR_TITLEMAXHAVE; i++)
		if (CHAR_getCharHaveTitle(charaindex, i) == titleindex) {
			/*  登録が呼び出しているので｝一緒に消しておく  (注册正调用，一并清除)    */
			if (CHAR_getInt(charaindex, CHAR_INDEXOFEQTITLE) == i) {
				CHAR_setInt(charaindex, CHAR_INDEXOFEQTITLE, -1);
			}
			/*    持っているので消す  (已拥有则清除)  */
			CHAR_setCharHaveTitle(charaindex, i, -1);

			del = TRUE;
		}

	return del;
}
/*------------------------------------------------------------
 * タイトルの初期化をする［  (初始化称号)
 * 戻り値  (返回值)
 *  filename        char*       データファイル  (数据文件)
 * 戻り値  (返回值)
 *  成功      TRUE(1)  (成功)
 *  失敗      FALSE(0)  (失败)
 *------------------------------------------------------------*/
BOOL TITLE_initTitleName(char *filename) {
	FILE *f;
	char line[256];
	int linenum = 0;
	int title_readlen = 0;

	f = fopen(filename, "r");
	if (f == NULL) {
		errorprint;
		return FALSE;
	}

	TITLE_titlenum = 0;

	/*  まず有効な行が何行あるかどうか調べる  (先检查有多少有效行)    */
	while (fgets(line, sizeof(line), f)) {
		linenum++;
		if (line[0] == '#')
			continue; /* comment */
		if (line[0] == '\n')
			continue; /* none    */
		chomp(line);

		TITLE_titlenum++;
	}

	if (fseek(f, 0, SEEK_SET) == -1) {
		fprint("寻找错误\n");
		fclose(f);
		return FALSE;
	}

	TITLE_table = allocateMemory(sizeof(struct tagTITLE_Table) * TITLE_titlenum);
	if (TITLE_table == NULL) {
		fprint("无法分配内存 %d\n",
			   sizeof(TITLE_table) * TITLE_titlenum);
		fclose(f);
		return FALSE;
	}
	/* 初期化  (初始化) */
	{
		int i;
		for (i = 0; i < TITLE_titlenum; i++) {
			TITLE_table[i].index = -1;
			TITLE_table[i].name[0] = '\0';
			TITLE_table[i].functype = TITLE_FUNCTYPENONE;
			TITLE_table[i].definefunction = NULL;
		}
	}

	/*  もう一度見る  (再看一遍)    */
	linenum = 0;
	while (fgets(line, sizeof(line), f)) {
		linenum++;
		if (line[0] == '#')
			continue; /* comment */
		if (line[0] == '\n')
			continue; /* none    */
		chomp(line);

		/*  数を整える  (整理数量)    */
		/*  まず tab を " " に置き換える  (先把tab替换为空格)    */
		replaceString(line, '\t', ' ');
		/* 先のスペースを取る  (去除前导空格)［*/
		{
			int i;
			char buf[256];
			for (i = 0; i < strlen(line); i++) {
				if (line[i] != ' ') {
					break;
				}
				strcpy(buf, &line[i]);
			}
			if (i != 0) {
				strcpy(line, buf);
			}
		}
		{
			char token[256];
			int ret;

			/*  ひとつ目のトークンを取る  (取第一个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 1, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			TITLE_table[title_readlen].index = atoi(token);

			/*  2つ目のトークンを取る  (取第2个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 2, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			if (strlen(token) > sizeof(TITLE_table[title_readlen].name) - 1) {
				fprint("警告! 头衔名称结束文件:%s 第%d行\n",
					   filename, linenum);
			}
			strcpysafe(TITLE_table[title_readlen].name,
					   sizeof(TITLE_table[title_readlen].name),
					   token);

			title_readlen++;
		}
	}
	fclose(f);

	TITLE_titlenum = title_readlen;

	print("有效头衔名称数是 %d...", TITLE_titlenum);

#ifdef DEBUG

	{
		int i;
		for (i = 0; i < TITLE_titlenum; i++)
			print("头衔索引[%d] 名称[%s] \n",
				  TITLE_table[i].index,
				  TITLE_table[i].name);
	}
#endif
	return TRUE;
}
/*------------------------------------------------------------
 * タイトルの再初期化をする［  (重新初始化称号)
 * 戻り値  (返回值)
 *  filename        char*       データファイル  (数据文件)
 * 戻り値  (返回值)
 *  成功      TRUE(1)  (成功)
 *  失敗      FALSE(0)  (失败)
 *------------------------------------------------------------*/
BOOL TITLE_reinitTitleName(void) {
	freeMemory(TITLE_table);
	return (TITLE_initTitleName(getTitleNamefile()));
}
/*------------------------------------------------------------
 * タイトルデータの初期化をする［  (初始化称号数据)
 * 戻り値  (返回值)
 *  filename        char*       データファイル  (数据文件)
 * 戻り値  (返回值)
 *  成功      TRUE(1)  (成功)
 *  失敗      FALSE(0)  (失败)
 *------------------------------------------------------------*/
static void TITLE_initTitleData(int array) {
	int j, k;
	for (j = 0; j < arraysizeof(TITLE_ConfigTable[array].paramindex); j++) {
		for (k = 0; k < TITLE_PARAMSIZE; k++) {
			TITLE_ConfigTable[array].param[j][k] = -1;
		}
		TITLE_ConfigTable[array].paramindex[j] = -1;
		TITLE_ConfigTable[array].compareflg[j] = -1;
	}
	TITLE_ConfigTable[array].title = -1;
	TITLE_ConfigTable[array].equipcheckflg = FALSE;
}
/*------------------------------------------------------------
 *   設定データを取り出して1行に変換する［読みやすくするためだ［  (取出配置数据转为一行，便于阅读)
 *------------------------------------------------------------*/
static int TITLE_getConfigOneLine(FILE *fp, char *line, int linelen) {
	char buf[1024];
	int startflg = FALSE;
	int linenum = 0;
	line[0] = '\0';

	while (fgets(buf, sizeof(buf), fp)) {
		linenum++;
		if (buf[0] == '#')
			continue; /* comment */
		if (buf[0] == '\n')
			continue; /* none    */
					  /*  数を整える  (整理数量)    */
					  /*  まず tab を " " に置き換える  (先把tab替换为空格)    */
		replaceString(buf, '\t', ' ');
		/* スペース削除  (删除空格) */
		deleteCharFromString(buf, " ");

		if (buf[0] == '{') {
			if (startflg == TRUE) {
				print("titleconfig:明明没有关闭「{」却出现了: %d \n", linenum);
				/* } */
				return -1;
			}
			startflg = TRUE;
		} else if (buf[0] == '}') {
			if (startflg == FALSE) {
				print("titleconfig:明明没有关闭「{」却出现了: %d \n", linenum);
				/* } */
				return -1;
			}
			return 1;
		} else {
			/* "{"の中にある時 }  ({内时) */
			if (startflg == TRUE) {
				if (strlen(line) != 0) {
					if (line[strlen(line) - 1] != ',') {
						strcatsafe(line, linelen, ",");
					}
				}
				/* 1行に取り出して中へ  (取出并入1行) */
				chompex(buf);
				strcatsafe(line, linelen, buf);
			}
			/*   有効な行が"{"ではない時はそのまま1行で返す  (非{的有效行则直接返回1行) }*/
			else {
				chompex(buf);
				strcatsafe(line, linelen, buf);
				return 1;
			}
		}
	}
	/* ここに入る場合はEOF もしくは行がなければ終了  (此处为EOF或无行则结束)   */
	return 0;
}
static int TITLE_getParamData(int readarray, int array, char *src) {
	int cnt;
	int ret;
	char wk[64];

	for (cnt = 1;; cnt++) {
		ret = getStringFromIndexWithDelim(src, "|", cnt, wk, sizeof(wk));
		if (ret) {
			TITLE_ConfigTable[readarray].param[array][cnt - 1] = atoi(wk);
		} else {
			break;
		}
	}
	if (cnt == 1) {
		return FALSE;
	}
	return TRUE;
}

/*------------------------------------------------------------
 * タイトルデータの初期化をする［  (初始化称号数据)
 * 戻り値  (返回值)
 *  filename        char*       データファイル  (数据文件)
 * 戻り値  (返回值)
 *  成功      TRUE(1)  (成功)
 *  失敗      FALSE(0)  (失败)
 *------------------------------------------------------------*/
BOOL TITLE_initTitleConfig(char *filename) {
	FILE *f;
	char line[1024];
	int linenum = 0;
	int titlecfg_readlen = 0;

	f = fopen(filename, "r");
	if (f == NULL) {
		errorprint;
		return FALSE;
	}

	TITLE_titlecfgnum = 0;

	/*  まず有効な行が何行あるかどうか調べる  (先检查有多少有效行)    */
	/*while( fgets( line, sizeof( line ), f ) ){}*/
	while (1) {
		int rc;
		rc = TITLE_getConfigOneLine(f, line, sizeof(line));
		if (rc == 0)
			break;
		if (rc == -1)
			continue;

		TITLE_titlecfgnum++;
	}

	if (fseek(f, 0, SEEK_SET) == -1) {
		fprint("Seek Error\n");
		fclose(f);
		return FALSE;
	}

	TITLE_ConfigTable = allocateMemory(sizeof(struct tagTITLE_configTable) * TITLE_titlecfgnum);
	if (TITLE_ConfigTable == NULL) {
		fprint("Can't allocate Memory %d\n",
			   sizeof(TITLE_ConfigTable) * TITLE_titlecfgnum);
		fclose(f);
		return FALSE;
	}
	/* 初期化  (初始化) */
	{
		int i;
		for (i = 0; i < TITLE_titlecfgnum; i++) {
			TITLE_initTitleData(i);
		}
	}
	TITLE_configbuf = allocateMemory(sizeof(TITLE_CONFIGBUF) * TITLE_titlecfgnum);
	if (TITLE_configbuf == NULL) {
		fprint("Can't allocate Memory %d\n",
			   sizeof(TITLE_CONFIGBUF) * TITLE_titlecfgnum);
		fclose(f);
		return FALSE;
	}

	/*  もう一度見る  (再看一遍)    */
	linenum = 0;
	/*while( fgets( line, sizeof( line ), f ) ){}*/
	while (1) {
		int rc;
		rc = TITLE_getConfigOneLine(f, line, sizeof(line));
		if (rc == 0)
			break;
		if (rc == -1)
			continue;

		linenum++;

		{
			char token[256];
			int ret;
			int i, j;
			int comppos;
			BOOL errflg = FALSE;
			for (i = 1;; i++) {
				/*  トークンを取る  (取字段)    */
				ret = getStringFromIndexWithDelim(line, ",", i, token,
												  sizeof(token));
				if (ret == FALSE) {
					break;
				}
				/*     を大文字にする  (转为大写) */
				for (j = 0; j < strlen(token); j++) {
					token[j] = toupper(token[j]);
				}
				/* タイトルデータを調べる  (查称号数据) */
				if (strncmp("TITLE", token, 5) == 0) {
					char buf[64];
					ret = getStringFromIndexWithDelim(token, "=", 2, buf,
													  sizeof(buf));
					if (ret == FALSE) {
						fprint("文件语法错误:%s 第%d行\n", filename, linenum);
						TITLE_initTitleData(titlecfg_readlen);
						errflg = TRUE;
						break;
					}
					TITLE_ConfigTable[titlecfg_readlen].title = atoi(buf);
				} else {
					/* パラメータデータを読む  (读取参数数据) */
					for (j = 0; j < arraysizeof(TITLE_param); j++) {
						if (strncmp(TITLE_param[j].paramname,
									token,
									strlen(TITLE_param[j].paramname)) == 0) {
							TITLE_ConfigTable[titlecfg_readlen].paramindex[i - 1] = j;
							/* アイテムインターフェースでタイトルをチェックするフラグを立てる  (置物品接口检查称号的旗标) */
							if (j == 0 || j == 1 || j == 2) {
								TITLE_ConfigTable[titlecfg_readlen].equipcheckflg = TRUE;
							}
							break;
						}
					}
					/* パラメータが見つからないとエラー  (找不到参数则报错) */
					if (j == arraysizeof(TITLE_param)) {
						fprint("文件语法错误:%s 第%d行\n",
							   filename, linenum);
						TITLE_initTitleData(titlecfg_readlen);
						errflg = TRUE;
						break;
					}
					/* 上位］下位の比較記号があるか調べる  (检查是否有比较符号) */
					comppos = charInclude(token, "<>=");
					if (comppos == -1) {
						fprint("文件语法错误:%s 第%d行\n", filename, linenum);
						TITLE_initTitleData(titlecfg_readlen);
						errflg = TRUE;
						break;
						;
					}
					/* 上位］下位の比較を複数使う場合  (同时使用多种比较) */
					if (charInclude(&token[comppos + 1], "<>=") != -1) {
						/* 優先順位の高いものを読む  (读取高优先级) */
						for (j = 0; j < 3; j++) {
							if (memcmp(TITLE_compare[j].compare, &token[comppos], 2) == 0) {
								break;
							}
						}
						if (j == 3) {
							fprint("文件语法错误:%s 第%d行\n", filename, linenum);
							TITLE_initTitleData(titlecfg_readlen);
							errflg = TRUE;
							break;
						} else {
							ret = TITLE_getParamData(titlecfg_readlen, i - 1, &token[comppos + 2]);
							if (!ret) {
								fprint("文件语法错误:%s 第%d行\n", filename, linenum);
								TITLE_initTitleData(titlecfg_readlen);
								errflg = TRUE;
								break;
							}

							/*TITLE_ConfigTable[titlecfg_readlen].param[i-1]
									= atoi( &token[comppos+2]);*/
							TITLE_ConfigTable[titlecfg_readlen].compareflg[i - 1] = j;
						}
					} else {
						ret = TITLE_getParamData(titlecfg_readlen, i - 1, &token[comppos + 1]);
						if (!ret) {
							fprint("文件语法错误:%s 第%d行\n", filename, linenum);
							TITLE_initTitleData(titlecfg_readlen);
							errflg = TRUE;
							break;
						}
						/*TITLE_ConfigTable[titlecfg_readlen].param[i-1]
								= atoi( &token[comppos+1]);*/
						/* 優先順位の高いものを読む  (读取高优先级) */
						for (j = 3; j < 6; j++) {
							if (memcmp(TITLE_compare[j].compare, &token[comppos], 1) == 0) {
								TITLE_ConfigTable[titlecfg_readlen].compareflg[i - 1] = j;
								break;
							}
						}
					}
				}
			}
			/* タイトルデータが無い or 優先しない時のエラー  (无称号数据或非优先时出错) */
			if (errflg || TITLE_ConfigTable[titlecfg_readlen].title == -1) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				TITLE_initTitleData(titlecfg_readlen);
			} else {
				titlecfg_readlen++;
			}
		}
	}
	fclose(f);

	TITLE_titlecfgnum = titlecfg_readlen;

	print("有效头衔配置数是 %d...", TITLE_titlecfgnum);

#if 0

	{
		int i, j;
		for( i=0; i < TITLE_titlecfgnum ; i++ ) {
			print( "头衔队列[%d]\n", i);
			for( j = 0; 
				j < arraysizeof( TITLE_ConfigTable[i].param) && 
				TITLE_ConfigTable[i].param[j] != -1 ; 
				j ++ ) 
			{
			   print( "name[%s] data[%d] flg[%s] ",
					   TITLE_param[TITLE_ConfigTable[i].paramindex[j]].paramname,
					   TITLE_ConfigTable[i].param[j],
					   TITLE_compare[TITLE_ConfigTable[i].compareflg[j]].compare
				   );
				if( j %2 ==0 ) print( "\n");
			}
			print( "\nTitleindex [%d] ",TITLE_ConfigTable[i].title);
			print( "equipcheckflg [%d]\n",TITLE_ConfigTable[i].equipcheckflg);
		}
	}
#endif
	return TRUE;
}
/*------------------------------------------------------------
 * タイトルデータに該当するか調べてタイトルを与える［  (检查是否符合称号数据并赋予称号)
 * 戻り値  (返回值)
 *  charaindex        int   キャラクタインデックス  (角色索引)
 *  mode              BOOL  TRUE:item=を持ってるか調べる FALSE:装備  (TRUE检查持有/FLASE检查装备)
 * 戻り値  (返回值)
 *  TRUE: タイトルに  合致  した［  (TRUE:符合称号)
 *  FALSE:まだ  いない［  (FALSE:不符合)
 *------------------------------------------------------------*/
static BOOL TITLE_TitleCheck_Main(int charaindex, BOOL mode, int *addcnt, int *delcnt) {
	int i, j, k, ret;

	/* 初期化  (初始化) */
	{
		int i;
		for (i = 0; i < TITLE_titlecfgnum && TITLE_configbuf[i].title != -1; i++) {
			TITLE_configbuf[i].title = -1;
			TITLE_configbuf[i].flg = 0;
		}
	}
	*addcnt = 0;
	*delcnt = 0;

	for (i = 0; i < TITLE_titlecfgnum; i++) {
		int rc = TRUE;
		int title = TITLE_ConfigTable[i].title;
		if (mode == TRUE) {
			if (TITLE_ConfigTable[i].equipcheckflg != TRUE) {
				continue;
			}
		}
		for (j = 0;
			 j < arraysizeof(TITLE_ConfigTable[i].param) &&
			 TITLE_ConfigTable[i].param[j][0] != -1;
			 j++) {
			int index = TITLE_ConfigTable[i].paramindex[j];
			if (TITLE_param[index].checkfunc != NULL) {
				rc = TITLE_param[index].checkfunc(
					charaindex,
					TITLE_param[index].element,
					TITLE_ConfigTable[i].param[j],
					TITLE_ConfigTable[i].compareflg[j]);
				if (rc != TRUE)
					break;
			}
		}
		/*   条件に  該当タイトルを割り当てた時のために］  (符合条件时分配称号之用)
		 * メッセージ文字列に保存した予定からadd,delitleする  (从保存的字符串计划中执行add/delitle)
		 */
		for (k = 0; k < TITLE_titlecfgnum; k++) {
			if (TITLE_configbuf[k].title == title) {
				if (rc) {
					TITLE_configbuf[k].flg = 1;
				}
				break;
			} else if (TITLE_configbuf[k].title == -1) {
				TITLE_configbuf[k].title = title;
				TITLE_configbuf[k].flg = rc ? 1 : -1;
				break;
			}
		}
	}
	ret = FALSE;
	for (i = 0; i < TITLE_titlecfgnum && TITLE_configbuf[i].title != -1; i++) {
		if (TITLE_configbuf[i].flg == -1) {
			*delcnt += TITLE_deltitle(charaindex, TITLE_configbuf[i].title);
		} else {
			*addcnt += TITLE_addtitle(charaindex, TITLE_configbuf[i].title) ? 1 : 0;
		}
	}
	if (*delcnt > 0 || *addcnt > 0) {
		ret = TRUE;
	}
	return ret;
}
/*------------------------------------------------------------
 * タイトルデータに該当するか調べてタイトルを与える［  (检查是否符合称号数据并赋予称号)
 * 戻り値  (返回值)
 *  charaindex        int   キャラクタインデックス  (角色索引)
 *  mode              BOOL  TRUE:item=を持ってるか調べる FALSE:装備  (TRUE检查持有/FLASE检查装备)
 * 戻り値  (返回值)
 *  TRUE: タイトルに  合致  した［  (TRUE:符合称号)
 *  FALSE:まだ  いない［  (FALSE:不符合)
 *------------------------------------------------------------*/
BOOL TITLE_TitleCheck(int charaindex, BOOL mode) {
#define TITLE_MSGUNIT1 "TSU"
#define TITLE_MSGUNIT2 "KO"
	int addcnt, delcnt;
	BOOL rc;
	char msgbuf[64];
	rc = TITLE_TitleCheck_Main(charaindex, mode, &addcnt, &delcnt);
	if (rc) {
		if (delcnt > 0) {
			snprintf(msgbuf, sizeof(msgbuf),
					 "失去%d%s 称号！", delcnt,
					 delcnt < 10 ? TITLE_MSGUNIT1 : TITLE_MSGUNIT2);
			CHAR_talkToCli(charaindex, -1, msgbuf, CHAR_COLORYELLOW);
		}
		if (addcnt > 0) {
			snprintf(msgbuf, sizeof(msgbuf),
					 "获得%d%s 称号！", addcnt,
					 addcnt < 10 ? TITLE_MSGUNIT1 : TITLE_MSGUNIT2);
			CHAR_talkToCli(charaindex, -1, msgbuf, CHAR_COLORYELLOW);
		}
	}
	return rc;
}
/*------------------------------------------------------------
 * タイトルデータに該当するか調べてタイトルを与える［  (检查是否符合称号数据并赋予称号)
 * 戻り値  (返回值)
 *  charaindex        int   キャラクタインデックス  (角色索引)
 *  mode              BOOL  TRUE:item=を持ってるか調べる FALSE:装備  (TRUE检查持有/FLASE检查装备)
 * 戻り値  (返回值)
 *  TRUE: タイトルに  合致  した［  (TRUE:符合称号)
 *  FALSE:まだ  いない［  (FALSE:不符合)
 *------------------------------------------------------------*/
BOOL TITLE_TitleCheck_Nomsg(int charaindex, BOOL mode, int *addcnt, int *delcnt) {
	return (TITLE_TitleCheck_Main(charaindex, mode, addcnt, delcnt));
}

static int TITLE_IntCheck(int charaindex, int elem, int *data, int flg) {
	int rc = FALSE;
	int i;
	for (i = 0; i < TITLE_PARAMSIZE && *(data + i) != -1; i++) {
		switch (flg) {
		case 0: /* "<=" */
			if (CHAR_getInt(charaindex, elem) <= *(data + i))
				rc = TRUE;
			break;
		case 1: /* ">=" */
			if (CHAR_getInt(charaindex, elem) >= *(data + i))
				rc = TRUE;
			break;
		case 2: /* "<>" */
			if (CHAR_getInt(charaindex, elem) != *(data + i))
				rc = TRUE;
			break;
		case 3: /* ">" */
			if (CHAR_getInt(charaindex, elem) > *(data + i))
				rc = TRUE;
			break;
		case 4: /* "<"  */
			if (CHAR_getInt(charaindex, elem) < *(data + i))
				rc = TRUE;
			break;
		case 5: /* "=" */
			if (CHAR_getInt(charaindex, elem) == *(data + i))
				rc = TRUE;
			break;
		default:
			rc = FALSE;
			break;
		}
		if (rc)
			break;
	}
	return rc;
}
static int TITLE_WorkIntCheck(int charaindex, int elem, int *data, int flg) {
	int rc = FALSE;
	int i;
	for (i = 0; i < TITLE_PARAMSIZE && *(data + i) != -1; i++) {
		switch (flg) {
		case 0: /* "<=" */
			if (CHAR_getWorkInt(charaindex, elem) <= *(data + i))
				rc = TRUE;
			break;
		case 1: /* ">=" */
			if (CHAR_getWorkInt(charaindex, elem) >= *(data + i))
				rc = TRUE;
			break;
		case 2: /* "<>" */
			if (CHAR_getWorkInt(charaindex, elem) != *(data + i))
				rc = TRUE;
			break;
		case 3: /* ">" */
			if (CHAR_getWorkInt(charaindex, elem) > *(data + i))
				rc = TRUE;
			break;
		case 4: /* "<"  */
			if (CHAR_getWorkInt(charaindex, elem) < *(data + i))
				rc = TRUE;
			break;
		case 5: /* "=" */
			if (CHAR_getWorkInt(charaindex, elem) == *(data + i))
				rc = TRUE;
			break;
		default:
			rc = FALSE;
			break;
		}
		if (rc)
			break;
	}
	return rc;
}
/* --------------------------------------
 * 指定されたdataで持っているアイテムを 確認する［  (用指定数据确认所持物品)
 * -------------------------------------*/
static int TITLE_ItemCheckMain(int charaindex, int itemhaveindex, int *data, int flg) {
	int i, j;
	int rc = FALSE;

	if (flg == 2)
		rc = TRUE;

	for (j = 0; j < TITLE_PARAMSIZE && *(data + j) != -1; j++) {
		for (i = 0; i < itemhaveindex; i++) {
			int itemindex = CHAR_getItemIndex(charaindex, i);
			if (ITEM_CHECKINDEX(itemindex)) {
				switch (flg) {
				case 0: /* "<=" */
					if (ITEM_getInt(itemindex, ITEM_ID) <= *(data + j))
						rc = TRUE;
					break;
				case 1: /* ">=" */
					if (ITEM_getInt(itemindex, ITEM_ID) >= *(data + j))
						rc = TRUE;
					break;
				case 3: /* ">" */
					if (ITEM_getInt(itemindex, ITEM_ID) > *(data + j))
						rc = TRUE;
					break;
				case 4: /* "<"  */
					if (ITEM_getInt(itemindex, ITEM_ID) < *(data + j))
						rc = TRUE;
					break;
				case 5: /* "=" */
					if (ITEM_getInt(itemindex, ITEM_ID) == *(data + j))
						rc = TRUE;
					break;
				/* これを複数 持っていても［  (即使拥有多个)
				 * アイテム数が 増えても持っていなければ判定する［  (数量增加但未拥有则判定)
				 * それ以外は 持っている物を探してチェック  (否则查找所持物检查)
				 */
				case 2: /* "<>" */
					if (ITEM_getInt(itemindex, ITEM_ID) == *(data + j))
						rc = FALSE;
					break;
				default:
					rc = FALSE;
					break;
				}
				if (rc && flg != 2)
					break;
			}
		}
		if (rc)
			break;
	}
	return rc;
}

static int TITLE_ItemCheck(int charaindex, int elem, int *data, int flg) {
	return (TITLE_ItemCheckMain(charaindex, CHAR_MAXITEMHAVE, data, flg));
}

static int TITLE_ItemEquipCheck(int charaindex, int elem, int *data, int flg) {
	return (TITLE_ItemCheckMain(charaindex, CHAR_EQUIPPLACENUM, data, flg));
}

static int TITLE_SkillCheck(int charaindex, int elem, int *data, int flg) {
	int i;
	int rc = FALSE;
	CHAR_HaveSkill *hskill;

	for (i = 0; i < CHAR_SKILLMAXHAVE; i++) {
		hskill = CHAR_getCharHaveSkill(charaindex, i);
		if (hskill != NULL && hskill->use == TRUE) {

			if (*(data + 1) != -2) {
				if (SKILL_getInt(&hskill->skill, SKILL_IDENTITY) == *data) {
					rc = TRUE;
					break;
				}
			}

			else {
				if (SKILL_getInt(&hskill->skill, SKILL_IDENTITY) == *data) {
					rc = FALSE;
					break;
				} else {
					rc = TRUE;
				}
			}
		}
	}
	if (rc) {

		if (*(data + 1) != -1 && *(data + 1) != -2) {
			rc = FALSE;
			switch (flg) {
			case 0: /* "<=" */
				if (SKILL_getInt(&hskill->skill, SKILL_LEVEL) <= *(data + 1))
					rc = TRUE;
				break;
			case 1: /* ">=" */
				if (SKILL_getInt(&hskill->skill, SKILL_LEVEL) >= *(data + 1))
					rc = TRUE;
				break;
			case 2: /* "<>" */
				if (SKILL_getInt(&hskill->skill, SKILL_LEVEL) != *(data + 1))
					rc = TRUE;
				break;
			case 3: /* ">" */
				if (SKILL_getInt(&hskill->skill, SKILL_LEVEL) > *(data + 1))
					rc = TRUE;
				break;
			case 4: /* "<"  */
				if (SKILL_getInt(&hskill->skill, SKILL_LEVEL) < *(data + 1))
					rc = TRUE;
				break;
			case 5: /* "=" */
				if (SKILL_getInt(&hskill->skill, SKILL_LEVEL) == *(data + 1))
					rc = TRUE;
				break;
			default:
				rc = FALSE;
				break;
			}
		}
	}
	return rc;
}
/* --------------------------------------
 * 男キャラか女キャラかチェックする［  (检查男角色或女角色)
 * -------------------------------------*/
static int TITLE_SexCheck(int charaindex, int elem, int *data, int flg) {
	int i, j;
	struct {
		int sex[25];
	} s_sex[] =
		{{{30008, 30009, 30010, 30011, 30012, 30013, 30014, 30015,
		   30020, 30021, 30022, 30023, 30024, 30025, 30026, 30027,
		   30028, 30029, 30030, 30031, 30032, 30033, 30034, 30035, -1}},
		 {{30000, 30001, 30002, 30003, 30004, 30005, 30006, 30007,
		   30016, 30017, 30018, 30019, 30036, 30037, 30038, 30039,
		   -1, -1, -1, -1, -1, -1, -1, -1, -1}}};
	for (i = 0; i < 2; i++) {
		for (j = 0; s_sex[i].sex[j] != -1; j++) {
			if (CHAR_getInt(charaindex, CHAR_BASEBASEIMAGENUMBER) == s_sex[i].sex[j]) {
				if (i == *data)
					return TRUE;
				else
					return FALSE;
			}
		}
	}
	return FALSE;
}
