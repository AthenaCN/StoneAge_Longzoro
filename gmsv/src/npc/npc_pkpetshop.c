// krynn 2001/12/6
// PKPetShop

#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "pet_skill.h"
#include "readmap.h"
#include "log.h"
#include "enemy.h"
#include "npc_pkpetshop.h"
#include "battle.h"

#ifdef _PKPETSHOP
#define MAXSHOPPET 33

static void NPC_PKPetShop_selectWindow(int meindex, int talker, int num, int select);
void NPC_PKPetShop_BuyMain(int meindex, int talker, int before);
void NPC_PKPetShop_GetPetList(char *argstr, char *argtoken2);
BOOL NPC_PKPetShop_SetNewPet(int meindex, int talker, char *data);
BOOL NPC_PKPetShop_SellNewPet(int meindex, int talker, char *data);

void NPC_PKPetShop_Menu(int meindex, int talker);
int NPC_PKPetShop_GetLimtPetList(int talker, char *argstr, char *token2, int sell);
void NPC_PKPetShop_SellMain(int meindex, int talker, int select);
int NPC_GetSellPetList(int itemindex, int flg, char *argstr, char *argtoken, int select, int sell);
BOOL NPC_AddPetBuy(int meindex, int talker, int petID, int kosuu, double rate);
void NPC_PetStrStr(int petID, double rate, char *name, char *token2, int index);
int NPC_SellPetstrsStr(int itemindex, int flg, double rate, char *argtoken, int select, int sell);
void NPC_LimitPetShop(int meindex, int talker, int select);
void NPC_PKPetShop_ExpressmanCheck(int meindex, int talker);

/*--ワークの定義  (工作变量定义)--*/
enum {
	NPC_PKPETSHOP_WORK_NO = CHAR_NPCWORKINT1,
	NPC_PKPETSHOP_WORK_EV = CHAR_NPCWORKINT2,
	NPC_PKPETSHOP_WORK_EXPRESS = CHAR_NPCWORKINT3,
};

typedef struct {
	char arg[32];
	int type;
} PKPETSHOP_NPC_Shop;

static PKPETSHOP_NPC_Shop TypeTable[] = {
	{"FIST", 0},
	{"AXE", 1},
	{"CLUB", 2},
	{"SPEAR", 3},
	{"BOW", 4},
	{"SHIELD", 5},
	{"HELM", 6},
	{"ARMOUR", 7},
	{"BRACELET", 8},
	{"ANCLET", 9},
	{"NECKLACE", 10},
	{"RING", 11},
	{"BELT", 12},
	{"EARRING", 13},
	{"NOSERING", 14},
	{"AMULET", 15},
	{"OTHER", 16},
	{"BOOMERANG", 17},
	{"BOUNDTHROW", 18},
	{"BREAKTHROW", 19},
#ifdef _ITEM_TYPETABLE
	{"DISH", 20},
	{"METAL", 21},
	{"JEWEL", 22},
	{"WARES", 23},
	{"WBELT", 24},
	{"WSHIELD", 25},
	{"WSHOES", 26},
	{"WGLOVE", 27},
	{"ANGELTOKEN", 28},
	{"HEROTOKEN", 29},
#endif
	{"ACCESSORY", 30},
	{"OFFENCE", 40},
	{"DEFENCE", 50},

};

/*********************************
 * 初始
 *********************************/
BOOL NPC_PKPetShopInit(int meindex) {

	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

	/*--タイプ修正  (修正类型)--*/
	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPEPKPetShop);

	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("NPC_PKPetShopInit_GetArgStrErr");
		return FALSE;
	}
	if (strstr(argstr, "LIMITSHOP") != NULL) { /*-  制限ショップフラグ  (限制商店标志)--*/
		CHAR_setWorkInt(meindex, NPC_PKPETSHOP_WORK_NO, 1);
	} else {
		CHAR_setWorkInt(meindex, NPC_PKPETSHOP_WORK_NO, 0);
	}

	if (strstr(argstr, "EVENT") != NULL) { /*-  制限ショップフラグ  (限制商店标志)--*/
		CHAR_setWorkInt(meindex, NPC_PKPETSHOP_WORK_EV, 1);
	} else {
		CHAR_setWorkInt(meindex, NPC_PKPETSHOP_WORK_EV, 0);
	}

	if (strstr(argstr, "EXPRESS") != NULL) { /*-エクスプレスフラグ  (快递标志)--*/
		CHAR_setWorkInt(meindex, NPC_PKPETSHOP_WORK_EXPRESS, 1);
	} else {
		CHAR_setWorkInt(meindex, NPC_PKPETSHOP_WORK_EXPRESS, 0);
	}
	return TRUE;
}

/*********************************
 *   しかけられた時の値  (被设置时的值)
 *********************************/
