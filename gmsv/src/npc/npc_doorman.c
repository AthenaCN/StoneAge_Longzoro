#include "version.h"
#include <string.h>
#include "object.h"
#include "char_base.h"
#include "char.h"
#include "item.h"
#include "util.h"
#include "handletime.h"
#include "npc_doorman.h"
#include "npc_door.h"
#include "npcutil.h"
#include "configfile.h"
/*
 *
 *    扉にドアがあるときにそのドアを何らかの条件によって開くNPC.  (当门前有门时按某种条件打开该门的NPC)
 *  ドアのとなりにcreateするだけで、そのドアを開閉させることができる。  (只需在门旁创建即可开闭该门)
 *  はなしかけられたときに周囲キャラを検索して、まわり8マスにドアがいる  (被搭话时搜索周围角色，周围8格内有门)
 *  場合にはそのすべてに対して影響する。ふたつのドアが同時に開くことになる。  (则对其全部产生影响，两扇门会同时打开)
 *
 *  インターフェイスはTalkで  (界面是Talk)
 *
 * ドアを開くために  (为了开门)
 *
 * 1 お金を収集する。収集できたらひらく     gold|100|  (1 收取金钱。能收取则打开)
 * 2 アイテムを1個収集する。収集できたらひらく  item|45|  (2 收取1个道具。能收取则打开)
 * 3 アイテムを持っているかどうか調べる。持っていたら開く。itemhave|44|  (3 检查是否持有道具。持有则打开)
 * 4 アイテムを持っていないかどうか調べる。持っていなかったら開く。  (4 检查是否未持有。未持有则打开)
 *          itemnothave|333
 * 5 称号を持っているかどうか調べる。持っていたら開く。 titlehave|string|  (5 检查是否持有称号。持有则打开)
 * 6 称号を持っていないかどうか調べる。持っていなかったら開く。  (6 检查是否未持有称号。未持有则打开)
 *      titlenothave|string
 *
 * 必ず質問に答えると開く。金の扉は、  (必定回答问题后打开。金门是，)
 *＞100ゴールドいただきますがいいですか？＝で＞はい＝というと100ゴールド  (>收下100金可以吗？=若答>是=则100金)
 * とられる。いきなり＞はい＝だけ答えてもとられる。で、＞100ゴールド  (被拿走。突然只说>是=也会被拿走。接着>100金)
 * いただきました。＝と答われる。  (收下了。=如此回答。)
 *
 * アイテム収集の扉は、＞何★を一個いただきますがいいですか？＝ときく。  (道具收集门：>请问可以收下1个★吗？=询问。)
 *  3から6の扉は、何かはなしかけて条件がそろったら開く。  (3到6的门：搭话后条件齐备则打开)
 *
 *
 *
 *    セットの方法  (设置方法)
 *
 *1  ドアをてきとうに置く  (1 随便放置一扇门)
 *2  このNPCを隣にドアのとなりに置く。引数を gold|100 にする  (2 把这个NPC放在门旁边。参数设为gold|100)
 *3  このNPCに対して、100ゴールド動かしも持っている状態で＞はい＝と動かす  (3 对这个NPC，在能拿出100金的状态下选>是=则支付)
 *4  ドアがひらいて金が減ったらおしまい。  (4 门打开、金钱减少即结束)
 *
 */

static void NPC_DoormanOpenDoor(char *nm);

BOOL NPC_DoormanInit(int meindex) {
	char arg[NPC_UTIL_GETARGSTR_BUFSIZE];
	char dname[1024];

	/* イベントのタイプ設定  */
	CHAR_setWorkInt(meindex, CHAR_WORKEVENTTYPE, CHAR_EVENT_NPC);

	CHAR_setInt(meindex, CHAR_HP, 0);
	CHAR_setInt(meindex, CHAR_MP, 0);
	CHAR_setInt(meindex, CHAR_MAXMP, 0);
	CHAR_setInt(meindex, CHAR_STR, 0);
	CHAR_setInt(meindex, CHAR_TOUGH, 0);
	CHAR_setInt(meindex, CHAR_LV, 0);

	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPETOWNPEOPLE);
	CHAR_setFlg(meindex, CHAR_ISOVERED, 0);
	CHAR_setFlg(meindex, CHAR_ISATTACKED, 0); /*   攻撃されないよん  */

	NPC_Util_GetArgStr(meindex, arg, sizeof(arg));

	if (!getStringFromIndexWithDelim(arg, "|", 3, dname, sizeof(dname))) {
		print("RINGO: 设定看门者时需要门的名字唷！:%s:\n",
			  arg);
		return FALSE;
	}
	print("RINGO: Doorman create: arg: %s dname: %s\n", arg, dname);
	CHAR_setWorkChar(meindex, CHAR_WORKDOORMANDOORNAME, dname);

	return TRUE;
}

