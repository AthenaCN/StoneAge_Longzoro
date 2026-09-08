#include "version.h"
#include <ctype.h>

#include "object.h"
#include "char_base.h"
#include "char.h"
#include "item.h"
#include "util.h"
#include "handletime.h"
#include "anim_tbl.h"
#include "npc_door.h"
#include "lssproto_serv.h"
#include "npcutil.h"
#include "npccreate.h"
#include "log.h"

/*
 *   簡単な店のルーチン  (简单商店的例程) by nakamura
 *
 * NPCARGUMENTに羅したアイテムを限定して扱っているお店  (仅处理NPC参数所列物品的商店)
 *     SIMPLESHOP_MAXINFINITITEM種限定で再生することができる  (可限量再生SIMPLESHOP_MAXINFINITITEM种)
 *   限定再生アイテムと同種のアイテムを持ったら持ったやつが  (若持有与限量再生物品同种的物品，则持有者)
 * 世界から消える  (从世界上消失)
 * またプレイヤーから物を買い取ることができる。ただしある段階  (也可从玩家处收购物品，但到一定阶段)
 * (NPC_SIMPLESHOPMAXBUYPRICE)より高い金を支払わなくてはいけ  (须支付高于NPC_SIMPLESHOPMAXBUYPRICE的金额)
 * ない 取はできない。買い取ったアイテムはその場でなくなる  (无法收购。收购的物品当场消失)
 * またアイテムの設定でcostが設定されていないものも買い取れない  (设定中未设置cost的物品也无法收购)
 * 2階以上のプレイヤーが店に来たときは先客を優先するようにする  (2楼以上玩家来店时优先先客)
 * ことにより店が永久に誰かを占有していることを防ぐ  (以此防止商店被某人永久占有)
 *
 * タイプ  “SimpleShop  (类型 “SimpleShop)
 * 扱うイベント: init, talked, specialtalked  (处理的事件：init、talked、specialtalked)
 *
 *
 */
static void NPC_SimpleShopOpenShopWindow(int meindex, int cliindex,
										 char *npcarg);
static void NPC_SimpleShopGetEscapedItemString(int shopindex,
											   int cliindex, char *str);
static BOOL NPC_SimpleShopProcessBuyMsg(char *msg, int shop, int cli);
static BOOL NPC_SimpleShopProcessSellMsg(char *msg, int shop, int cli);

static BOOL NPC_LimitBuyInShopProcessSellMsg(char *msg, int shop, int cli);

static BOOL NPC_SimpleShopFillItem(int meindex, char *npcarg);
static int NPC_SimpleShopGetItemNum(int meindex);
static void NPC_SimpleShopNormalWindow(int meindex,
									   int playerindex, char *str);
static int NPC_ShopWindowIndexToItemTableIndex(int charindex,
											   int shopwindowindex);
static BOOL NPC_SimpleShopOpenShopOrNot(char *msg, char *openshopstr);
// static void NPC_SimpleShopSetLeakLevel( int meindex );

#define NPC_SIMPLESHOPMAXBUYPRICE 9999
#define SIMPLESHOPTALKBUFSIZE 256

#define SIMPLESHOP_FATALERRSTR "好痛！对不起,最近状况不太佳。。。"

#define SIMPLESHOP_MAXINFINITITEM 40

enum {
	OPENSHOPTOKEN = 1,
	MSGTOKEN,
	MAINTOKEN,
	BUYTOKEN,
	OKBUYTOKEN,
	SELLTOKEN,
	OKSELLTOKEN,
	POORTOKEN,
	ITEMFULLTOKEN,
	ANOTHERPLAYERTOKEN,
	RAREITEMTOKEN,
	THANKYOUTOKEN,
	ITEMLISTTOKEN,

