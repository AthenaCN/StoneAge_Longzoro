#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "configfile.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "npc_riderman.h"
#include "family.h"
#include "log.h"

extern int familyNumTotal;
extern char familyListBuf[MAXFAMILYLIST];
extern tagRidePetTable ridePetTable[296];
extern struct FM_POINTLIST fmpointlist;

char sendbuf[1024];
char buf[1024];
char subbuf[128];
int i, j;

/*
 * 設定されたウィンドウを開くNPC  (打开已设定窗口的NPC)
 * 簡単テキストアドベンチャーくらいならできるかも  (简单文字冒险的话或许可以)
 *
 */

struct {
	int windowno;
	int windowtype;
	int buttontype;
	int takeitem;
	int giveitem;
	int takegold;
	char message[4096];
	int letter[4];
} w;

struct {
	BOOL use;
	int checkhaveitem;
	int checkhaveitemgotowin;
	int checkdonthaveitem;
	int checkdonthaveitemgotowin;
	int warp;
	int battle;
	int gotowin;
} buttonproc[13]; /* ok,cancel, yes,no,prev,next の時の処理  (ok/cancel/yes/no/prev/next 时的处理) */

enum {
	CHAR_WORK_MSGCOLOR = CHAR_NPCWORKINT1,
};

static void NPC_Riderman_selectWindow(int meindex, int toindex, int num);
static BOOL NPC_Riderman_readData(int meindex, int windowno, BOOL chkflg);
static int NPC_Riderman_restoreButtontype(char *data);
// static BOOL getLetter(int meindex,int talker );
// static BOOL checkLetter(int meindex,int talker );

/*********************************
 * 初期化  (初始化)
 *********************************/
BOOL NPC_RidermanInit(int meindex) {
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[1024];
	// int		i;
	// char	secondToken[1024];

	// Robin test
	// print(" riderman_Init ");

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	if (NPC_Util_GetStrFromStrWithDelim(argstr, "conff", buf, sizeof(buf)) == NULL) {
		print("riderman:没有指定设定的档案 。\n");
		return FALSE;
	}
	/* 初期時に設定データをチェックしてやる  (初始化时检查设定数据) */
	if (!NPC_Riderman_readData(meindex, -1, TRUE)) {
		return FALSE;
	}

	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPEWINDOWMAN);

	return TRUE;
}

/*********************************
 *   しかけられた時の処理  (被设置时的处理)
 *********************************/
void NPC_RidermanTalked(int meindex, int talkerindex, char *szMes, int color) {
	// print(" Rideman_fmindex:%d ", CHAR_getInt( meindex, CHAR_FMINDEX) );

	NPC_Riderman_selectWindow(meindex, talkerindex, 1);
}
/*********************************
 * 話された時の処理  (被搭话时的处理)
 *********************************/
void NPC_RidermanLooked(int meindex, int lookedindex) {

	// print(" Look:me:%d,looked:%d ",meindex,lookedindex);
	NPC_Riderman_selectWindow(meindex, lookedindex, 1);
}