void NPC_PKPetShopTalked(int meindex, int talker, char *szMes, int color) {
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buff[1024];
	char buf2[256];
	char token[1024];
	int i = 1;
	BOOL sellonlyflg = FALSE;
	char sellmsg[1024];

	/* プレイヤーかどうか調べる  (检查是否玩家) */
	if (CHAR_getInt(talker, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) {
		return;
	}

	/*--  の前にいるかどうか  (是否在前面)--*/
	if (NPC_Util_isFaceToFace(meindex, talker, 2) == FALSE) { /* グリッド移動のみ  (仅网格移动) */
		if (NPC_Util_CharDistance(talker, meindex) > 1)
			return;
	}

	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("NPC_PKPetShopInit_GetArgStrErr");
		return;
	}

	if (NPC_Util_GetStrFromStrWithDelim(argstr, "sellonly_msg",
										token, sizeof(token)) != NULL) {
		sellonlyflg = TRUE;
		strcpysafe(sellmsg, sizeof(sellmsg), token);
	}

	/*--  買いウィンドウに含まれるかどうか  (是否包含在购买窗口中)--*/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "buy_msg",
										buff, sizeof(buff)) != NULL) {
		while (getStringFromIndexWithDelim(buff, ",", i, buf2, sizeof(buf2)) != FALSE) {
			i++;
			if (strstr(szMes, buf2) != NULL) {
				if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_EV) == 0) {
					/*-  制限ショップかどうかのチェック  (检查是否为限制商店)--*/
					if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_NO) == 1) {
						if (sellonlyflg) {
							NPC_PKPetShop_selectWindow(meindex, talker, 3, -1);
							return;
						}
					} else {
						NPC_PKPetShop_selectWindow(meindex, talker, 1, -1);
						return;
					}
				} else {
					if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_NO) == 1) {
						if (sellonlyflg) {
							NPC_PKPetShop_selectWindow(meindex, talker, 3, -1);
							return;
						}
					} else {
						NPC_PKPetShop_selectWindow(meindex, talker, 1, -1);
						return;
					}
					return;
				}
			}
		}
	}
	i = 1;

	/*--  売りウィンドウに含まれるかどうか  (是否包含在出售窗口中)--*/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "sell_msg",
										buff, sizeof(buff)) != NULL) {
		while (getStringFromIndexWithDelim(buff, ",", i, buf2, sizeof(buf2)) != FALSE) {
			i++;
			if (strstr(szMes, buf2) != NULL) {
				NPC_PKPetShop_selectWindow(meindex, talker, 2, -1);
				return;
			}
		}
	}
	i = 1;

	/*--その他のメッセージを処理する時に呼ばれる  (处理其他消息时被调用)--*/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "other_msg",
										buff, sizeof(buff)) != NULL) {
		while (getStringFromIndexWithDelim(buff, ",", i, buf2, sizeof(buf2)) != FALSE) {
			i++;
			if (strstr(szMes, buf2) != NULL) {
				/*--ヒントメッセージ取得  (取得提示消息)--*/
				if (NPC_Util_GetStrFromStrWithDelim(argstr, "hint_msg",
													token, sizeof(token)) != NULL) {
					CHAR_talkToCli(talker, meindex, token, CHAR_COLORWHITE);
					return;
				}
			}
		}
	}

	/*-  制限ショップかどうかのチェック  (检查是否为限制商店)--*/
	if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_NO) == 1) {
		if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_EV) == 1) {
			if (sellonlyflg) {
				CHAR_talkToCli(talker, meindex, sellmsg, CHAR_COLORWHITE);
				return;
			}
		} else { /*--  制限ショップのメッセージ  (限制商店的消息)--*/
			if (sellonlyflg) {
				NPC_PKPetShop_selectWindow(meindex, talker, 3, -1);
				return;
			}
		}
	} else {
		if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_EV) == 1) {
			if (sellonlyflg) {
				CHAR_talkToCli(talker, meindex, sellmsg, CHAR_COLORWHITE);
				return;
			}
		} else {

			if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_EXPRESS) == 1) {
				NPC_PKPetShop_ExpressmanCheck(meindex, talker);
			} else {
				/*--ここで表示するウィンドウ(メニュー)  (此处显示的窗口(菜单))--*/
				/*--  制限ショップに設定されたメニューを開く  (打开限制商店设置的菜单)--*/
				NPC_PKPetShop_selectWindow(meindex, talker, 0, -1);
			}
		}
	}
}

static void NPC_PKPetShop_selectWindow(int meindex, int talker, int num, int select) {

	print("\n NPC_PKPetShop_selectWindow()");
	print("\n num = %d ", num);
	switch (num) {
	case 0:
		/*--メニュー  (菜单)    --*/
		/*--パラメータ送信  (发送参数)--*/
		CHAR_send_P_StatusString(talker, CHAR_P_STRING_GOLD);

		if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_EXPRESS) == 1) {
			if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_NO) == 0) {
				NPC_PKPetShop_ExpressmanCheck(meindex, talker);
			}
		} else if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_NO) == 1) {
		} else {
			NPC_PKPetShop_Menu(meindex, talker);
		}
		break;

	case 1:
		/*--  買う  (购买)    --*/
		NPC_PKPetShop_BuyMain(meindex, talker, select);
		break;

	case 2:
		/*--  売る  (出售)    --*/
		NPC_PKPetShop_SellMain(meindex, talker, select);
		break;

	case 3:
		/*--  制限ショップを開く  (打开限制商店)    --*/
		NPC_LimitPetShop(meindex, talker, select);
		break;
	}
}

/*-----------------------------------------
 * クライアントから返ってきた時に呼び出される  (客户端返回时被调用)［
 *
-------------------------------------------*/
void NPC_PKPetShopWindowTalked(int meindex, int talkerindex,
							   int seqno, int select, char *data) {
	/*--       離れている時は反応しない  (离开时不响应)   --*/
	if (NPC_Util_CharDistance(talkerindex, meindex) > 3) {
		/*--パラメータ送信  (发送参数)--*/
		CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_GOLD);
		return;
	}

	makeStringFromEscaped(data);
	switch (seqno) {

	case CHAR_WINDOWTYPE_WINDOWITEMSHOP_STARTMSG:
		/*--メニューに戻る  (返回菜单)--*/
		/*--  買う  (购买)--*/
		if (atoi(data) == 1)
			NPC_PKPetShop_selectWindow(meindex, talkerindex, 1, -1);

		/*--  売る  (出售)--*/
		if (atoi(data) == 2)
			NPC_PKPetShop_selectWindow(meindex, talkerindex, 2, -1);

		/*--何もしない  (什么也不做)--*/
		if (atoi(data) == 3)
			return; /*--終了する  (结束)--*/

		break;

	case CHAR_WINDOWTYPE_WINDOWITEMSHOP_BUY_MSG:
		/*--アイテムの購入  (购买物品)--*/
		if (NPC_PKPetShop_SetNewPet(meindex, talkerindex, data) == TRUE) {

			NPC_PKPetShop_selectWindow(meindex, talkerindex, 1, 0);

		} else {
			NPC_PKPetShop_selectWindow(meindex, talkerindex, 0, -1);
		}

		break;

	case CHAR_WINDOWTYPE_WINDOWITEMSHOP_SELL_MSG:
		/*--アイテムの売却  (出售物品)--*/
		if (NPC_PKPetShop_SellNewPet(meindex, talkerindex, data) == TRUE) {
			NPC_PKPetShop_selectWindow(meindex, talkerindex, 2, 0);

		} else {
			NPC_PKPetShop_selectWindow(meindex, talkerindex, 0, -1);
		}

		break;

	case CHAR_WINDOWTYPE_WINDOWITEMSHOP_LIMIT:
		if (select == WINDOW_BUTTONTYPE_YES) {
			NPC_PKPetShop_selectWindow(meindex, talkerindex, 2, -1);

		} else if (select == WINDOW_BUTTONTYPE_NO) {
			return;
		} else if (select == WINDOW_BUTTONTYPE_OK) {
			NPC_PKPetShop_selectWindow(meindex, talkerindex, 2, -1);
		}
		break;

	case CHAR_WINDOWTYPE_WINDOWITEMSHOP_EXPRESS:
		if (atoi(data) == 2) {
			NPC_PKPetShop_selectWindow(meindex, talkerindex, 1, -1);
		} else if (atoi(data) == 4) {
			NPC_PKPetShop_selectWindow(meindex, talkerindex, 2, -1);
		}
	}
}

/*-----------------------------------------
 *
 *  買いメイン(クライアント送信先の作成  (购买主界面(客户端发送目标的创建)
 *
 *krynn 2001/12/9 加的注释
 *before == -1
 *before <> -1
 *krynn end
 *-----------------------------------------*/
