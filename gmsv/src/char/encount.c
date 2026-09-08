#include "version.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef _REDHAT_V9
#include <errno.h>
#endif

#include "common.h"
#include "util.h"
#include "buf.h"
#include "char_base.h"
#include "char.h"
#include "configfile.h"
#include "encount.h"
#include "enemy.h"

#ifdef _ADD_ENCOUNT // WON ADD 增加敌遭遇触发修件
#include "encount.h"
#endif

/* エンカウント関連のソース  (遭遇相关源码) */

#ifndef _ADD_ENCOUNT // WON ADD 增加敌遭遇触发修件
typedef struct tagENCOUNT_Table {
	int index;
	int floor;
	int encountprob_min; /* エンカウント確率  (遭遇概率)   */
	int encountprob_max; /* エンカウント確率  (遭遇概率)   */
	int enemymaxnum;	 /* どれだけ敵を出すか  (出多少敌人) */
	int zorder;
	int groupid[ENCOUNT_GROUPMAXNUM];	 /* グループNo  (怪物组编号) */
	int createprob[ENCOUNT_GROUPMAXNUM]; /* そのグループの出現  (该怪物组出现率)   */
	RECT rect;
} ENCOUNT_Table;
ENCOUNT_Table *ENCOUNT_table;
#endif

int ENCOUNT_encountnum;
#define ENCOUNT_ENEMYMAXCREATENUM 10

static INLINE BOOL ENCOUNT_CHECKENCOUNTTABLEARRAY(int array) {
	if (array < 0 || array > ENCOUNT_encountnum - 1)
		return FALSE;
	return TRUE;
}

/*------------------------------------------------------------
 * エンカウントデータの初期化をする  (初始化遭遇数据)［
 * 戻り値  (返回值)
 *  filename        char*       データファイル  (数据文件)
 * 戻り値  (返回值)
 *  正常      TRUE(1)  (正常)
 *  取得失敗      FALSE(0)  (失败)
 *------------------------------------------------------------*/