static void NPC_Riderman_selectWindow(int meindex, int toindex, int num) {

	int fd;
	char buf[256];

	/* プレイヤーに対してだけ実行する  (仅对玩家执行) */
	if (CHAR_getInt(toindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) {
		return;
	}
	/* グリッド移動のみ   (仅网格移动) */
	if (!NPC_Util_charIsInFrontOfChar(toindex, meindex, 1))
		return;

	if (!NPC_Riderman_readData(meindex, num, FALSE)) {
		print("riderman:readdata error\n");
		return;
	}

	fd = getfdFromCharaIndex(toindex);
	if (fd != -1) {
		lssproto_WN_send(fd, w.windowtype,
						 w.buttontype,
						 w.windowno + 100,
						 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
						 makeEscapeString(w.message, buf, sizeof(buf)));
	}
}

void NPC_RidermanWindowTalked(int meindex, int talkerindex,
							  int seqno, int select, char *data)

{

	int button = -1;
	char buf[512];

	/* グリッド移動のみ   (仅网格移动) */
	if (NPC_Util_CharDistance(talkerindex, meindex) > 1)
		return;

	/* このウィンドウ番号のデータを読み込み  (读取该窗口编号的数据) */
	if (!NPC_Riderman_readData(meindex, seqno - 100, FALSE)) {
		print("riderman:readdata error\n");
		return;
	}
	/* 押したボタンを調べる  (检查按下的按钮) */
	if (w.windowtype == WINDOW_MESSAGETYPE_SELECT) {
		button = atoi(data) + 5;
		if (button > 12) {
			print("riderman:invalid button[%d]\n", button);
			return;
		}
	} else if (select & WINDOW_BUTTONTYPE_OK)
		button = 0;
	else if (select & WINDOW_BUTTONTYPE_CANCEL)
		button = 1;
	else if (select & WINDOW_BUTTONTYPE_YES)
		button = 2;
	else if (select & WINDOW_BUTTONTYPE_NO)
		button = 3;
	else if (select & WINDOW_BUTTONTYPE_PREV)
		button = 4;
	else if (select & WINDOW_BUTTONTYPE_NEXT)
		button = 5;
	else {
		print("riderman:invalid button[%d]\n", select);
		return;
	}
	if (buttonproc[button].use == TRUE) {
		int fd;
		int newwin = -1;

		fd = getfdFromCharaIndex(talkerindex);

		/* ボタンによって処理を振り分ける  (根据按钮分配处理) */
		if (newwin == -1) {
			newwin = buttonproc[button].gotowin;
		}

		// print(" takegold:%d ", w.takegold );

		// Robin
		// 学习 Lv 40
		if (newwin == 6) {

			int charImg;
			// int i, petindex, bFind = 0;

			charImg = CHAR_getInt(talkerindex, CHAR_BASEBASEIMAGENUMBER);
			if (CHAR_getInt(talkerindex, CHAR_LEARNRIDE) >= 40) {
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString("\n\n你已经学会初级班了呀。\n不需要再学了。",
												  buf, sizeof(buf)));
				return;
			}

#if 0
			if( !checkLetter( meindex, talkerindex) )
			{
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n\n很抱歉喔！\n学习骑乘宠物需要本庄园族长的同意书。",
							buf, sizeof(buf)));
				return;					
			}

			for( i=0; i<CHAR_MAXPETHAVE ; i++)
			{
				if( bFind ) break;
				petindex = CHAR_getCharPet( talkerindex, i );
				if( CHAR_CHECKINDEX(petindex) )
				{
					int petImg, j;
					
					//if( CHAR_getWorkInt( petindex, CHAR_WORKFIXAI ) < 100 )
					//	continue;
					petImg = CHAR_getInt( petindex, CHAR_BASEBASEIMAGENUMBER);
					for( j=0; j<arraysizeof(ridePetTable); j++)
					{
						if( ridePetTable[j].charNo == charImg
						    && ridePetTable[j].petNo == petImg )
						{
							bFind = 1;
							break;
						}
					}
				}
			}
			if( bFind != 1 )
			{
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n\n你身上没有适合骑乘的宠物。请先准备一只。",
							buf, sizeof(buf)));
				return;
			}

			if( !getLetter( meindex, talkerindex) )	return;
#endif

			if (CHAR_getInt(talkerindex, CHAR_GOLD) < w.takegold) {
				char buf2[512];

				sprintf(buf2, "\n很抱歉喔！你的学费不足！\n学习骑乘宠物初级班需要%d石币。", w.takegold);
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString(buf2, buf, sizeof(buf)));
				return;
			}
			CHAR_DelGold(talkerindex, w.takegold);

			CHAR_setInt(talkerindex, CHAR_LEARNRIDE, 40);
			CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_GOLD);
			CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_LEARNRIDE);

			// sprintf( buf, "R|L|1" );
			// lssproto_FM_send( fd, buf );
			lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
							 WINDOW_BUTTONTYPE_OK, -1, -1,
							 makeEscapeString(
								 "\n\n恭喜你！你已经完成骑乘初级班了。\n可以骑乘 Lv40 以下的宠物，\n请小心骑乘喔！",
								 buf, sizeof(buf)));

			// 家族收入
			{
				int fmindex, fmindexi, village, i;
				char fmname[256], token[256], buf2[64];

				sprintf(buf2, "%d", w.takegold / 5);
				for (i = 0; i < FMPOINTNUM; i++) {
					getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 9, token, sizeof(token));
					village = atoi(token);
					if (village == CHAR_getInt(meindex, CHAR_FMINDEX)) {
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 5, token, sizeof(token));
						fmindex = atoi(token);
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 6, token, sizeof(token));
						strcpy(fmname, token);
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 8, token, sizeof(token));
						fmindexi = atoi(token);
						saacproto_ACFixFMData_send(acfd, fmname, fmindex, fmindexi,
												   FM_FIX_FMGOLD, buf2, "",
												   CHAR_getWorkInt(meindex, CHAR_WORKFMCHARINDEX),
												   CONNECT_getFdid(fd));

						// print(" rider_fm:%s ", fmname);
					}
				}
			}

			return;
		}

		// 学习 Lv 80
		if (newwin == 7) {

			int charImg;
			// int i, petindex, bFind = 0;

			charImg = CHAR_getInt(talkerindex, CHAR_BASEBASEIMAGENUMBER);
			if (CHAR_getInt(talkerindex, CHAR_LEARNRIDE) >= 80) {
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString("\n\n你已经学会中级班了呀。\n不需要再学了。",
												  buf, sizeof(buf)));
				return;
			} else if (CHAR_getInt(talkerindex, CHAR_LEARNRIDE) < 40) {
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString("\n\n你必须先学过初级班才能来中级班唷。",
												  buf, sizeof(buf)));
				return;
			}