void NPC_PKPetShop_BuyMain(int meindex, int talker, int before) {
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char token[NPC_UTIL_GETARGSTR_BUFSIZE];
	int fd = getfdFromCharaIndex(talker);

	/*
	  イベントフラグ、前回のパラメータ送信フラグ、前回のメッセージ|購入メッセージ  (事件标志、上次的参数发送标志、上次的消息|购买消息)
	  アイテム選択メッセージ、レベル制限メッセージ、割引メッセージ  (物品选择消息、等级限制消息、折扣消息)
	  何かをする時、種別フラグ、アイテムレベル、アイテム名、画像、パラメータ  (执行操作时、类别标志、物品等级、物品名、图像、参数)
	  何かをする時、種別フラグ、アイテムレベル、アイテム名、画像、パラメータ  (执行操作时、类别标志、物品等级、物品名、图像、参数)
	*/

	/*--設定データ、その他のデータが取得できない時は反応しない  (设置数据、其它数据无法取得时不响应)  --*/
	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("itemshop_GetArgStr_Err");
		return;
	}

	/*--前のデータ送信  (发送之前的数据)---*/
	if (before != -1) { /*--前のデータが変更している時の次の値を送信  (之前数据变更时发送下一个值)    --*/
		sprintf(token, "0|0");

		lssproto_WN_send(fd, WINDOW_MESSAGETYPE_ITEMSHOPMAIN,
						 WINDOW_BUTTONTYPE_NONE,
						 CHAR_WINDOWTYPE_WINDOWITEMSHOP_BUY_MSG,
						 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
						 token);
	} else {

		char token2[NPC_UTIL_GETARGSTR_BUFSIZE];
		char buff2[256];
		char buff[256];

		/*--メッセージを取得しクライアントに送るデータを作成  (取得消息并生成发送给客户端的数据)-*/
		/*--ここでは送信しない  (此处不发送)--*/
		NPC_Util_GetStrFromStrWithDelim(argstr, "main_msg", buff, sizeof(buff));
		NPC_Util_GetStrFromStrWithDelim(argstr, "buy_main", buff2, sizeof(buff2));
		sprintf(token, "0|1|%d|%s|%s|%s|", CHAR_WINDOWTYPE_WINDOWITEMSHOP_STARTMSG,
				CHAR_getChar(meindex, CHAR_NAME), buff, buff2);

		NPC_Util_GetStrFromStrWithDelim(argstr, "what_msg", buff, sizeof(buff));
		NPC_Util_GetStrFromStrWithDelim(argstr, "level_msg", buff2, sizeof(buff));
		snprintf(token2, sizeof(token2), "%s|%s", buff, buff2);

		/*--  データを  (数据)  --*/
		strncat(token, token2, sizeof(token));

		NPC_Util_GetStrFromStrWithDelim(argstr, "realy_msg", buff, sizeof(buff));
		NPC_Util_GetStrFromStrWithDelim(argstr, "itemfull_msg", buff2, sizeof(buff2));
		sprintf(token2, "|%s|%s", buff, buff2);

		/*--  データを  (数据)--*/
		strncat(token, token2, sizeof(token));
		strcpy(token2, "|");

		/*--アイテム情報の  (物品信息)      --*/
		NPC_PKPetShop_GetPetList(argstr, token2);
		// krynn 2001/12/12 bebug 用
		print("%s", token2);
		// end krynn

		/*--メッセージとアイテム情報を結合  (拼接消息与物品信息)  --*/
		strncat(token, token2, sizeof(token));
	}

	/*--  データをエスケープする  (转义数据)--*/
	//		makeEscapeString( token, escapedname, sizeof(escapedname));

	/*--ここで送信する  (在此发送)--*/
	lssproto_WN_send(fd, WINDOW_MESSAGETYPE_ITEMSHOPMAIN,
					 WINDOW_BUTTONTYPE_NONE,
					 CHAR_WINDOWTYPE_WINDOWITEMSHOP_BUY_MSG,
					 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
					 token);
}

/*------------------------------------------------------
 *アイテムのデータを作成(買い用)  (生成物品数据(购买用))
 *------------------------------------------------------*/
void NPC_PKPetShop_GetPetList(char *argstr, char *argtoken) {

	int i = 1;
	int tmp;
	int EnemyCnt;
	char *name;
	char buff2[256];
	char buff[NPC_UTIL_GETARGSTR_LINEMAX];
	char token2[NPC_UTIL_GETARGSTR_BUFSIZE];
	double rate = 1.0;
	int loopcnt = 0;

	/*-レートを取得する  (取得倍率)--*/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "buy_rate", buff2, sizeof(buff2)) != NULL) {
		rate = atof(buff2);
	}

	/*-  ペットリストを取得  (取得宠物列表)    --*/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "PetList", buff, sizeof(buff)) != NULL) {
		while (getStringFromIndexWithDelim(buff, ",", i, buff2, sizeof(buff2)) != FALSE) {
			i++;
			/*--修正データのアイテムが"-"で含まれているかのチェック  (检查修正数据中的物品是否含"-")--*/
			if (strstr(buff2, "-") == NULL) {
				/*--エネミーを探す  (查找怪物)      --*/
				// krynn 2001/12/10
				EnemyCnt = ENEMY_getEnemyNum();
				for (tmp = 0; tmp < EnemyCnt; tmp++) {
					if (ENEMY_getInt(tmp, ENEMY_ID) == atoi(buff2)) {
						print("\nNPC_PKPetShop_GetPetList: tmp = %d", tmp);
						break;
					}
				}
				if (tmp == EnemyCnt) {
					return;
				}
				name = ENEMY_getChar(tmp, ENEMY_NAME);
				// krynn end
				/*--  名前がNULLの時はスキップ  (名字为空时跳过)--*/
				if (name == NULL)
					continue;

				loopcnt++;
				if (loopcnt == MAXSHOPPET)
					break;

				/*--  ペットデータを作る  (生成宠物数据)--*/
				NPC_PetStrStr(atoi(buff2), rate, name, token2, tmp);

				/*--メッセージデータを結合  (拼接消息数据)--*/
				strncat(argtoken, token2, sizeof(token2));
			} else {
				return;
				/*krynn 2001/12/13 这段应该用不到了，先 mark，改为直接 return
				--アイテムが15-25の範囲に含まれるか  (物品是否在15-25范围内)--
				int start;
				int end;

				//-"-"で含まれた番号の数値を取得  (取得"-"分隔的编号数值)  --
				getStringFromIndexWithDelim( buff2, "-", 1, token2, sizeof(token2));
				start = atoi( token2);
				getStringFromIndexWithDelim( buff2, "-", 2 ,token2, sizeof(token2));
				end = atoi( token2);

				//--  逆になっている時入れ替える  (顺序颠倒时交换)
				if(start > end)
				{
					tmp = start;
					start = end;
					end = tmp;
				}

				end++;

				//--"-"で含まれた先のアイテムを取る  (取得"-"中先头的物品)--
				for(; start < end ; start++ )
				{
					//--エネミーを探す  (查找怪物)--

					name = ITEM_getNameFromNumber( start );
					//--  名前がNULLの時はスキップ  (名字为空时跳过)--
					if(name == NULL) continue;

					loopcnt++;
					if(loopcnt == MAXSHOPPET) break;

					//--ペットデータを作る  (生成宠物数据)--
					NPC_PetStrStr( start, rate, name, token2, tmp);

					//--メッセージデータを結合  (拼接消息数据)--
					strncat( argtoken, token2, sizeof(token2));
				}*/
			}
		}
	}
}