	BUYINITEMLISTTOKEN,
};
void NPC_SimpleShopTalked(int meindex, int talker, char *msg, int color) {
#define SHOPRANGE 3
	char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];
	if (CHAR_getInt(talker, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) {
		return;
	}
	if (NPC_Util_charIsInFrontOfChar(talker, meindex, SHOPRANGE) && !CHAR_getFlg(talker, CHAR_ISDIE)) {
		NPC_Util_GetArgStr(meindex, npcarg, sizeof(npcarg));
		getStringFromIndexWithDelim(npcarg, "|", OPENSHOPTOKEN, token, sizeof(token));
		if (NPC_SimpleShopOpenShopOrNot(msg, token) && NPC_Util_isFaceToFace(meindex, talker, SHOPRANGE)) {
			CHAR_setWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX, talker);
			NPC_SimpleShopOpenShopWindow(meindex, talker, npcarg);
		} else {
			int tokennum;
			int i;
			char tmp[NPC_UTIL_GETARGSTR_LINEMAX];
			getStringFromIndexWithDelim(npcarg, "|", MSGTOKEN, token, sizeof(token));
			tokennum = 1;
			for (i = 0; token[i] != '\0'; i++) {
				if (token[i] == ',')
					tokennum++;
			}
			getStringFromIndexWithDelim(token, ",", rand() % tokennum + 1, tmp, sizeof(tmp));
			CHAR_talkToCli(talker, meindex, tmp, CHAR_COLORWHITE);
		}
	}
}

void NPC_SimpleShopSpecialTalked(int meindex, int talker, char *msg, int color) {
	if (CHAR_getInt(talker, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER && NPC_Util_isFaceToFace(meindex, talker, SHOPRANGE)) {
		char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];
		NPC_Util_GetArgStr(meindex, npcarg, sizeof(npcarg));
		switch (tolower(msg[0])) {
		case 's': {
			int ret;
			if (CHAR_getWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX) != talker) {
				getStringFromIndexWithDelim(npcarg, "|", ANOTHERPLAYERTOKEN,
											token, sizeof(token));
				NPC_SimpleShopNormalWindow(meindex, talker, token);
				return;
			}
			ret = getStringFromIndexWithDelim(msg, "|", 3, token, sizeof(token));
			if (ret == FALSE) {
				token[0] = 'e';
				token[1] = '\0';
			}
			switch (tolower(token[0])) {
			case 'b':
				if ((ret = NPC_SimpleShopProcessBuyMsg(msg, meindex, talker)) < 0) {
					switch (ret) {
					case -2:
						getStringFromIndexWithDelim(npcarg, "|", POORTOKEN,
													token, sizeof(token));
						NPC_SimpleShopNormalWindow(meindex, talker, token);
						break;
					case -3:
						getStringFromIndexWithDelim(npcarg, "|",
													ITEMFULLTOKEN,
													token, sizeof(token));
						NPC_SimpleShopNormalWindow(meindex, talker, token);
						break;
					case -1:
					default:
						NPC_SimpleShopNormalWindow(meindex, talker,
												   SIMPLESHOP_FATALERRSTR);
						break;
					}
				} else {
					NPC_SimpleShopFillItem(meindex, npcarg);
					getStringFromIndexWithDelim(npcarg, "|", THANKYOUTOKEN,
												token, sizeof(token));
					NPC_SimpleShopNormalWindow(meindex, talker, token);
				}
				break;
			case 's':
				if (CHAR_getWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX) != talker) {
					getStringFromIndexWithDelim(npcarg, "|",
												ANOTHERPLAYERTOKEN,
												token, sizeof(token));
					NPC_SimpleShopNormalWindow(meindex, talker, token);
					return;
				}
				if ((ret = NPC_SimpleShopProcessSellMsg(msg, meindex, talker)) < 0) {
					switch (ret) {
					case -2:
						getStringFromIndexWithDelim(npcarg, "|",
													RAREITEMTOKEN,
													token, sizeof(token));
						NPC_SimpleShopNormalWindow(meindex, talker, token);
						break;
					case -1:
					default:
						NPC_SimpleShopNormalWindow(meindex, talker,
												   SIMPLESHOP_FATALERRSTR);
						break;
					}
				} else {
					getStringFromIndexWithDelim(npcarg, "|", THANKYOUTOKEN,
												token, sizeof(token));
					NPC_SimpleShopNormalWindow(meindex, talker, token);
				}
				break;
			case 'e':
			default:
				CHAR_setWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX, -1);
				break;
			}
			break;
		}
		case 'i':
			if (CHAR_getWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX) == talker) {
				NPC_SimpleShopOpenShopWindow(meindex, talker, npcarg);
			}
			break;
		default:
			break;
		}
	}
}