BOOL ENCOUNT_initEncount(char *filename) {
	FILE *f;
	char line[256];
	int linenum = 0;
	int encount_readlen = 0;

	f = fopen(filename, "r");
	if (f == NULL) {
		errorprint;
		return FALSE;
	}

	ENCOUNT_encountnum = 0;

	/*  まず有効な行が何行あるかどうか調べる  (先检查有多少有效行)    */
	while (fgets(line, sizeof(line), f)) {
		linenum++;
		if (line[0] == '#')
			continue; /* comment */
		if (line[0] == '\n')
			continue; /* none    */
		chomp(line);

		ENCOUNT_encountnum++;
	}

	if (fseek(f, 0, SEEK_SET) == -1) {
		fprint("寻找错误\n");
		fclose(f);
		return FALSE;
	}

	ENCOUNT_table = allocateMemory(sizeof(struct tagENCOUNT_Table) * ENCOUNT_encountnum);
	if (ENCOUNT_table == NULL) {
		fprint("无法分配内存 %d\n",
			   sizeof(ENCOUNT_table) * ENCOUNT_encountnum);
		fclose(f);
		return FALSE;
	}

	/* 初期化  (初始化) */
	{
		int i, j;
		for (i = 0; i < ENCOUNT_encountnum; i++) {
			ENCOUNT_table[i].index = -1;
			ENCOUNT_table[i].floor = 0;
			ENCOUNT_table[i].encountprob_min = 1;
			ENCOUNT_table[i].encountprob_min = 50;
			ENCOUNT_table[i].enemymaxnum = 4;
			ENCOUNT_table[i].rect.x = 0;
			ENCOUNT_table[i].rect.y = 0;
			ENCOUNT_table[i].rect.height = 0;
			ENCOUNT_table[i].rect.width = 0;
			ENCOUNT_table[i].zorder = 0;
			for (j = 0; j < ENCOUNT_GROUPMAXNUM; j++) {
				ENCOUNT_table[i].groupid[j] = -1;
				ENCOUNT_table[i].createprob[j] = -1;
			}
#ifdef _ADD_ENCOUNT // WON ADD 增加敌遭遇触发修件
			ENCOUNT_table[i].event_now = -1;
			ENCOUNT_table[i].event_end = -1;
			ENCOUNT_table[i].enemy_group = -1;
#endif
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
			int x1, x2, y1, y2;
			int j;

			/*   度目のループに足す為の初期化  (为逐行读取做的初始化) */
			ENCOUNT_table[encount_readlen].index = -1;
			ENCOUNT_table[encount_readlen].floor = 0;
			ENCOUNT_table[encount_readlen].encountprob_min = 1;
			ENCOUNT_table[encount_readlen].encountprob_min = 50;
			ENCOUNT_table[encount_readlen].enemymaxnum = 4;
			ENCOUNT_table[encount_readlen].rect.x = 0;
			ENCOUNT_table[encount_readlen].rect.y = 0;
			ENCOUNT_table[encount_readlen].rect.height = 0;
			ENCOUNT_table[encount_readlen].rect.width = 0;
			ENCOUNT_table[encount_readlen].zorder = 0;
			for (j = 0; j < ENCOUNT_GROUPMAXNUM; j++) {
				ENCOUNT_table[encount_readlen].groupid[j] = -1;
				ENCOUNT_table[encount_readlen].createprob[j] = -1;
			}
#ifdef _ADD_ENCOUNT // WON ADD 增加敌遭遇触发修件
			ENCOUNT_table[encount_readlen].event_now = -1;
			ENCOUNT_table[encount_readlen].event_end = -1;
			ENCOUNT_table[encount_readlen].enemy_group = -1;
#endif

			/*  ひとつ目のトークンを取る  (取第一个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 1, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			ENCOUNT_table[encount_readlen].index = atoi(token);

			/*  2つ目のトークンを取る  (取第2个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 2, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			ENCOUNT_table[encount_readlen].floor = atoi(token);

			/*  3つ目のトークンを取る  (取第3个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 3, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			x1 = atoi(token);

			/*  4つ目のトークンを取る  (取第4个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 4, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			y1 = atoi(token);

			/*  5つ目のトークンを取る  (取第5个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 5, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}

			x2 = atoi(token);

			/*  6つ目のトークンを取る  (取第6个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 6, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			y2 = atoi(token);

			ENCOUNT_table[encount_readlen].rect.x = min(x1, x2);
			ENCOUNT_table[encount_readlen].rect.width = max(x1, x2) - min(x1, x2);
			ENCOUNT_table[encount_readlen].rect.y = min(y1, y2);
			ENCOUNT_table[encount_readlen].rect.height = max(y1, y2) - min(y1, y2);

			/*  7つ目のトークンを取る  (取第7个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 7, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			ENCOUNT_table[encount_readlen].encountprob_min = atoi(token);

			/*  8つ目のトークンを取る  (取第8个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 8, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			ENCOUNT_table[encount_readlen].encountprob_max = atoi(token);

			{
				int a, b;
				a = ENCOUNT_table[encount_readlen].encountprob_min;
				b = ENCOUNT_table[encount_readlen].encountprob_max;
				/*   大小関係の調整  (调整大小关系) */
				ENCOUNT_table[encount_readlen].encountprob_min = min(a, b);
				ENCOUNT_table[encount_readlen].encountprob_max = max(a, b);
			}
			/*  9つ目のトークンを取る  (取第9个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 9, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			{
				int maxnum = atoi(token);
				/* 数の大小のチェック  (检查数值大小) */
				if (maxnum < 1 || maxnum > ENCOUNT_ENEMYMAXCREATENUM) {
					fprint("文件语法错误:%s 第%d行\n", filename, linenum);
					continue;
				}
				ENCOUNT_table[encount_readlen].enemymaxnum = maxnum;
			}
			/*  10つ目のトークンを取る  (取第10个字段)    */
			ret = getStringFromIndexWithDelim(line, ",", 10, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			ENCOUNT_table[encount_readlen].zorder = atoi(token);
#define CREATEPROB_TOKEN 11

			/*  11 31つ目のトークンを取る  (取第11~31个字段)    */
			{
				int i;

				for (i = CREATEPROB_TOKEN; i < CREATEPROB_TOKEN + ENCOUNT_GROUPMAXNUM * 2; i++) {
					ret = getStringFromIndexWithDelim(line, ",", i, token,
													  sizeof(token));
					if (ret == FALSE) {
						fprint("文件语法错误:%s 第%d行\n", filename, linenum);
						continue;
					}
					if (strlen(token) != 0) {
						if (i < CREATEPROB_TOKEN + ENCOUNT_GROUPMAXNUM) {
							ENCOUNT_table[encount_readlen].groupid[i - CREATEPROB_TOKEN] = atoi(token);
						} else {
							ENCOUNT_table[encount_readlen].createprob[i - (CREATEPROB_TOKEN + ENCOUNT_GROUPMAXNUM)] = atoi(token);
						}
					}
				}

				/* 重複チェック  (重复检查) */
				if (checkRedundancy(ENCOUNT_table[encount_readlen].groupid,
									arraysizeof(ENCOUNT_table[encount_readlen].groupid))) {
					fprint("文件语法错误:%s 第%d行\n",
						   filename, linenum);
					continue;
				}
			}

#ifdef _ADD_ENCOUNT // WON ADD 增加敌遭遇触发修件
			ret = getStringFromIndexWithDelim(line, ",", 31, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			ENCOUNT_table[encount_readlen].event_now = atoi(token);

			ret = getStringFromIndexWithDelim(line, ",", 32, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			ENCOUNT_table[encount_readlen].event_end = atoi(token);

			ret = getStringFromIndexWithDelim(line, ",", 33, token,
											  sizeof(token));
			if (ret == FALSE) {
				fprint("文件语法错误:%s 第%d行\n", filename, linenum);
				continue;
			}
			ENCOUNT_table[encount_readlen].enemy_group = atoi(token);
#endif

			encount_readlen++;
		}
	}
	fclose(f);

	ENCOUNT_encountnum = encount_readlen;

	print("有效的遇敌坐标数是 %d..", ENCOUNT_encountnum);

#if 0

    {
        int i;
        for( i=0; i <ENCOUNT_encountnum ; i++ )
            print( "encount idx[%d] fl[%d] prob_min[%d] prob_max[%d] e_max[%d] x[%d] wth[%d] y[%d] hgt[%d] \n",
                   ENCOUNT_table[i].index,
                   ENCOUNT_table[i].floor,
                   ENCOUNT_table[i].encountprob_min,
                   ENCOUNT_table[i].encountprob_max,
                   ENCOUNT_table[i].enemymaxnum,
                   ENCOUNT_table[i].rect.x,
                   ENCOUNT_table[i].rect.width,
                   ENCOUNT_table[i].rect.y,
                   ENCOUNT_table[i].rect.height);
    }
#endif
	return TRUE;
}
/*------------------------------------------------------------------------
 * エンカウントデータファイル 読み直し  (重读遭遇数据文件)
 *-----------------------------------------------------------------------*/
BOOL ENCOUNT_reinitEncount(void) {
	freeMemory(ENCOUNT_table);
	return (ENCOUNT_initEncount(getEncountfile()));
}

/*------------------------------------------------------------
 * 指定された座標のENCOUNT_tableの検索を調べる  (按指定坐标查ENCOUNT_table)［
 * zorderの数値を上げて優先順位の高い場所を取得する  (提高zorder数值以取得高优先级位置)［
 * 戻り値  (返回值)
 *  floor       int     フロアID  (楼层ID)
 *  x           int     x座標  (x坐标)
 *  y           int     y座標  (y坐标)
 * 戻り値  (返回值)
 *  正常      検索  (正常/找到)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getEncountAreaArray(int floor, int x, int y) {
	int i;
	int index = -1;
	for (i = 0; i < ENCOUNT_encountnum; i++) {
		if (ENCOUNT_table[i].floor == floor) {
			if (CoordinateInRect(&ENCOUNT_table[i].rect, x, y)) {
				int curZorder = ENCOUNT_getZorderFromArray(i);
				if (curZorder > 0) {
					if (index != -1) {
						/* 優先順位を調べる  (检查优先级) */
						/*   大きい 優先  (更大者优先) */
						if (curZorder > ENCOUNT_getZorderFromArray(index)) {
							index = i;
						}
					} else {
						index = i;
					}
				}
			}
		}
	}
	return index;
}