/*-----------------------------------------------------------------
	データ作成  (生成数据)
-------------------------------------------------------------------*/
void NPC_PetStrStr(int petID, double rate, char *name, char *token2, int index) {
	int i;
	int gold;
	int level;
	int graNo;
	int TempNo;
	int EnemyTempNum;
	char info[1024];
	// char tryItem[256];	// krynn 2001/12/12 只是要看 getItemInfoFromNumber 读出来的字串长什麽样的
	char escape[256] = {"PK Server 宠"};

	// krynn 2001/12/10 try
	// gold  = ITEM_getcostFromITEMtabl( itemID);
	// level = ITEM_getlevelFromITEMtabl( itemID);
	// graNo = ITEM_getgraNoFromITEMtabl( itemID);
	TempNo = ENEMY_getInt(index, ENEMY_TEMPNO);
	EnemyTempNum = ENEMYTEMP_getEnemyNum();
	for (i = 0; i < EnemyTempNum; i++) {
		if (ENEMYTEMP_getInt(i, E_T_TEMPNO) == TempNo) {
			break;
		}
	}
	if (i == EnemyTempNum) {
		return;
	}
	gold = RAND(0, 20);
	level = 0;
	graNo = ENEMYTEMP_getInt(i, E_T_IMGNUMBER);
	// print("\nPKPetShop::NPC_PetStrStr(): TempNo = %d ; graNo = %d",TempNo,graNo);
	/*--レートを掛ける  (乘以倍率)--*/
	gold = (int)(gold * rate);

	// strcpy( tryItem,ITEM_getItemInfoFromNumber( 10 ) );
	// print("PKPetShop::NPC_PetStrStr(): try = %s\n",tryItem);
	// krynn end

	makeEscapeString(escape, info, sizeof(info));

	makeEscapeString(name, escape, sizeof(escape));

	sprintf(token2, "%s|0|%d|%d|%d|%s|", escape, level, gold, graNo, info);
}

/*-------------------------------------------
 *(  丹)
 *クライアントから返ってきたデータを受け取る  (接收客户端返回的数据)
 *
 *------------------------------------------*/
BOOL NPC_PKPetShop_SetNewPet(int meindex, int talker, char *data) {

	char buf[1024];
	char buff2[128];
	int i = 1, j = 1;
	int select;
	int kosuu = 0;
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	double rate = 1.0;
	int gold = 0;
	int EmptyPetCnt = 0;

	/*--返ってきたパラメータを表示する  (显示返回的参数)--*/
	print("\nNPC_PKPetShop_SetNewPet: data = %s", data);
	getStringFromIndexWithDelim(data, "|", 1, buf, sizeof(buf));
	select = atoi(buf); // krynn 2001/12/10  select 是玩家传回要买的第几样
	print("\nNPC_PKPetShop_SetNewPet: select = %d", select);
	if (select == 0)
		return FALSE;
	getStringFromIndexWithDelim(data, "|", 2, buf, sizeof(buf));
	kosuu = atoi(buf);
	print("\nNPC_PKPetShop_SetNewPet: kosuu(玩家要买的数量) = %d", kosuu);

	if (kosuu <= 0)
		return FALSE;

	/* 新しいペットの空きを探す  (查找新宠物的空位) */
	if (!CHAR_CHECKINDEX(talker)) {
		return FALSE;
	}
	for (i = 0; i < CHAR_MAXPETHAVE; i++) {
		if (CHAR_getCharPet(talker, i) == -1) {
			EmptyPetCnt++;
		}
	}
	// 有 EmptyPetCnt 个宠物空位
	/* 空きの数  (空位数量) */
	print("\nNPC_PKPetShop_SetNewPet:EmptyPetCnt(玩家有的空位) = %d", EmptyPetCnt);
	if (EmptyPetCnt <= 0)
		return FALSE;
	if (EmptyPetCnt > CHAR_MAXPETHAVE) {
		EmptyPetCnt = CHAR_MAXPETHAVE;
	}
	if (kosuu > EmptyPetCnt) {
		kosuu = EmptyPetCnt;
	}

	// krynn 2001/12/10
	// 应该用不到，所以 mark 起来
	/*--アイテムのチェックをする  (检查物品)＂
	for( i = CHAR_STARTITEMARRAY ; i < CHAR_MAXITEMHAVE ; i++ ) {
		itemindex = CHAR_getItemIndex( talker , i );

		if( !ITEM_CHECKINDEX( itemindex) ) {
			kosuucnt++;
		 }
	}

	--修正したアイテムが対象アイテムに含まれていない時  (修正的物品不在目标物品中时)
	--対象を処理する  (处理目标)
	if( kosuucnt < kosuu){
		kosuu = kosuucnt;
	}
	--エラーかどうか  (是否错误)
	if(kosuucnt == 0 ){
		return FALSE;
	}
	krynn end */

	i = 1;

	/*--設定データ、その他のデータが取得できない時は反応しない  (设置数据、其它数据无法取得时不响应)  --*/
	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("shop_GetArgStr_Err");
		return FALSE;
	}

	/*---レートを取得しプールに1.0  (取得倍率存入池中1.0)1.0)-*/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "buy_rate", buf, sizeof(buf)) != NULL) {
		rate = atof(buf);
	}

	/*--アイテムの購入リストを作成  (生成物品购买列表)-*/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "PetList",
										buf, sizeof(buf)) != NULL) {
		while (getStringFromIndexWithDelim(buf, ",", j, buff2, sizeof(buff2)) != FALSE) {
			j++;
			/*--  "-""-"が含まれているかどうか  (是否包含"-")--*/
			if (strstr(buff2, "-") == NULL) { // krynn 2001/12/10 try
											  // if( ITEM_getcostFromITEMtabl(atoi(buff2)) !=-1) {
				//
				if (i == select) {
					/*---アイテムの作成  (生成物品)---*/
					/*--アイテムを作成  (生成物品)--*/
					if (NPC_AddPetBuy(meindex, talker, atoi(buff2), kosuu, rate) != TRUE) {
						return FALSE;
					}
					return TRUE;
				}
				i++;
				// krynn end}
			} else {
				/*--アイテムが15-25の範囲に含まれるか  (物品是否在15-25范围内)--*/
				int start;
				int end;

				/* "-"-で含まれた番号の数値を取得  (取得"-"分隔的编号数值)  --*/
				getStringFromIndexWithDelim(buff2, "-", 1, argstr, sizeof(argstr));
				start = atoi(argstr);
				getStringFromIndexWithDelim(buff2, "-", 2, argstr, sizeof(argstr));
				end = atoi(argstr);
				end++;

				/*--  逆になっている時入れ替える  (顺序颠倒时交换)**/
				if (start > end) {
					gold = start;
					start = end;
					end = gold;
				}

				/*--"-"-で含まれた先のアイテムを取る  (取得"-"中先头的物品)--*/
				for (; start < end; start++) {
					if (ITEM_getcostFromITEMtabl(start) != -1) {
						if (i == select) {
							/*---アイテムの作成  (生成物品)---*/
							/*--アイテムを作成  (生成物品)--*/
							if (NPC_AddPetBuy(meindex, talker, start, kosuu, rate) != TRUE) {
								return FALSE;
							}
							return TRUE;
						}
						i++;
					}
				}
			}
		}
	}

	return FALSE;
}