void NPC_LimitBuyInShopSpecialTalked(int meindex, int talker, char *msg, int color) {
	if (CHAR_getInt(talker, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER && NPC_Util_isFaceToFace(meindex, talker, SHOPRANGE)) {
		char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];
		NPC_Util_GetArgStr(meindex, npcarg, sizeof(npcarg));
		switch (tolower(msg[0])) {
		case 's': {
			int ret;
			if (CHAR_getWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX) != talker) {
				getStringFromIndexWithDelim(npcarg, "|",
											ANOTHERPLAYERTOKEN,
											token, sizeof(token));
				NPC_SimpleShopNormalWindow(meindex, talker, token);
				return;
			}
			ret = getStringFromIndexWithDelim(msg, "|", 3, token, sizeof(token));
			if (ret == FALSE) {
				token[0] = 'e';
				token[1] = '\0';
			}
			switch (tolower(token[0])) {
			case 'b':
				if ((ret = NPC_SimpleShopProcessBuyMsg(msg, meindex, talker)) < 0) {
					switch (ret) {
					case -2:
						getStringFromIndexWithDelim(npcarg, "|", POORTOKEN,
													token, sizeof(token));
						NPC_SimpleShopNormalWindow(meindex, talker, token);
						break;
					case -3:
						getStringFromIndexWithDelim(npcarg, "|",
													ITEMFULLTOKEN,
													token, sizeof(token));
						NPC_SimpleShopNormalWindow(meindex, talker, token);
						break;
					case -1:
					default:
						NPC_SimpleShopNormalWindow(meindex, talker,
												   SIMPLESHOP_FATALERRSTR);
						break;
					}
				} else {
					NPC_SimpleShopFillItem(meindex, npcarg);
					getStringFromIndexWithDelim(npcarg, "|", THANKYOUTOKEN,
												token, sizeof(token));
					NPC_SimpleShopNormalWindow(meindex, talker, token);
				}
				break;
			case 's':
				if (CHAR_getWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX) != talker) {
					getStringFromIndexWithDelim(npcarg, "|",
												ANOTHERPLAYERTOKEN,
												token, sizeof(token));
					NPC_SimpleShopNormalWindow(meindex, talker, token);
					return;
				}
				if ((ret = NPC_LimitBuyInShopProcessSellMsg(
						 msg, meindex, talker)) < 0) {
					switch (ret) {
					case -2:
						getStringFromIndexWithDelim(npcarg, "|",
													RAREITEMTOKEN,
													token, sizeof(token));
						NPC_SimpleShopNormalWindow(meindex, talker, token);
						break;
					case -1:
					default:
						NPC_SimpleShopNormalWindow(meindex, talker, SIMPLESHOP_FATALERRSTR);
						break;
					}
				} else {
					getStringFromIndexWithDelim(npcarg, "|", THANKYOUTOKEN, token, sizeof(token));
					NPC_SimpleShopNormalWindow(meindex, talker, token);
				}
				break;
			case 'e':
			default:
				CHAR_setWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX, -1);
				break;
			}
			break;
		}
		case 'i':
			if (CHAR_getWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX) == talker) {
				NPC_SimpleShopOpenShopWindow(meindex, talker, npcarg);
			}
			break;
		default:
			break;
		}
	}
}