#if 0
			if( !checkLetter( meindex, talkerindex) )
			{
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n\n很抱歉喔！\n学习骑乘宠物需要本庄园族长的同意书。",
							buf, sizeof(buf)));
				return;					
			}

			for( i=0; i<CHAR_MAXPETHAVE ; i++)
			{
				if( bFind ) break;
				petindex = CHAR_getCharPet( talkerindex, i );
				if( CHAR_CHECKINDEX(petindex) )
				{
					int petImg, j;
					
					petImg = CHAR_getInt( petindex, CHAR_BASEBASEIMAGENUMBER);
					for( j=0; j<arraysizeof(ridePetTable); j++)
					{
						if( ridePetTable[j].charNo == charImg
						    && ridePetTable[j].petNo == petImg )
						{
							bFind = 1;
							break;
						}
					}
				}
			}			
			if( bFind != 1 )
			{
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n\n请先准备一只适合自己骑乘的宠物。",
							buf, sizeof(buf)));
				return;
			}

			if( !getLetter( meindex, talkerindex) ) return;
#endif

			if (CHAR_getInt(talkerindex, CHAR_GOLD) < w.takegold) {
				char buf2[512];

				sprintf(buf2, "\n很抱歉喔！你的学费不足！\n学习骑乘宠物中级班需要%d石币", w.takegold);
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString(buf2, buf, sizeof(buf)));
				return;
			}
			CHAR_DelGold(talkerindex, w.takegold);

			CHAR_setInt(talkerindex, CHAR_LEARNRIDE, 80);
			CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_GOLD);
			CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_LEARNRIDE);

			// sprintf( buf, "R|L|1" );
			// lssproto_FM_send( fd, buf );
			lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
							 WINDOW_BUTTONTYPE_OK, -1, -1,
							 makeEscapeString("\n\n恭喜你！你已经完成骑乘中级班了。\n可以骑乘 Lv80 以下的宠物，\n请小心骑乘喔！",
											  buf, sizeof(buf)));

			// 家族收入
			{
				int fmindex, fmindexi, village, i;
				char fmname[256], token[256], buf2[64];

				sprintf(buf2, "%d", w.takegold / 5);
				for (i = 0; i < FMPOINTNUM; i++) {
					getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 9, token, sizeof(token));
					village = atoi(token);
					if (village == CHAR_getInt(meindex, CHAR_FMINDEX)) {
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 5, token, sizeof(token));
						fmindex = atoi(token);
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 6, token, sizeof(token));
						strcpy(fmname, token);
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 8, token, sizeof(token));
						fmindexi = atoi(token);
						saacproto_ACFixFMData_send(acfd, fmname, fmindex, fmindexi,
												   FM_FIX_FMGOLD, buf2, "",
												   CHAR_getWorkInt(meindex, CHAR_WORKFMCHARINDEX),
												   CONNECT_getFdid(fd));

						// print(" rider_fm:%s ", fmname);
					}
				}
			}

			return;
		}

		// 学习 Lv 120
		if (newwin == 8) {

			int charImg;
			// int i, petindex, bFind = 0;

			charImg = CHAR_getInt(talkerindex, CHAR_BASEBASEIMAGENUMBER);
			if (CHAR_getInt(talkerindex, CHAR_LEARNRIDE) >= 120) {
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString("\n\n你已经学会高级班了呀。\n不需要再学了。",
												  buf, sizeof(buf)));
				return;
			} else if (CHAR_getInt(talkerindex, CHAR_LEARNRIDE) < 80) {
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString("\n\n你必须先学过中级班才能来高级班唷。",
												  buf, sizeof(buf)));
				return;
			}