/*---------------------------------------------
 *アイテムの購入を作成  (生成物品购买)
 *--------------------------------------------*/
BOOL NPC_AddPetBuy(int meindex, int talker, int petID, int kosuu, double rate) {
	int i, j, k, index, EnemyCnt, UpLevel;
	int gold;
	int ret;
	int maxgold;
	int Grade = 0;
	char buf[1024];
	char msgbuf[64];
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

	// krynn 2001/12/15 get get's grade of this PKPetShop
	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("shop_GetArgStr_Err");
		return FALSE;
	}
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "Grade", buf, sizeof(buf)) != NULL) {
		Grade = atoi(buf);
	}
	// end krynn

	/*--レートを掛ける  (乘以倍率)--*/
	// krynn 2001/12/11 mark and change
	// gold = ITEM_getcostFromITEMtabl( itemID);
	gold = 4;
	// krynn end
	gold = (int)(gold * rate);
	maxgold = gold * kosuu;

	if (CHAR_getInt(talker, CHAR_GOLD) < maxgold)
		return FALSE;

	/*--お金を払う  (支付金钱)--*/
	CHAR_setInt(talker, CHAR_GOLD, CHAR_getInt(talker, CHAR_GOLD) - maxgold);

	EnemyCnt = ENEMY_getEnemyNum();
	for (index = 0; index < EnemyCnt; index++) {
		if (ENEMY_getInt(index, ENEMY_ID) == petID) {
			print("\nNPC_AddPetBuy: index = %d", index);
			break;
		}
	}
	if (index == EnemyCnt) {
		return FALSE;
	}

	/*--アイテムを作成  (生成物品)--*/
	for (i = 0; i < kosuu; i++) {
		if ((ret = ENEMY_createPetFromEnemyIndex(talker, index)) == -1) {
			return FALSE;
		};
		/******************/
		/* ペット情報を送信  (发送宠物信息) */
		/******************/
		// どこにあるか探す  (查找在哪里)
		for (j = 0; j < CHAR_MAXPETHAVE; j++) {
			if (CHAR_getCharPet(talker, j) == ret) {
				break;
			}
		}
		if (j == CHAR_MAXPETHAVE) {
			return FALSE;
		}
		if (CHAR_CHECKINDEX(ret) == TRUE) {
			CHAR_setMaxExpFromLevel(ret, Grade);
			UpLevel = CHAR_LevelUpCheck(ret, talker);
			for (k = 0; k < UpLevel; k++) {
				CHAR_PetLevelUp(ret);
				CHAR_PetAddVariableAi(ret, AI_FIX_PETLEVELUP);
			}
			CHAR_complianceParameter(ret);
			CHAR_setInt(ret, CHAR_HP, CHAR_getWorkInt(ret, CHAR_WORKMAXHP));

			snprintf(msgbuf, sizeof(msgbuf), "K%d", j);
			CHAR_sendStatusString(talker, msgbuf);

			snprintf(msgbuf, sizeof(msgbuf), "W%d", j);
			CHAR_sendStatusString(talker, msgbuf);
		}
	}
	CHAR_send_P_StatusString(talker, CHAR_P_STRING_GOLD);
	return TRUE;
}

/*----------------------------------------
 *メニュー  (菜单)□
 *----------------------------------------*/
void NPC_PKPetShop_Menu(int meindex, int talker) {
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char token[NPC_UTIL_GETARGSTR_LINEMAX];
	char buff[256];
	int fd = getfdFromCharaIndex(talker);

	/* 送信メッセージ  (发送消息)| */
	/*--設定データ、その他のデータが取得できない時は反応しない  (设置数据、其它数据无法取得时不响应)  --*/
	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("shop_GetArgStr_Err");
		return;
	}

	NPC_Util_GetStrFromStrWithDelim(argstr, "main_msg", buff, sizeof(buff));
	snprintf(token, sizeof(token), "%s|%s", CHAR_getChar(meindex, CHAR_NAME), buff);

	//	print("%s",escapedname);
	/*--ここで送信する  (在此发送)--*/
	// krynn 2001/12/10	这里的参数似乎可以延用 ItemShop 的 code，先用用看
	lssproto_WN_send(fd, WINDOW_MESSAGETYPE_ITEMSHOPMENU,
					 WINDOW_BUTTONTYPE_NONE,
					 CHAR_WINDOWTYPE_WINDOWITEMSHOP_STARTMSG,
					 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
					 token);
	// krynn end
}

/*-------------------------------------------
 *	  売りメイン(データの作成  (出售主界面(生成数据)
 *
 *-------------------------------------------*/