/*
 * 店がノーマルウインドウを開く。ウインドウのボタンを押すと  (商店打开普通窗口，按下窗口按钮时)
 *   最初に店にしかけたときと同じ状態に戻る  (回到最初来店时相同的状态)
 * int meindex : 店のindex  (商店的index)
 * int playerindex : ウィンドウを開くプレイヤーのindex  (打开窗口的玩家index)
 * char *str :
 */
static void NPC_SimpleShopNormalWindow(int meindex, int playerindex,
									   char *str) {
	int fd;
	int objindex;
	char tmp[512], *name;

	objindex = CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX);
	name = CHAR_getChar(meindex, CHAR_NAME);
	snprintf(tmp, sizeof(tmp), "W|%d|N|%s|123|%s", objindex, name, str);
	fd = getfdFromCharaIndex(playerindex);
	if (fd == -1)
		return;
	lssproto_TK_send(fd, -1, tmp, CHAR_COLORWHITE);
}

/*
 * 店ウインドウのインデックスからアイテムテーブルのインデックスに 変換  (从商店窗口索引转换为物品表索引)
 * 娄醒
 * int charindex:キャラのインデックス  (角色索引)
 * int shopwindowindex:店ウインドウのインデックス。0から始まる  (商店窗口索引，从0开始)
 * 返り値  (返回值)
 * アイテムテーブルのインデックス。エラーは-1  (物品表索引，出错为-1)
 */
static int NPC_ShopWindowIndexToItemTableIndex(int charindex,
											   int shopwindowindex) {
	int i, counter;

	counter = 0;
	for (i = CHAR_STARTITEMARRAY; i < CHAR_MAXITEMHAVE; i++) {
		if (ITEM_CHECKINDEX(CHAR_getItemIndex(charindex, i))) {
			if (shopwindowindex == counter)
				return i;
			counter++;
		}
	}
	return -1;
}

static BOOL NPC_SimpleShopProcessBuyMsg(char *msg, int shopindex,
										int playerindex) {
	int shopwindowindex, itemtableindex, itemind;
	int price, tmpgold;
	char buf[64];

	if (!CHAR_CHECKINDEX(shopindex) || !CHAR_CHECKINDEX(playerindex))
		return -1;

	if (!getStringFromIndexWithDelim(msg, "|", 4, buf, sizeof(buf))) {
		return -1;
	}
	shopwindowindex = atoi(buf);
	itemtableindex = NPC_ShopWindowIndexToItemTableIndex(shopindex, shopwindowindex);
	itemind = CHAR_getItemIndex(shopindex, itemtableindex);
	if (!ITEM_CHECKINDEX(itemind))
		return -1;
	price = ITEM_getInt(itemind, ITEM_COST) * NPC_Util_buyRate(playerindex);

	tmpgold = CHAR_getInt(playerindex, CHAR_GOLD);
	if (tmpgold < price) {
		return -2;
	}

	if (!NPC_Util_moveItemToChar(playerindex, itemind, TRUE)) {
		return -3;
	}
	CHAR_AddGold(playerindex, price);

	CHAR_sendStatusString(playerindex, "P");
	return 1;
}