void NPC_DoormanTalked(int meindex, int talkerindex, char *msg,
					   int color) {
	char mode[128];
	char opt[256];
	char arg[NPC_UTIL_GETARGSTR_BUFSIZE];

	/* プレイヤーがドアマンの1グリッド移動なら反応する  */
	if (NPC_Util_CharDistance(talkerindex, meindex) > 1)
		return;

	NPC_Util_GetArgStr(meindex, arg, sizeof(arg));

	if (!getStringFromIndexWithDelim(arg, "|", 1, mode, sizeof(mode)))
		return;

	if (!getStringFromIndexWithDelim(arg, "|", 2, opt, sizeof(opt)))
		return;

	if (strcmp(mode, "gold") == 0) {
		int g = atoi(opt);
		int yn = NPC_Util_YN(msg);
		/*char *nm = CHAR_getChar( meindex , CHAR_NAME );*/
		char msg[256];

		if (g > 0 && yn < 0) {
			snprintf(msg, sizeof(msg),
					 "打开门需要给我%d的金子这样可以吗？", g);
			CHAR_talkToCli(talkerindex, meindex, msg, CHAR_COLORWHITE);
		} else if (g > 0 && yn == 0) {
			snprintf(msg, sizeof(msg),
					 "打开门 %d的金子是必要的。", g);
		} else if (g > 0 && yn == 1) {
			int now_g = CHAR_getInt(talkerindex, CHAR_GOLD);
			if (now_g < g) {
				snprintf(msg, sizeof(msg),
						 "打开门 %d的金子是必要的。", g);
				CHAR_talkToCli(talkerindex, meindex, msg, CHAR_COLORWHITE);
			} else {
				snprintf(msg, sizeof(msg),
						 "%d 收到金子了。现在就来开门。", g);
				CHAR_talkToCli(talkerindex, meindex, msg, CHAR_COLORWHITE);

				/* お金をゲット  */
				now_g -= g;
				CHAR_setInt(talkerindex, CHAR_GOLD, now_g);
				/* あたらしいステータスを送信  */
				CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_GOLD);

				/* ドアひらく  */
				NPC_DoormanOpenDoor(
					CHAR_getWorkChar(meindex, CHAR_WORKDOORMANDOORNAME));
			}
		}
	} else if (strcmp(mode, "item") == 0) {
		CHAR_talkToCli(talkerindex, meindex,
					   "尚在未支援模式。",
					   CHAR_COLORWHITE);
	} else if (strcmp(mode, "itemhave") == 0) {
		CHAR_talkToCli(talkerindex, meindex,
					   "尚在未支援模式。",
					   CHAR_COLORWHITE);
	} else if (strcmp(mode, "itemnothave") == 0) {
		CHAR_talkToCli(talkerindex, meindex,
					   "尚在未支援模式。",
					   CHAR_COLORWHITE);
	} else if (strcmp(mode, "titlehave") == 0) {
		CHAR_talkToCli(talkerindex, meindex,
					   "尚在未支援模式。",
					   CHAR_COLORWHITE);

	} else if (strcmp(mode, "roomlimit") == 0) {

		/*   部屋の定数制限がある場合  */
		char szOk[256], szNg[256], szBuf[32];
		int checkfloor;
		int maxnum, i, iNum;

		if (!getStringFromIndexWithDelim(arg, "|", 2, szBuf, sizeof(szBuf)))
			return;

		/* 調べるフラグと定数  */
		if (sscanf(szBuf, "%d:%d", &checkfloor, &maxnum) != 2) {
			return;
		}

		for (iNum = 0, i = 0; i < getFdnum(); i++) {
			/* プレイヤー動線上には障害がない  */
			if (CHAR_getCharUse(i) == FALSE)
				continue;
			if (CHAR_getInt(i, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER)
				continue;
			/* 指定のフラグ動線上に障害がない  */
			if (CHAR_getInt(i, CHAR_FLOOR) != checkfloor)
				continue;
			iNum++;
		}
		if (!getStringFromIndexWithDelim(arg, "|", 5, szNg, sizeof(szNg))) {
			strcpy(szNg, "。。。。"); /* 会話なしのセリフ  */
		}
		if (!getStringFromIndexWithDelim(arg, "|", 4, szOk, sizeof(szOk))) {
			strcpy(szOk, "开门吧。。。"); /* 会話ありのセリフ  */
		}

		if (iNum >= maxnum) {
			/*     条件を満たしている場合  */
			CHAR_talkToCli(talkerindex, meindex, szNg, CHAR_COLORWHITE);
		} else {
			/*     条件に合わない場合  */
			CHAR_talkToCli(talkerindex, meindex, szOk, CHAR_COLORWHITE);
			NPC_DoormanOpenDoor(
				CHAR_getWorkChar(meindex, CHAR_WORKDOORMANDOORNAME));
		}

	} else if (strcmp(mode, "titlenothave") == 0) {
		CHAR_talkToCli(talkerindex, meindex,
					   "尚在未支援模式。",
					   CHAR_COLORWHITE);
	}
}

/*
 *    前に検索してヒットしたのをすべて開く。  (在前面搜索，命中的全部打开)
 *
 */
static void NPC_DoormanOpenDoor(char *nm) {
	int doori = NPC_DoorSearchByName(nm);
	print("RINGO: Doorman's Door: index: %d\n", doori);

	NPC_DoorOpen(doori, -1);
}