#if 0
			if( !checkLetter( meindex, talkerindex) )
			{
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n\n很抱歉喔！\n学习骑乘宠物需要本庄园族长的同意书。",
							buf, sizeof(buf)));
				return;					
			}

			for( i=0; i<CHAR_MAXPETHAVE ; i++)
			{
				if( bFind ) break;
				petindex = CHAR_getCharPet( talkerindex, i );
				if( CHAR_CHECKINDEX(petindex) )
				{
					int petImg, j;
					
					petImg = CHAR_getInt( petindex, CHAR_BASEBASEIMAGENUMBER);
					for( j=0; j<arraysizeof(ridePetTable); j++)
					{
						if( ridePetTable[j].charNo == charImg
						    && ridePetTable[j].petNo == petImg )
						{
							bFind = 1;
							break;
						}
					}
				}
			}			
			if( bFind != 1 )
			{
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n\n请先准备一只适合自己骑乘的宠物。",
							buf, sizeof(buf)));
				return;
			}

			if( !getLetter( meindex, talkerindex) ) return;
#endif

			if (CHAR_getInt(talkerindex, CHAR_GOLD) < w.takegold) {
				char buf2[512];

				sprintf(buf2, "\n很抱歉喔！你的学费不足！\n学习骑乘宠物高级班需要%d石币", w.takegold);
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString(buf2, buf, sizeof(buf)));
				return;
			}
			CHAR_DelGold(talkerindex, w.takegold);

			CHAR_setInt(talkerindex, CHAR_LEARNRIDE, 120);
			CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_GOLD);
			CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_LEARNRIDE);

			// sprintf( buf, "R|L|1" );
			// lssproto_FM_send( fd, buf );
			lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
							 WINDOW_BUTTONTYPE_OK, -1, -1,
							 makeEscapeString("\n\n恭喜你！你已经完成骑乘高级班了。\n可以骑乘 Lv120 以下的宠物，\n请小心骑乘喔！",
											  buf, sizeof(buf)));

			// 家族收入
			{
				int fmindex, fmindexi, village, i;
				char fmname[256], token[256], buf2[64];

				sprintf(buf2, "%d", w.takegold / 5);
				for (i = 0; i < FMPOINTNUM; i++) {
					getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 9, token, sizeof(token));
					village = atoi(token);
					if (village == CHAR_getInt(meindex, CHAR_FMINDEX)) {
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 5, token, sizeof(token));
						fmindex = atoi(token);
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 6, token, sizeof(token));
						strcpy(fmname, token);
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 8, token, sizeof(token));
						fmindexi = atoi(token);
						saacproto_ACFixFMData_send(acfd, fmname, fmindex, fmindexi,
												   FM_FIX_FMGOLD, buf2, "",
												   CHAR_getWorkInt(meindex, CHAR_WORKFMCHARINDEX),
												   CONNECT_getFdid(fd));

						// print(" rider_fm:%s ", fmname);
					}
				}
			}

			return;
		}

		// 学习  Lv All
		if (newwin == 9) {

			int charImg;
			// int petindex, i, bFind = 0;

			charImg = CHAR_getInt(talkerindex, CHAR_BASEBASEIMAGENUMBER);
			if (CHAR_getInt(talkerindex, CHAR_LEARNRIDE) > 120) {
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString("\n\n你已经学会特级班了呀。\n不需要再学了。",
												  buf, sizeof(buf)));
				return;
			} else if (CHAR_getInt(talkerindex, CHAR_LEARNRIDE) < 120) {
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString("\n\n你必须先学过高级班才能来特级班唷。",
												  buf, sizeof(buf)));
				return;
			}

#if 0
			if( !checkLetter( meindex, talkerindex) )
			{
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n\n很抱歉喔！\n学习骑乘宠物需要本庄园族长的同意书。",
							buf, sizeof(buf)));
				return;					
			}

			for( i=0; i<CHAR_MAXPETHAVE ; i++)
			{
				if( bFind ) break;
				petindex = CHAR_getCharPet( talkerindex, i );
				if( CHAR_CHECKINDEX(petindex) )
				{
					int petImg, j;
					
					petImg = CHAR_getInt( petindex, CHAR_BASEBASEIMAGENUMBER);
					for( j=0; j<arraysizeof(ridePetTable); j++)
					{
						if( ridePetTable[j].charNo == charImg
						    && ridePetTable[j].petNo == petImg )
						{
							bFind = 1;
							break;
						}
					}
				}
			}			
			if( bFind != 1 )
			{
				lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
					WINDOW_BUTTONTYPE_OK, -1, -1,
					makeEscapeString( "\n\n请先准备一只适合自己骑乘的宠物。",
							buf, sizeof(buf)));
				return;
			}

			if( !getLetter( meindex, talkerindex) ) return;