/*------------------------------------------------------------
 * 指定された座標のエンカウント数を調べる  (按指定坐标查遭遇数)［
 * 戻り値  (返回值)
 *  floor       int     フロアID  (楼层ID)
 *  x           int     x座標  (x坐标)
 *  y           int     y座標  (y坐标)
 * 戻り値  (返回值)
 *  正常      遭遇の数  (正常/遭遇数)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getEncountPercentMin(int charaindex, int floor, int x, int y) {
	int ret;

	ret = ENCOUNT_getEncountAreaArray(floor, x, y);
	if (ret != -1) {
		ret = ENCOUNT_table[ret].encountprob_min;
		/* トヘリオス効果をつける  (附加托赫里奥斯效果) */
		if (CHAR_getWorkInt(charaindex, CHAR_WORK_TOHELOS_COUNT) > 0) {
			ret = ceil(ret *
					   ((100 + CHAR_getWorkInt(charaindex, CHAR_WORK_TOHELOS_CUTRATE)) / 100.0));
		}
		if (ret < 0)
			ret = 0;
		if (ret > 100)
			ret = 100;
	}
	return ret;
}
/*------------------------------------------------------------
 * 指定された座標のエンカウント数を調べる  (按指定坐标查遭遇数)［
 * 戻り値  (返回值)
 *  floor       int     フロアID  (楼层ID)
 *  x           int     x座標  (x坐标)
 *  y           int     y座標  (y坐标)
 * 戻り値  (返回值)
 *  正常      遭遇の数  (正常/遭遇数)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getEncountPercentMax(int charaindex, int floor, int x, int y) {
	int ret;

	ret = ENCOUNT_getEncountAreaArray(floor, x, y);
	if (ret != -1) {
		ret = ENCOUNT_table[ret].encountprob_max;
		/* トヘリオス効果をつける  (附加托赫里奥斯效果) */
		if (CHAR_getWorkInt(charaindex, CHAR_WORK_TOHELOS_COUNT) > 0) {
			ret = ceil(ret *
					   ((100 + CHAR_getWorkInt(charaindex, CHAR_WORK_TOHELOS_CUTRATE)) / 100.0));
		}
		if (ret < 0)
			ret = 0;
		if (ret > 100)
			ret = 100;
	}
	return ret;
}
/*------------------------------------------------------------
 * 指定された座標のパラメータMAX数を調べる  (按指定坐标查参数MAX数)［
 * 戻り値  (返回值)
 *  floor       int     フロアID  (楼层ID)
 *  x           int     x座標  (x坐标)
 *  y           int     y座標  (y坐标)
 * 戻り値  (返回值)
 *  正常      遭遇の数  (正常/遭遇数)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getCreateEnemyMaxNum(int floor, int x, int y) {
	int ret;

	ret = ENCOUNT_getEncountAreaArray(floor, x, y);
	if (ret != -1) {
		ret = ENCOUNT_table[ret].enemymaxnum;
	}
	return ret;
}
/*------------------------------------------------------------
 * 指定された座標のエンカウントフィールドのindexを調べる  (按指定坐标查遭遇字段索引)［
 * 戻り値  (返回值)
 *  floor       int     フロアID  (楼层ID)
 *  x           int     x座標  (x坐标)
 *  y           int     y座標  (y坐标)
 * 戻り値  (返回值)
 *  正常      遭遇  (正常/遭遇)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getEncountIndex(int floor, int x, int y) {
	int ret;

	ret = ENCOUNT_getEncountAreaArray(floor, x, y);
	if (ret != -1) {
		ret = ENCOUNT_table[ret].index;
	}
	return ret;
}
/*------------------------------------------------------------
 * 指定された検索のエンカウントフィールドのindexを調べる  (按指定检索查遭遇字段索引)［
 * 戻り値  (返回值)
 *  array           int     ENCOUNTTABLEの検索  (ENCOUNT表检索)
 * 戻り値  (返回值)
 *  正常      遭遇  (正常/遭遇)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getEncountIndexFromArray(int array) {
	if (!ENCOUNT_CHECKENCOUNTTABLEARRAY(array))
		return -1;
	return ENCOUNT_table[array].index;
}
/*------------------------------------------------------------
 * 指定された検索のエンカウント数を調べる  (按指定检索查遭遇数)［
 * 戻り値  (返回值)
 *  array           int     ENCOUNTTABLEの検索  (ENCOUNT表检索)
 * 戻り値  (返回值)
 *  正常      遭遇  (正常/遭遇)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getEncountPercentFromArray(int array) {
	if (!ENCOUNT_CHECKENCOUNTTABLEARRAY(array))
		return -1;
	return ENCOUNT_table[array].encountprob_min;
}
/*------------------------------------------------------------
 * 指定された検索のパラメータMAX数を調べる  (按指定检索查参数MAX数)［
 * 戻り値  (返回值)
 *  array           int     ENCOUNTTABLEの検索  (ENCOUNT表检索)
 * 戻り値  (返回值)
 *  正常      遭遇  (正常/遭遇)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getCreateEnemyMaxNumFromArray(int array) {
	if (!ENCOUNT_CHECKENCOUNTTABLEARRAY(array))
		return -1;
	return ENCOUNT_table[array].enemymaxnum;
}
/*------------------------------------------------------------
 * 指定された検索のグループ号を調べる  (按指定检索查怪物组号)［
 * 戻り値  (返回值)
 *  array           int     ENCOUNTTABLEの検索  (ENCOUNT表检索)
 * 戻り値  (返回值)
 *  正常      遭遇  (正常/遭遇)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getGroupIdFromArray(int array, int grouparray) {
	if (!ENCOUNT_CHECKENCOUNTTABLEARRAY(array))
		return -1;
	return ENCOUNT_table[array].groupid[grouparray];
}
/*------------------------------------------------------------
 * 指定された検索のグループの出現を調べる  (按指定检索查怪物组出现率)［
 * 戻り値  (返回值)
 *  array           int     ENCOUNTTABLEの検索  (ENCOUNT表检索)
 * 戻り値  (返回值)
 *  正常      遭遇  (正常/遭遇)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getGroupProbFromArray(int array, int grouparray) {
	if (!ENCOUNT_CHECKENCOUNTTABLEARRAY(array))
		return -1;
	return ENCOUNT_table[array].createprob[grouparray];
}
/*------------------------------------------------------------
 * 指定された検索の優先順位を調べる  (按指定检索查优先级)［
 * 戻り値  (返回值)
 *  array           int     ENCOUNTTABLEの検索  (ENCOUNT表检索)
 * 戻り値  (返回值)
 *  正常      遭遇  (正常/遭遇)
 *  取得失敗    -1  (获取失败)
 ------------------------------------------------------------*/
int ENCOUNT_getZorderFromArray(int array) {
	if (!ENCOUNT_CHECKENCOUNTTABLEARRAY(array))
		return -1;
	return ENCOUNT_table[array].zorder;
}