void NPC_PKPetShop_SellMain(int meindex, int talker, int before) {

	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char token[NPC_UTIL_GETARGSTR_BUFSIZE];
	int fd = getfdFromCharaIndex(talker);

	/*
	  イベントフラグ、前回のパラメータ送信フラグ、前回のメッセージ|売却メッセージ  (事件标志、上次的参数发送标志、上次的消息|出售消息)
	  お金に関するメッセージ、割引メッセージ  (金钱相关消息、折扣消息)
	  何かをする時、種別フラグ、アイテム情報、画像、パラメータ、アイテムID、個数  (执行操作时、类别标志、物品信息、图像、参数、物品ID、数量)
	  何かをする時、種別フラグ、アイテム情報、画像、パラメータ、アイテムID、個数  (执行操作时、类别标志、物品信息、图像、参数、物品ID、数量)
	*/

	/*--設定データ、その他のデータが取得できない時は反応しない  (设置数据、其它数据无法取得时不响应)  --*/
	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("shop_GetArgStr_Err");
		return;
	}

	/*--前のデータ送信  (发送之前的数据)---*/
	if (before != -1) {

		/*--前のデータが変更している時の次の値を送信  (之前数据变更时发送下一个值)    --*/
		sprintf(token, "1|0");
		lssproto_WN_send(fd, WINDOW_MESSAGETYPE_ITEMSHOPMAIN + CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_NO),
						 WINDOW_BUTTONTYPE_NONE,
						 CHAR_WINDOWTYPE_WINDOWITEMSHOP_SELL_MSG,
						 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
						 token);

	} else {

		char token2[NPC_UTIL_GETARGSTR_BUFSIZE];
		char buff2[256];
		char buff[256];

		/*--メッセージを取得しクライアントに送るデータを作成  (取得消息并生成发送给客户端的数据)-*/
		/*--ここでは送信しない  (此处不发送)--*/
		NPC_Util_GetStrFromStrWithDelim(argstr, "main_msg", buff, sizeof(buff));
		NPC_Util_GetStrFromStrWithDelim(argstr, "sell_main", buff2, sizeof(buff));
		sprintf(token, "1|1|%d|%s|%s|%s|", CHAR_WINDOWTYPE_WINDOWITEMSHOP_STARTMSG,
				CHAR_getChar(meindex, CHAR_NAME), buff, buff2);

		NPC_Util_GetStrFromStrWithDelim(argstr, "stone_msg", buff, sizeof(buff));

		if (CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_EXPRESS) == 1) {
			NPC_Util_GetStrFromStrWithDelim(argstr, "exrealy_msg", buff2, sizeof(buff2));
		} else {
			NPC_Util_GetStrFromStrWithDelim(argstr, "realy_msg", buff2, sizeof(buff2));
		}
		sprintf(token2, "%s|%s|", buff, buff2);

		/*--アイテム情報の  (物品信息)      --*/
		NPC_PKPetShop_GetLimtPetList(talker, argstr, token2, -1);

		/*--メッセージとアイテム情報を結合  (拼接消息与物品信息)  --*/
		strncat(token, token2, sizeof(token));

		/*--ここで送信する  (在此发送)--*/
		lssproto_WN_send(fd, WINDOW_MESSAGETYPE_ITEMSHOPMAIN + CHAR_getWorkInt(meindex, NPC_PKPETSHOP_WORK_NO),
						 WINDOW_BUTTONTYPE_NONE,
						 CHAR_WINDOWTYPE_WINDOWITEMSHOP_SELL_MSG,
						 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
						 token);
	}
}

/*--------------------------------

売却データが設定されているアイテムのデータを取得  (取得设置了出售数据的物品数据)

 *-------------------------------*/