#endif

			if (CHAR_getInt(talkerindex, CHAR_GOLD) < w.takegold) {
				char buf2[512];

				sprintf(buf2, "\n很抱歉喔！你的学费不足！\n学习骑乘宠物特级班需要%d石币", w.takegold);
				lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
								 WINDOW_BUTTONTYPE_OK, -1, -1,
								 makeEscapeString(buf2, buf, sizeof(buf)));
				return;
			}
			CHAR_DelGold(talkerindex, w.takegold);

			CHAR_setInt(talkerindex, CHAR_LEARNRIDE, 200);
			CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_GOLD);
			CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_LEARNRIDE);

			// sprintf( buf, "R|L|1" );
			// lssproto_FM_send( fd, buf );
			lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
							 WINDOW_BUTTONTYPE_OK, -1, -1,
							 makeEscapeString("\n\n恭喜你！你已经完成骑乘特级班了。\n可以骑乘所有等级的宠物，\n请小心骑乘喔！",
											  buf, sizeof(buf)));

			// 家族收入
			{
				int fmindex, fmindexi, village, i;
				char fmname[256], token[256], buf2[64];

				sprintf(buf2, "%d", w.takegold / 5);
				for (i = 0; i < FMPOINTNUM; i++) {
					getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 9, token, sizeof(token));
					village = atoi(token);
					if (village == CHAR_getInt(meindex, CHAR_FMINDEX)) {
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 5, token, sizeof(token));
						fmindex = atoi(token);
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 6, token, sizeof(token));
						strcpy(fmname, token);
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 8, token, sizeof(token));
						fmindexi = atoi(token);
						saacproto_ACFixFMData_send(acfd, fmname, fmindex, fmindexi,
												   FM_FIX_FMGOLD, buf2, "",
												   CHAR_getWorkInt(meindex, CHAR_WORKFMCHARINDEX),
												   CONNECT_getFdid(fd));

						// print(" rider_fm:%s ", fmname);
					}
				}
			}

			return;
		}

		// 说明 , 列表
		if (newwin == 5) {

			lssproto_WN_send(fd, WINDOW_MESSAGETYPE_SHOWRIDEPET,
							 WINDOW_BUTTONTYPE_OK, -1, -1,
							 "");
			return;
		}

		if (!NPC_Riderman_readData(meindex, newwin, FALSE)) {
			print("riderman:readdata error\n");
			return;
		}

#if 0

#endif

		// fd = getfdFromCharaIndex( talkerindex);
		if (fd != -1) {
			lssproto_WN_send(fd, w.windowtype,
							 w.buttontype,
							 w.windowno + 100,
							 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
							 makeEscapeString(w.message, buf, sizeof(buf)));
		}
	}
}
/*
 * 設定ファイルを読んで指定されたwindownoのデータをセットする  (读取设定文件并设置指定windowno的数据)
 *
 * 娄醒“
 *		meindex		int		このNPCのcharaindex  (该NPC的charaindex)
 *		windowno	int		ウィンドウ番号  (窗口编号)
 *
 */