static BOOL NPC_SimpleShopProcessSellMsg(char *msg, int shopindex,
										 int playerindex) {
	int sellwindowindex, itemtableindex, itemind, id;
	int price, tmpgold;
	char buf[64];
	if (!CHAR_CHECKINDEX(shopindex) || !CHAR_CHECKINDEX(playerindex))
		return -1;
	if (!getStringFromIndexWithDelim(msg, "|", 4, buf, sizeof(buf))) {
		return -1;
	}
	sellwindowindex = atoi(buf);
	itemtableindex = sellwindowindex + CHAR_STARTITEMARRAY;
	itemind = CHAR_getItemIndex(playerindex, itemtableindex);
	if (!ITEM_CHECKINDEX(itemind))
		return -1;

	price = ITEM_getInt(itemind, ITEM_COST);

	if (price <= 0 || price > NPC_SIMPLESHOPMAXBUYPRICE) {
		return -2;
	}
	price *= NPC_Util_sellRate(playerindex);

	CHAR_AddGold(playerindex, price);

	CHAR_sendStatusString(playerindex, "P");

	id = ITEM_getInt(itemind, ITEM_ID);
	{
		LogItem(
			CHAR_getChar(playerindex, CHAR_NAME), /* キャラ   (角色) */
			CHAR_getChar(playerindex, CHAR_CDKEY),
#ifdef _add_item_log_name // WON ADD 在item的log中增加item名称
			itemind,
#else
			ITEM_getInt(itemind, ITEM_ID), /* アイテム番号  (物品编号) */
#endif
			"Sell",
			CHAR_getInt(playerindex, CHAR_FLOOR),
			CHAR_getInt(playerindex, CHAR_X),
			CHAR_getInt(playerindex, CHAR_Y),
			ITEM_getChar(itemind, ITEM_UNIQUECODE),
			ITEM_getChar(itemind, ITEM_NAME),
			ITEM_getInt(itemind, ITEM_ID));
	}

	if (!NPC_Util_moveItemToChar(shopindex, itemind, TRUE)) {
		return -1;
	}
	NPC_Util_RemoveItemByID(shopindex, id, FALSE);
	return 1;
}

static BOOL NPC_LimitBuyInShopProcessSellMsg(char *msg,
											 int shopindex, int playerindex) {
	int sellwindowindex, itemtableindex, itemind, id, i, buyokflag, setid;
	int price, tmpgold;
	char buf[BUFSIZ];
	char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];

	if (!CHAR_CHECKINDEX(shopindex) || !CHAR_CHECKINDEX(playerindex))
		return -1;
	if (!getStringFromIndexWithDelim(msg, "|", 4, buf, sizeof(buf))) {
		return -1;
	}
	sellwindowindex = atoi(buf);
	itemtableindex = sellwindowindex + CHAR_STARTITEMARRAY;
	itemind = CHAR_getItemIndex(playerindex, itemtableindex);
	if (!ITEM_CHECKINDEX(itemind))
		return -1;
	id = ITEM_getInt(itemind, ITEM_ID);
	NPC_Util_GetArgStr(shopindex, npcarg, sizeof(npcarg));
	getStringFromIndexWithDelim(npcarg, "|", BUYINITEMLISTTOKEN, token, sizeof(token));
	buyokflag = FALSE;
	for (i = 1; getStringFromIndexWithDelim(token, ",", i, buf, sizeof(buf)); i++) {

		char *tok1 = NULL;
		char *tok2 = NULL;
		int no1 = 0, no2 = 0;
		tok1 = strtok(buf, "-");
		tok2 = strtok(NULL, "-");

		setid = atoi(buf);

		if (tok1 == NULL || tok2 == NULL) {
			if (setid == 0)
				return -2;
			if (setid == id)
				buyokflag = TRUE;
		} else {
			no1 = atoi(tok1);
			no2 = atoi(tok2);
			if (no1 <= id && id <= no2) {
				buyokflag = TRUE;
			}
		}
	}
	if (buyokflag == FALSE)
		return -2;
	price = ITEM_getInt(itemind, ITEM_COST);
	price *= NPC_Util_sellRate(playerindex);

	CHAR_AddGold(playerindex, price);

	CHAR_sendStatusString(playerindex, "P");
	if (!NPC_Util_moveItemToChar(shopindex, itemind, TRUE)) {
		return -1;
	}

	{
		LogItem(
			CHAR_getChar(playerindex, CHAR_NAME), /* キャラ   (角色) */
			CHAR_getChar(playerindex, CHAR_CDKEY),
#ifdef _add_item_log_name // WON ADD 在item的log中增加item名称
			itemind,
#else
			ITEM_getInt(itemind, ITEM_ID), /* アイテム番号  (物品编号) */
#endif
			"Sell",
			CHAR_getInt(playerindex, CHAR_FLOOR),
			CHAR_getInt(playerindex, CHAR_X),
			CHAR_getInt(playerindex, CHAR_Y),
			ITEM_getChar(itemind, ITEM_UNIQUECODE),
			ITEM_getChar(itemind, ITEM_NAME),
			ITEM_getInt(itemind, ITEM_ID));
	}

	/*   買い取ったアイテムを消去  (删除收购的物品) */
	NPC_Util_RemoveItemByID(shopindex, id, FALSE);

	return 1;
}