int NPC_PKPetShop_GetLimtPetList(int talker, char *argstr, char *token2, int sell) {

	char token[NPC_UTIL_GETARGSTR_LINEMAX];
	char buff[NPC_UTIL_GETARGSTR_LINEMAX];
	char token3[NPC_UTIL_GETARGSTR_LINEMAX];
	int k = 0, i = 1, j = 0;
	int imax;
	int itemtype = 0;
	int itemindex;
	int okflg = 0;
	char buf[256];
	int flg = 0;
	int cost;

	if (sell == -1) {
		i = CHAR_STARTITEMARRAY;
		imax = CHAR_MAXITEMHAVE;
		flg = -1;
	} else {
		i = sell;
		imax = sell + 1;
		flg = 1;
	}

	/*---  対象のアイテムを取得する  (取得目标物品)［--*/
	/**  スペシャルアイテムを指定している時はレートで計算  (指定特殊物品时按倍率计算)--*/
	/*-- スペシャルレートのデータアイテム  (特殊倍率的数据物品)    --*/

	for (; i < imax; i++) {
		okflg = 0;
		itemindex = CHAR_getItemIndex(talker, i);

		if (ITEM_CHECKINDEX(itemindex)) {

			/*--アイテムのタイプが一致している時  (物品类型一致时)--*/
			if (NPC_Util_GetStrFromStrWithDelim(argstr, "LimitItemType",
												buff, sizeof(buff)) != NULL) {
				k = 1;
				while (getStringFromIndexWithDelim(buff, ",", k, token, sizeof(token)) != FALSE) {
#ifdef _ITEM_TYPETABLE
					int cmpmaxitem = sizeof(TypeTable) / sizeof(TypeTable[0]);
#endif
					k++;
#ifdef _ITEM_TYPETABLE
					for (j = 0; j < cmpmaxitem; j++) {
#else
					for (j = 0; j < ITEM_CATEGORYNUM + 3; j++) {
#endif
						if (strcmp(TypeTable[j].arg, token) == 0) {
							itemtype = TypeTable[j].type;
							if (ITEM_getInt(itemindex, ITEM_TYPE) == itemtype) {

								/*--  データを作成  (生成数据)--*/
								cost = NPC_GetSellPetList(itemindex, 0, argstr, token3, i, sell);
								if (cost != -1)
									return cost;
								strncat(token2, token3, sizeof(token3));
								okflg = 1;
							} else if (itemtype == 30) {
								if (8 <= ITEM_getInt(itemindex, ITEM_TYPE) && (ITEM_getInt(itemindex, ITEM_TYPE) <= 15)) {
									/*--  データを作成  (生成数据)--*/
									cost = NPC_GetSellPetList(itemindex, 0, argstr, token3, i, sell);
									if (cost != -1)
										return cost;
									strncat(token2, token3, sizeof(token3));
									okflg = 1;
								}
							} else if (itemtype == 40) {
								if ((0 <= ITEM_getInt(itemindex, ITEM_TYPE) && (ITEM_getInt(itemindex, ITEM_TYPE) <= 4)) || (17 <= ITEM_getInt(itemindex, ITEM_TYPE) && (ITEM_getInt(itemindex, ITEM_TYPE) <= 19))) {
									/*--  データを作成  (生成数据)--*/
									cost = NPC_GetSellPetList(itemindex, 0, argstr, token3, i, sell);
									if (cost != -1)
										return cost;
									strncat(token2, token3, sizeof(token3));
									okflg = 1;
								}
							} else if (itemtype == 50) {
								if (5 <= ITEM_getInt(itemindex, ITEM_TYPE) && (ITEM_getInt(itemindex, ITEM_TYPE) <= 7)) {
									/*--  データを作成  (生成数据)--*/
									cost = NPC_GetSellPetList(itemindex, 0, argstr, token3, i, sell);
									if (cost != -1)
										return cost;
									strncat(token2, token3, sizeof(token3));
									okflg = 1;
								}
							}
							break;
						}
					}

					if (okflg == 1)
						break;
				}
			}
			/*--タイプに設定されなかったアイテムかチェック  (检查是否为未按类型设置的物品)--*/
			if ((NPC_Util_GetStrFromStrWithDelim(argstr, "LimitItemNo",
												 buff, sizeof(buff)) != NULL) &&
				okflg == 0) {
				k = 1;
				while (getStringFromIndexWithDelim(buff, ",", k, token, sizeof(token)) != FALSE) {
					k++;
					/*--数値修正していない時はスキップ  (未做数值修正时跳过)-*/
					if (strstr(token, "-") == NULL && strcmp(token, "") != 0) {
						if (ITEM_getInt(itemindex, ITEM_ID) == atoi(token)) {
							/*--  データを作成  (生成数据)--*/
							cost = NPC_GetSellPetList(itemindex, 0, argstr, token3, i, sell);
							if (cost != -1)
								return cost;
							strncat(token2, token3, sizeof(token3));
							okflg = 1;
						}
					} else if (strstr(token, "-") != NULL) {
						int start;
						int end;
						int work;

						/*--  データを作成  (生成数据)--*/
						/* "-"-で含まれた番号の数値を取得  (取得"-"分隔的编号数值)  --*/
						getStringFromIndexWithDelim(token, "-", 1, buf, sizeof(buf));
						start = atoi(buf);
						getStringFromIndexWithDelim(token, "-", 2, buf, sizeof(buf));
						end = atoi(buf);

						/*--  逆になっている時入れ替える  (顺序颠倒时交换)**/
						if (start > end) {
							work = start;
							start = end;
							end = work;
						}

						end++;
						/*--"-"-で含まれた先のアイテムを取る  (取得"-"中先头的物品)--*/
						if ((start <= ITEM_getInt(itemindex, ITEM_ID)) && (ITEM_getInt(itemindex, ITEM_ID) < end)) {
							/*--  データを作成  (生成数据)--*/
							cost = NPC_GetSellPetList(itemindex, 0, argstr, token3, i, sell);
							if (cost != -1)
								return cost;
							strncat(token2, token3, sizeof(token3));
							okflg = 1;
						}
					}
				}
			}

			/*--  何も当てはまらない時  (什么都不符合时)--*/
			if (okflg == 0) {
				cost = NPC_GetSellPetList(itemindex, 1, argstr, token3, i, sell);
				if (sell != -1)
					return -1;
				strncat(token2, token3, sizeof(token3));
			}
		}
	}
	return -1;
}

/*----------------------------------------------------------

	クライアントに送信するデータの作成  (生成发送给客户端的数据)

 *----------------------------------------------------------*/
int NPC_GetSellPetList(int itemindex, int flg, char *argstr, char *argtoken, int select, int sell) {

	char buff[256];
	double rate = 0.2;
	char buff2[256];
	char buff3[64];
	int k = 1;
	int cost = -1;

	/*--スペシャルレート  (特殊倍率)--**/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "special_item", buff, sizeof(buff)) != NULL) {
		if (NPC_Util_GetStrFromStrWithDelim(argstr, "special_rate", buff2, sizeof(buff2)) != NULL) {
			rate = atof(buff2);
		} else {
			rate = 1.2;
		}

		while (getStringFromIndexWithDelim(buff, ",", k, buff2, sizeof(buff2)) != FALSE) {
			k++;

			if (strstr(buff2, "-") == NULL && strcmp(buff2, "") != 0) {
				if (ITEM_getInt(itemindex, ITEM_ID) == atoi(buff2)) {
					cost = NPC_SellPetstrsStr(itemindex, 0, rate, argtoken, select, sell);
					return cost;
				}
			} else if (strstr(buff2, "-") != NULL) {
				int start;
				int end;
				int work;

				/*--  データを作成  (生成数据)--*/
				/* "-"-で含まれた番号の数値を取得  (取得"-"分隔的编号数值)  --*/
				getStringFromIndexWithDelim(buff2, "-", 1, buff3, sizeof(buff3));
				start = atoi(buff3);
				getStringFromIndexWithDelim(buff2, "-", 2, buff3, sizeof(buff3));
				end = atoi(buff3);

				/*--  逆になっている時入れ替える  (顺序颠倒时交换)**/
				if (start > end) {
					work = start;
					start = end;
					end = work;
				}
				end++;

				/*--"-"-で含まれた先のアイテムを取る  (取得"-"中先头的物品)--*/
				if ((start <= ITEM_getInt(itemindex, ITEM_ID)) && (ITEM_getInt(itemindex, ITEM_ID) < end)) {

					cost = NPC_SellPetstrsStr(itemindex, 0, rate, argtoken, select, sell);
					return cost;
				}
			}
		}
	}

	/*--ノーマルレート  (普通倍率)--*/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "sell_rate", buff, sizeof(buff)) != NULL) {
		rate = atof(buff);
		cost = NPC_SellPetstrsStr(itemindex, flg, rate, argtoken, select, sell);
		return cost;
	}

	return cost;
}

/*------------------------------------------

   データを作成(売り用)  (生成数据(出售用)))

*------------------------------------------*/
int NPC_SellPetstrsStr(int itemindex, int flg, double rate, char *argtoken, int select, int sell) {
	int cost;
	char escapedname[256];
	char name[256];
	char *eff;

	cost = ITEM_getInt(itemindex, ITEM_COST);
	cost = (int)(cost * rate);

	if (sell != -1)
		return cost;

	// strcpy( escapedname, ITEM_getChar( itemindex, ITEM_NAME));
	strcpy(escapedname, ITEM_getChar(itemindex, ITEM_SECRETNAME));
	makeEscapeString(escapedname, name, sizeof(name));
	eff = ITEM_getChar(itemindex, ITEM_EFFECTSTRING);
	makeEscapeString(eff, escapedname, sizeof(escapedname));

	sprintf(argtoken, "%s|%d|%d|%d|%s|%d|",
			name,
			flg,
			cost,
			ITEM_getInt(itemindex, ITEM_BASEIMAGENUMBER),
			escapedname,
			select);

	return -1;
}

/*--------------------------------------------
 *
 *クライアントから返ってきたデータを受け取る  (接收客户端返回的数据)
 *
 *-------------------------------------------*/