static BOOL NPC_Riderman_readData(int meindex, int windowno, BOOL chkflg) {

	int i;
	int linenum = 0;
	int endflg = FALSE;
	int buttonendflg;
	int winno = -1;
	int buttonconfmode;
	int b_mode;
	int selectnum;
	int messagepos;
	BOOL errflg = FALSE;
	BOOL readflg = TRUE;
	FILE *fp;
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char filename[64];
	char opfile[128];
	char line[1024];
	char firstToken[1024];
	char secondToken[1024];

	/* ウィンドウの設定を読み込む  (读取窗口设定) */

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));
	/* 設定ファイル取得  (获取设定文件) */
	NPC_Util_GetStrFromStrWithDelim(argstr, "conff", filename, sizeof(filename));

	sprintf(opfile, "%s/", getNpcdir());
	strcat(opfile, filename);

	fp = fopen(opfile, "r");
	if (fp == NULL) {
		print("riderman:file open error [%s]\n", opfile);
		return FALSE;
	}

	while (readflg == TRUE) {
		endflg = FALSE;
		buttonendflg = TRUE;
		buttonconfmode = FALSE;
		selectnum = 0;
		messagepos = 0;
		winno = -1;
		b_mode = -1;
		errflg = FALSE;

		/* 初期化  (初始化) */
		w.windowno = -1;
		w.windowtype = -1;
		w.buttontype = -1;
		w.takeitem = -1;
		w.giveitem = -1;
		w.takegold = 0;
		w.message[0] = '\0';

		for (i = 0; i < arraysizeof(buttonproc); i++) {
			buttonproc[i].use = FALSE;
			buttonproc[i].checkhaveitem = -1;
			buttonproc[i].checkhaveitemgotowin = -1;
			buttonproc[i].checkdonthaveitem = -1;
			buttonproc[i].checkdonthaveitemgotowin = -1;
			buttonproc[i].warp = -1;
			buttonproc[i].battle = -1;
			buttonproc[i].gotowin = -1;
		}

		while (1) {
			char buf[256];
			int ret;
			if (!fgets(line, sizeof(line), fp)) {
				readflg = FALSE;
				break;
			}

			linenum++;

			/* コメントは無視  (忽略注释) */
			if (line[0] == '#' || line[0] == '\n')
				continue;
			/* 数値を取る  (取数值) */
			chomp(line);

			/*  数値を整形する    (格式化数值) */
			/*  前の tab を " " に置き換える    (将tab替换为空格) */
			replaceString(line, '\t', ' ');
			/* 先のスペースを取る  (去掉前面的空格) */
			for (i = 0; i < strlen(line); i++) {
				if (line[i] != ' ') {
					break;
				}
				strcpy(buf, &line[i]);
			}
			if (i != 0)
				strcpy(line, buf);

			/* delim "=" で  初期(1)のトークンを取る  (用"="取第1个标记) */
			ret = getStringFromIndexWithDelim(line, "=", 1, firstToken,
											  sizeof(firstToken));
			if (ret == FALSE) {
				print("Find error at %s in line %d. Ignore\n",
					  filename, linenum);
				continue;
			}
			/* delim "=" で2番目のトークンを取る  (用"="取第2个标记) */
			ret = getStringFromIndexWithDelim(line, "=", 2, secondToken,
											  sizeof(secondToken));
			if (ret == FALSE) {
				print("Find error at %s in line %d. Ignore\n",
					  filename, linenum);
				continue;
			}

			if (strcasecmp(firstToken, "winno") == 0) {
				if (winno != -1) {
					print("riderman:已有winno却重新定义winno\n");
					print("filename:[%s] line[%d]\n", filename, linenum);
					errflg = TRUE;
					readflg = FALSE;
					break;
				}
				/* ウィンドウNoを保持  (保存窗口编号) */
				winno = atoi(secondToken);
				continue;
			}

			/* ウィンドウNoが決まっていない時の処理は無視する  (窗口编号未确定时忽略处理) */
			if (winno == -1) {
				print("riderman:winno 尚未定义，资料却已设定。\n");
				print("filename:[%s] line[%d]\n", filename, linenum);
				readflg = FALSE;
				errflg = FALSE;
				break;
			}
			/* ウィンドウNoが一致した時は条件を読む  (窗口编号一致时读取条件)
			 * それ以外は無視する  (其他情况忽略) */
			if ((chkflg == FALSE && winno == windowno) ||
				chkflg == TRUE) {
				if (buttonconfmode == TRUE) {
					if (strcasecmp(firstToken, "gotowin") == 0) {
						buttonproc[b_mode].gotowin = atoi(secondToken);
					} else if (strcasecmp(firstToken, "checkhaveitem") == 0) {
						buttonproc[b_mode].checkhaveitem = atoi(secondToken);
					} else if (strcasecmp(firstToken, "haveitemgotowin") == 0) {
						buttonproc[b_mode].checkhaveitemgotowin = atoi(secondToken);
					} else if (strcasecmp(firstToken, "checkdonthaveitem") == 0) {
						buttonproc[b_mode].checkdonthaveitem = atoi(secondToken);
					} else if (strcasecmp(firstToken, "donthaveitemgotowin") == 0) {
						buttonproc[b_mode].checkdonthaveitemgotowin = atoi(secondToken);
					} else if (strcasecmp(firstToken, "endbutton") == 0) {
						if (buttonproc[b_mode].gotowin == -1) {
							if (buttonproc[b_mode].checkhaveitem == -1 &&
								buttonproc[b_mode].checkdonthaveitem == -1) {
								errflg = TRUE;
							} else {
								/* どちらか片方だけでも設定されていれば  (只要任意一方被设置) */
								if (!((buttonproc[b_mode].checkhaveitem != -1 &&
									   buttonproc[b_mode].checkhaveitemgotowin != -1) ||
									  (buttonproc[b_mode].checkdonthaveitem != -1 &&
									   buttonproc[b_mode].checkdonthaveitemgotowin != -1))) {
									errflg = TRUE;
								}
							}
						}

						if (errflg == TRUE) {
							print("riderman: 找不到gotowin\n");
							print("filename:[%s] line[%d]\n", filename, linenum);
							readflg = FALSE;
							errflg = TRUE;
							break;
						}
						buttonproc[b_mode].use = TRUE;
						buttonconfmode = FALSE;
						buttonendflg = TRUE;
					}
				} else {

					w.windowno = winno;
					/* ウィンドウタイプの設定  (窗口类型的设定) */
					if (strcasecmp(firstToken, "wintype") == 0) {
						w.windowtype = atoi(secondToken);
					}
					/* ボタンタイプの設定  (按钮类型的设定) */
					else if (strcasecmp(firstToken, "buttontype") == 0) {
						w.buttontype = NPC_Riderman_restoreButtontype(secondToken);
					}
					/* getitemの設定  (getitem的设定) */
					else if (strcasecmp(firstToken, "takeitem") == 0) {
						w.takeitem = atoi(secondToken);
					}
					/* giveitemの設定  (giveitem的设定) */
					else if (strcasecmp(firstToken, "giveitem") == 0) {
						w.giveitem = atoi(secondToken);
					}

					/* takegold 学费 */
					else if (strcasecmp(firstToken, "takegold") == 0) {
						w.takegold = atoi(secondToken);
					}

					// Robin
					else if (strcasecmp(firstToken, "letter1") == 0) {
						w.letter[0] = atoi(secondToken);
					} else if (strcasecmp(firstToken, "letter2") == 0) {
						w.letter[1] = atoi(secondToken);
					} else if (strcasecmp(firstToken, "letter3") == 0) {
						w.letter[2] = atoi(secondToken);
					} else if (strcasecmp(firstToken, "letter4") == 0) {
						w.letter[3] = atoi(secondToken);
					}

					/* messageの設定  (message的设定) */
					else if (strcasecmp(firstToken, "message") == 0) {
						if (messagepos == 0) {
							strcpy(w.message, secondToken);
							messagepos = strlen(w.message);
						} else {
							w.message[messagepos] = '\n';
							messagepos++;
							strcpy(&w.message[messagepos], secondToken);
							messagepos += strlen(secondToken);
						}
					}
					/* ボタンを押した時の設定  (按下按钮时的设定) */
					else if (strcasecmp(firstToken, "okpressed") == 0) {
						buttonconfmode = TRUE;
						b_mode = 0;
						buttonendflg = FALSE;
					} else if (strcasecmp(firstToken, "cancelpressed") == 0) {
						buttonconfmode = TRUE;
						b_mode = 1;
						buttonendflg = FALSE;
					} else if (strcasecmp(firstToken, "yespressed") == 0) {
						buttonconfmode = TRUE;
						b_mode = 2;
						buttonendflg = FALSE;
					} else if (strcasecmp(firstToken, "nopressed") == 0) {
						buttonconfmode = TRUE;
						b_mode = 3;
						buttonendflg = FALSE;
					} else if (strcasecmp(firstToken, "prevpressed") == 0) {
						buttonconfmode = TRUE;
						b_mode = 4;
						buttonendflg = FALSE;
					} else if (strcasecmp(firstToken, "nextpressed") == 0) {
						buttonconfmode = TRUE;
						b_mode = 5;
						buttonendflg = FALSE;
					} else if (strcasecmp(firstToken, "selected") == 0) {
						buttonconfmode = TRUE;
						b_mode = 6 + selectnum;
						buttonendflg = FALSE;
						selectnum++;
					}
					/* 設定終わり  (设定结束) */
					else if (strcasecmp(firstToken, "endwin") == 0) {
						endflg = TRUE;
						if (chkflg == FALSE) {
							readflg = FALSE;
						}
						break;
					} else {
						print("riderman:设定是不可能的参数\n");
						print("filename:[%s] line[%d]\n", filename, linenum);
					}
				}
			} else {
				if (strcasecmp(firstToken, "endwin") == 0) {
					winno = -1;
				}
			}
		}
		if (buttonendflg == FALSE) {
			print("riderman: 找不到endbutton\n");
			print("filename:[%s] line[%d]\n", filename, linenum);
			errflg = TRUE;
			break;
		}
		if (winno != -1) {
			if (w.windowtype == -1) {
				print("riderman: 找不到wintype\n");
				print("filename:[%s] line[%d]\n", filename, linenum);
				errflg = TRUE;
				break;
			}
			if (w.buttontype == -1) {
				print("riderman: 找不到button\n");
				print("filename:[%s] line[%d]\n", filename, linenum);
				errflg = TRUE;
				break;
			}
			if (strlen(w.message) == 0) {
				print("riderman: 找不到message\n");
				print("filename:[%s] line[%d]\n", filename, linenum);
				errflg = TRUE;
				break;
			}
		}
	}
	fclose(fp);

	if (chkflg == FALSE && w.windowno == -1) {
		print("riderman: 找不到所指定的windowno\n");
		print("filename:[%s] line[%d]\n", filename, linenum);
		return FALSE;
	}
	if (winno != -1 && endflg == FALSE) {
		print("riderman: 找不到endwin\n");
		print("filename:[%s] line[%d]\n", filename, linenum);
		return FALSE;
	}
	if (errflg == TRUE)
		return FALSE;

	return TRUE;
}
/*
 * buttontype= で指定した値を数値に変換する  (将buttontype=指定的值转换为数值)
 *
 */