/*
 * 店にあるアイテムの数を数える  (统计商店中物品数量)
 */
static int NPC_SimpleShopGetItemNum(int meindex) {
	int counter, i;

	counter = 0;
	for (i = CHAR_STARTITEMARRAY; i < CHAR_MAXITEMHAVE; i++) {
		/* アイテムを検索してもってるやつをカウントする  (检索物品并计数持有者) */
		if (ITEM_CHECKINDEX(CHAR_getItemIndex(meindex, i))) {
			counter++;
		}
	}
	return counter;
}

/*
 * あるキャラインデックスのキャラにおみせを開くための  (为某角色索引的角色打开店铺的)
 * 番号を与える  (赋予编号)
 * int meindex:自分の(店の)インデックス  (自己(商店)的索引)
 * int cliindex:客のインデックス  (客人的索引)
 */
static void NPC_SimpleShopOpenShopWindow(int meindex, int cliindex,
										 char *npcarg) {
	int fd;
	int objindex;

	char tmp[1024 * 12], itemstr[1024 * 8];

	char maincaption[SIMPLESHOPTALKBUFSIZE];
	char buycaption[SIMPLESHOPTALKBUFSIZE];
	char okbuycaption[SIMPLESHOPTALKBUFSIZE];
	char sellcaption[SIMPLESHOPTALKBUFSIZE];
	char oksellcaption[SIMPLESHOPTALKBUFSIZE];
	char poorcaption[SIMPLESHOPTALKBUFSIZE];
	char itemfullcaption[SIMPLESHOPTALKBUFSIZE];

	getStringFromIndexWithDelim(npcarg, "|", MAINTOKEN,
								maincaption, sizeof(maincaption));
	getStringFromIndexWithDelim(npcarg, "|", BUYTOKEN,
								buycaption, sizeof(buycaption));
	getStringFromIndexWithDelim(npcarg, "|", OKBUYTOKEN,
								okbuycaption, sizeof(okbuycaption));
	getStringFromIndexWithDelim(npcarg, "|", SELLTOKEN,
								sellcaption, sizeof(sellcaption));
	getStringFromIndexWithDelim(npcarg, "|", OKSELLTOKEN,
								oksellcaption, sizeof(oksellcaption));
	getStringFromIndexWithDelim(npcarg, "|", POORTOKEN,
								poorcaption, sizeof(poorcaption));
	getStringFromIndexWithDelim(npcarg, "|", ITEMFULLTOKEN,
								itemfullcaption, sizeof(itemfullcaption));

	tmp[0] = 0;
	NPC_SimpleShopGetEscapedItemString(meindex, cliindex, itemstr);
	objindex = CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX);
	snprintf(tmp, sizeof(tmp), "S|%d|%s|%s|%s|"
							   "%s|%s|%s|%s|"
							   "-1|%d%s",
			 objindex,
			 maincaption,
			 buycaption,
			 okbuycaption,
			 sellcaption,
			 oksellcaption,
			 poorcaption,
			 itemfullcaption,
			 NPC_SimpleShopGetItemNum(meindex),
			 itemstr);
	fd = getfdFromCharaIndex(cliindex);

	if (fd == -1)
		return;

	lssproto_TK_send(fd, -1, tmp, CHAR_COLORWHITE);
}