BOOL NPC_PKPetShop_SellNewPet(int meindex, int talker, char *data) {
	int select;
	int cost;
	int oldcost;
	char token[32];
	int k;
	int itemindex;
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char token2[256];

	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("GetArgStrErr");
		return FALSE;
	}

	/*--返ってきたパラメータの取得  (取得返回的参数)--*/
	getStringFromIndexWithDelim(data, "|", 1, token, sizeof(token));
	select = atoi(token);
	getStringFromIndexWithDelim(data, "|", 2, token, sizeof(token));
	oldcost = atoi(token);

	if (select == 0)
		return FALSE;

	cost = NPC_PKPetShop_GetLimtPetList(talker, argstr, token2, select);

	/*--選択した価格と実際の価格のチェック  (检查所选价格与实际价格)□--*/
	if (oldcost != cost || cost == -1) {
		int fd = getfdFromCharaIndex(talker);
		char token[256];

		sprintf(token, "\n\n哎呀!对不起"
					   "\n\n对不起啊 ! 可不可以再选一次呢？");

		k = select;
		itemindex = CHAR_getItemIndex(talker, k);

		/*--ログの記録  (记录日志)  --*/
		if (itemindex != -1) {
			print("\n%s(%d,%d,%d):和选择的东西不同error([%s(%d)]要消失了）",
				  CHAR_getChar(talker, CHAR_NAME),
				  CHAR_getInt(talker, CHAR_FLOOR),
				  CHAR_getInt(talker, CHAR_X),
				  CHAR_getInt(talker, CHAR_Y),
				  ITEM_getChar(itemindex, CHAR_NAME),
				  ITEM_getInt(itemindex, ITEM_ID));
			LogItem(
				CHAR_getChar(talker, CHAR_NAME), /* キャラクター  (角色)   */
				CHAR_getChar(talker, CHAR_CDKEY),
#ifdef _add_item_log_name // WON ADD 在item的log中增加item名称
				itemindex,
#else
				ITEM_getInt(itemindex, ITEM_ID), /* アイテムID  (物品ID) */
#endif
				"SellErr",
				CHAR_getInt(talker, CHAR_FLOOR),
				CHAR_getInt(talker, CHAR_X),
				CHAR_getInt(talker, CHAR_Y),
				ITEM_getChar(itemindex, ITEM_UNIQUECODE),
				ITEM_getChar(itemindex, ITEM_NAME),
				ITEM_getInt(itemindex, ITEM_ID));
		} else {
			print("\n%s(%d,%d,%d):和选择的东西不同error(没有任何item存在）",
				  CHAR_getChar(talker, CHAR_NAME),
				  CHAR_getInt(talker, CHAR_FLOOR),
				  CHAR_getInt(talker, CHAR_X),
				  CHAR_getInt(talker, CHAR_Y));
			LogItem(
				CHAR_getChar(talker, CHAR_NAME), /* キャラクター  (角色)   */
				CHAR_getChar(talker, CHAR_CDKEY),
				-1, /* アイテムID  (物品ID) */
				"SellErr",
				CHAR_getInt(talker, CHAR_FLOOR),
				CHAR_getInt(talker, CHAR_X),
				CHAR_getInt(talker, CHAR_Y),
				"-1", "NULL", -1);
		}
		/*--ここで送信する  (在此发送)--*/
		lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
						 WINDOW_BUTTONTYPE_OK,
						 CHAR_WINDOWTYPE_WINDOWITEMSHOP_LIMIT,
						 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
						 token);

		return FALSE;
	}

	k = select;
	itemindex = CHAR_getItemIndex(talker, k);

	if (itemindex != -1) {
		{
			LogItem(
				CHAR_getChar(talker, CHAR_NAME), /* キャラクター  (角色)   */
				CHAR_getChar(talker, CHAR_CDKEY),
#ifdef _add_item_log_name // WON ADD 在item的log中增加item名称
				itemindex,
#else
				ITEM_getInt(itemindex, ITEM_ID), /* アイテムID  (物品ID) */
#endif
				"Sell",
				CHAR_getInt(talker, CHAR_FLOOR),
				CHAR_getInt(talker, CHAR_X),
				CHAR_getInt(talker, CHAR_Y),
				ITEM_getChar(itemindex, ITEM_UNIQUECODE),
				ITEM_getChar(itemindex, ITEM_NAME),
				ITEM_getInt(itemindex, ITEM_ID)

			);
		}
	}

	CHAR_DelItem(talker, k);
	CHAR_AddGold(talker, cost);
	CHAR_send_P_StatusString(talker, CHAR_P_STRING_GOLD);

	return TRUE;
}

void NPC_LimitPetShop(int meindex, int talker, int select) {

	int fd = getfdFromCharaIndex(talker);
	char token[NPC_UTIL_GETARGSTR_LINEMAX];
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[1024];

	/*--設定データ、その他のデータが取得できない時は反応しない  (设置数据、其它数据无法取得时不响应)  --*/
	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("shop_GetArgStr_Err");
		return;
	}

	/*--  制限ショップの送信メッセージ  (限制商店的发送消息)-*/
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "sellonly_msg", buf, sizeof(buf)) != NULL) {
		sprintf(token, "\n\n%s", buf);

		lssproto_WN_send(fd, WINDOW_MESSAGETYPE_MESSAGE,
						 WINDOW_BUTTONTYPE_YESNO,
						 CHAR_WINDOWTYPE_WINDOWITEMSHOP_LIMIT,
						 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
						 token);

	} else {
		CHAR_talkToCli(talker, meindex, "这是买卖专门店。", CHAR_COLORWHITE);
	}
	return;
}

/*--エクスプレスチェック  (快递检查)--*/
void NPC_PKPetShop_ExpressmanCheck(int meindex, int talker) {
	int fd = getfdFromCharaIndex(talker);
	char token[1024];
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[1024];

	/*--設定データ、その他のデータが取得できない時は反応しない  (设置数据、其它数据无法取得时不响应)  --*/
	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("shop_GetArgStr_Err");
		return;
	}

	/*--  制限ショップの送信メッセージ  (限制商店的发送消息)-*/
	NPC_Util_GetStrFromStrWithDelim(argstr, "main_msg", buf, sizeof(buf));
	sprintf(token, "4\n　　　　　　　%s\n\n%s"
				   "\n\n　　　　　＜  打工  ＞　　　"
				   "\n\n　　　　  ＜交付行李＞",
			CHAR_getChar(meindex, CHAR_NAME), buf);

	lssproto_WN_send(fd, WINDOW_MESSAGETYPE_SELECT,
					 WINDOW_BUTTONTYPE_CANCEL,
					 CHAR_WINDOWTYPE_WINDOWITEMSHOP_EXPRESS,
					 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
					 token);

	return;
}

#endif
// _PKPETSHOP