static int NPC_Riderman_restoreButtontype(char *data) {
	int ret = 0;
	int rc;
	int i;
	char buff[1024];

	for (i = 1;; i++) {
		rc = getStringFromIndexWithDelim(data, "|", i, buff,
										 sizeof(buff));
		if (rc == FALSE)
			break;
		if (strcasecmp(buff, "ok") == 0) {
			ret |= WINDOW_BUTTONTYPE_OK;
		} else if (strcasecmp(buff, "cancel") == 0) {
			ret |= WINDOW_BUTTONTYPE_CANCEL;
		} else if (strcasecmp(buff, "yes") == 0) {
			ret |= WINDOW_BUTTONTYPE_YES;
		} else if (strcasecmp(buff, "no") == 0) {
			ret |= WINDOW_BUTTONTYPE_NO;
		} else if (strcasecmp(buff, "prev") == 0) {
			ret |= WINDOW_BUTTONTYPE_PREV;
		} else if (strcasecmp(buff, "next") == 0) {
			ret |= WINDOW_BUTTONTYPE_NEXT;
		}
	}
	if (ret == 0) {
		ret = atoi(data);
	}
	return ret;
}

#if 0
static BOOL checkLetter(int meindex,int talker )
{
	int i;
	int itemindex=-1;
	int id;
	int itemNo = w.letter[CHAR_getInt( meindex, CHAR_FMINDEX ) -1];

	for( i=0;i<CHAR_MAXITEMHAVE;i++ ){

		itemindex=CHAR_getItemIndex( talker , i );
		if( ITEM_CHECKINDEX(itemindex) ){
			id=ITEM_getInt(itemindex ,ITEM_ID );
			if( itemNo == id )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

static BOOL getLetter(int meindex,int talker )
{
	int i;
	int itemindex=-1;
	int id;
	int itemNo = w.letter[CHAR_getInt( meindex, CHAR_FMINDEX ) -1];
	char buf[1024];

	for( i=0;i<CHAR_MAXITEMHAVE;i++ ){

		itemindex=CHAR_getItemIndex( talker , i );
		if( ITEM_CHECKINDEX(itemindex) ){
			id=ITEM_getInt(itemindex ,ITEM_ID );
			if( itemNo == id )
			{
				LogItem(
					CHAR_getChar( talker, CHAR_NAME ), /* キャラ   (角色) */
					CHAR_getChar( talker, CHAR_CDKEY ),
#ifdef _add_item_log_name // WON ADD 在item的log中增加item名称
					itemindex,
#else
					itemNo,  /* アイテム番号  (物品编号) */
#endif
					"RiderManDelItem",
					CHAR_getInt( talker, CHAR_FLOOR),
					CHAR_getInt( talker, CHAR_X ),
					CHAR_getInt( talker, CHAR_Y ),
					ITEM_getChar( itemindex, ITEM_UNIQUECODE),
					ITEM_getChar( itemindex, ITEM_NAME),
					ITEM_getInt( itemindex, ITEM_ID)
				);
				CHAR_DelItem( talker, i) ;
				return TRUE;
			}
		}
	}
	return FALSE;
}
#endif