/*
 * TKで送るための  値  をつくる  (制作用TK发送的值)
 */
static void NPC_SimpleShopGetEscapedItemString(int shopindex,
											   int cliindex, char *str) {
	int i, cost, cl, imageno, itemindex;
	char *itemname;
	char tmp[1024];
	char escapedname[256];
	//    char *oli; /* oli means onelineinfo */

	str[0] = '\0';

	/* 前の店の  品  のリスト  (之前商店的商品列表) */
	for (i = CHAR_STARTITEMARRAY; i < CHAR_MAXITEMHAVE; i++) {
		itemindex = CHAR_getItemIndex(shopindex, i);
		if (ITEM_CHECKINDEX(itemindex)) {
			itemname = ITEM_getChar(itemindex, ITEM_SECRETNAME);
			makeEscapeString(itemname, escapedname, sizeof(escapedname));
			cost = ITEM_getInt(itemindex, ITEM_COST);
			/* 金額の調節。MERCHANTLEVELによって変わる  (金额调节，根据MERCHANTLEVEL变化) */
			cost *= NPC_Util_buyRate(cliindex);
			cl = ITEM_getInt(itemindex, ITEM_LEVEL);
			imageno = ITEM_getInt(itemindex, ITEM_BASEIMAGENUMBER);
			// oli =  ITEM_getMemo( itemindex );
#if 0
            snprintf( tmp, sizeof(tmp), "|%s|%d|%d|%d|%s",
                      escapedname, cost, cl, imageno, oli );
#endif
			snprintf(tmp, sizeof(tmp), "|%s|%d|%d|%d|",
					 escapedname, cost, cl, imageno);
			strcat(str, tmp);
		}
	}

	/* 客に見せる客の  品  のリスト  (展示给客人的商品列表) */
	for (i = CHAR_STARTITEMARRAY; i < CHAR_MAXITEMHAVE; i++) {
		itemindex = CHAR_getItemIndex(cliindex, i);
		if (ITEM_CHECKINDEX(itemindex)) {
			cost = ITEM_getInt(itemindex, ITEM_COST);
			/* 金額の調節。MERCHANTLEVELによって変わる  (金额调节，根据MERCHANTLEVEL变化) */
			cost *= NPC_Util_sellRate(cliindex);
			snprintf(tmp, sizeof(tmp), "|%d", cost);
			strcat(str, tmp);
		} else {
			/* アイテムを持ってない場所には0を入れる  (没有物品的位置填0) */
			strcat(str, "|0");
		}
	}
}

/*
 * 店が限定で持っているアイテムを出せる。初期化時と取り扱いの  (商店可拿出限量持有的物品。初始化时与处理时)
 * に呼ぶ  (时调用)
 */
static BOOL NPC_SimpleShopFillItem(int meindex, char *npcarg) {
	char token[256], buf[16];
	int i, itemid, num;

	getStringFromIndexWithDelim(npcarg, "|", ITEMLISTTOKEN, token,
								sizeof(token));

	for (i = 1; i <= SIMPLESHOP_MAXINFINITITEM; i++) {
		if (getStringFromIndexWithDelim(token, ",", i, buf, sizeof(buf))) {
			itemid = atoi(buf);
			if (itemid == 0) {
				NPC_Util_ReleaseHaveItemAll(meindex);
				return FALSE;
			}
			num = NPC_Util_countHaveItem(meindex, itemid);
			if (num < 0) {
				return FALSE;	   /* 不意なidなどのfatal err  (意外id等致命错误) */
			} else if (num == 0) { /* 持ってなかったら  エラー  (如果没有则报错) */
				NPC_Util_createItemToChar(meindex, itemid, FALSE);

			} else if (num >= 2) { /* 持ちすぎの場合セーブ  (持有过多时保存) */
				NPC_Util_RemoveItemByID(meindex, itemid, FALSE);
			}
		} else {
			break;
		}
	}

	/* 一括して区別された状態にする  (设为统一区分状态) */
	// NPC_SimpleShopSetLeakLevel( meindex );

	return TRUE;
}

/*
 * 初期化する  (初始化)
 */
BOOL NPC_SimpleShopInit(int meindex) {
	unsigned int mlevel;
	char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];

	/* 一括消失したあとは二度と戻らないようにする  (统一消失后使其不再恢复) */
	int createindex = CHAR_getInt(meindex, CHAR_NPCCREATEINDEX);

	/*    print("shopinit start\n" );*/
	if (NPC_CHECKCREATEINDEX(createindex)) {
		NPC_create[createindex].intdata[NPC_CREATETIME] = -1;
	}

	CHAR_setInt(meindex, CHAR_HP, 0);
	CHAR_setInt(meindex, CHAR_MP, 0);
	CHAR_setInt(meindex, CHAR_MAXMP, 0);
	CHAR_setInt(meindex, CHAR_STR, 0);
	CHAR_setInt(meindex, CHAR_TOUGH, 0);
	CHAR_setInt(meindex, CHAR_LV, 0);
	mlevel = ((100 << 16) | (100 << 0)); /* 上位2バイトが使うときの   (使用高位2字节时)
										  下位2バイトが使うときの   (使用低位2字节时) */
	CHAR_setInt(meindex, CHAR_MERCHANTLEVEL, mlevel);

	CHAR_setWorkInt(meindex, CHAR_WORKSHOPCLIENTINDEX, -1);

	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPESHOP);
	CHAR_setFlg(meindex, CHAR_ISOVERED, 0);
	CHAR_setFlg(meindex, CHAR_ISATTACKED, 0);

	NPC_Util_GetArgStr(meindex, npcarg, sizeof(npcarg));

	/* アイテムを出せる。失敗したらreturn FALSE  (可拿出物品，失败则返回FALSE) */
	if (!NPC_SimpleShopFillItem(meindex, npcarg)) {
		print("SHOP INIT ERROR: npcarg=%s\n", npcarg);
		return FALSE;
	}

	/*    print("shopinit end\n" );*/

	return TRUE;
}

/*
 * 店ウインドウを開くかどうか  決定する  (决定是否打开商店窗口)
 * npcargの中のあるトークンによって  (根据npcarg中的某个标记)
 * こんにちは、ご注文ください、くださいな  (你好、请点单、请)
 * のように羅された  値  をもとに  決定する。このときのデリミタは","  (根据如此列举的值决定，此时的分隔符是",")
 *
 * 娄醒
 * msg:プレイヤーがしゃべった  値  (玩家说出的值)
 * openshopstr:npcargの中のあるトークン  (npcarg中的某标记)
 * 返り値  (返回值)
 * 開くならTRUE, 開かないならFALSE  (开则TRUE，不开则FALSE)
 */
static BOOL NPC_SimpleShopOpenShopOrNot(char *msg, char *openshopstr) {
	int i;
	char buf[256];

	i = 1;
	while (getStringFromIndexWithDelim(openshopstr, ",", i, buf, sizeof(buf))) {
		if (strstr(msg, buf))
			return TRUE;
		i++;
	}
	return FALSE;
}

/*
 * すべての持ち物を一括識別された状態にする  (将全部持有物设为统一标识状态)
 */
#if 0
static void NPC_SimpleShopSetLeakLevel( int meindex )
{
    int i, itemindex;

    for( i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++ ){
        itemindex=CHAR_getItemIndex(meindex,i);
        if( ITEM_CHECKINDEX(itemindex)){
            ITEM_setInt(itemindex,ITEM_LEAKLEVEL, 1 );
        }
    }
}
#endif
