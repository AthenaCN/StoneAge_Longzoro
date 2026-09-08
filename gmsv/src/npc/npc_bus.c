#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "npc_bus.h"
#include "handletime.h"

/*
 * バスを運行するNPC  (运行巴士的NPC)
 */

enum {
	NPC_WORK_ROUTETOX = CHAR_NPCWORKINT1,	/* どこへの座標  (去往的坐标)   */
	NPC_WORK_ROUTETOY = CHAR_NPCWORKINT2,	/* どこへの座標  (去往的坐标)   */
	NPC_WORK_ROUTEPOINT = CHAR_NPCWORKINT3, /* ルートポイント  (路线点) */
	NPC_WORK_ROUNDTRIP = CHAR_NPCWORKINT4,	/* 往復するかどうか(1"往復" 0"復路")  (是否往返(1"往返" 0"返程"))  */
	NPC_WORK_MODE = CHAR_NPCWORKINT5,
	NPC_WORK_CURRENTROUTE = CHAR_NPCWORKINT6,
	NPC_WORK_ROUTEMAX = CHAR_NPCWORKINT7,
	NPC_WORK_WAITTIME = CHAR_NPCWORKINT8,
	NPC_WORK_CURRENTTIME = CHAR_NPCWORKINT9,
	NPC_WORK_SEFLG = CHAR_NPCWORKINT10,
};

/* 表示メッセージのenum  (显示消息的enum) */
enum {
	NPC_BUS_MSG_GETTINGON,
	NPC_BUS_MSG_NOTPARTY,
	NPC_BUS_MSG_OVERPARTY,
	NPC_BUS_MSG_DENIEDITEM,
	NPC_BUS_MSG_ALLOWITEM,
	NPC_BUS_MSG_LEVEL,
	NPC_BUS_MSG_GOLD,
	NPC_BUS_MSG_EVENT,
	NPC_BUS_MSG_START,
	NPC_BUS_MSG_END,
};
typedef struct {
	char option[32];
	char defaultmsg[128];
} NPC_BUS_MSG;
NPC_BUS_MSG busmsg[] = {
	{"msg_gettingon", "PAON！（你无法於中途加入我们唷！）"},
	{"msg_notparty", "PAPAON！！无法以团队加入唷！"},
	{"msg_overparty", "PAON！！人数已满。"},
	{"msg_denieditem", "PAPAON！！我可不要这个道具！"},
	{"msg_allowitem", "哇喔~(想要那个道具啊!)"},
	{"msglevel", "PAPAON！！你的等级还不够唷！"},
	{"msg_stone", "PAPAON！！金钱不足唷！"},
	{"msg_event", "PAON！！你无法加入唷！"},
	{"msg_start", "哇喔~(出发进行)"},
	{"msg_end", "哇喔~(到罗)"}

};

static int NPC_BusSetPoint(int meindex, char *argstr);
static void NPC_BusSetDestPoint(int meindex, char *argstr);
static BOOL NPC_BusCheckDeniedItem(int meindex, int charaindex, char *argstr);
static BOOL NPC_BusCheckLevel(int meindex, int charaindex, char *argstr);
static int NPC_BusCheckStone(int meindex, int charaindex, char *argstr);
static void NPC_BusSendMsg(int meindex, int talkerindex, int tablenum);
static int NPC_BusGetRoutePointNum(int meindex, char *argstr);
static void NPC_Bus_walk(int meindex);

#define NPC_BUS_LOOPTIME 200

/* スタート時間リスト  (开始时间列表) */
#define NPC_BUS_WAITTIME_DEFAULT 180

#define NPC_BUS_WAITINGMODE_WAITTIME 5000

/*********************************
 * 初期化  (初始化)
 *********************************/
BOOL NPC_BusInit(int meindex) {
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	int i;
	char buf[256];
	int routenum;
	int waittime;
	int seflg;

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	/* 引数なしの関数のチェック  (检查无参数的函数) */
	routenum = NPC_Util_GetNumFromStrWithDelim(argstr, "routenum"); // 行驶路线数量

	if (routenum == -1) {
		print("npcbus:nothing routenum \n");
		return FALSE;
	}
	CHAR_setWorkInt(meindex, NPC_WORK_ROUTEMAX, routenum);

	for (i = 1; i <= routenum; i++) {
		char routetostring[64];
		snprintf(routetostring, sizeof(routetostring), "routeto%d", i);
		if (NPC_Util_GetStrFromStrWithDelim(argstr, routetostring, buf, sizeof(buf)) == NULL) {
			print("npcbus:nothing route to \n");
			return FALSE;
		}
	}
	waittime = NPC_Util_GetNumFromStrWithDelim(argstr, "waittime");

	if (waittime == -1)
		waittime = NPC_BUS_WAITTIME_DEFAULT;
	CHAR_setWorkInt(meindex, NPC_WORK_WAITTIME, waittime);

	seflg = NPC_Util_GetNumFromStrWithDelim(argstr, "seflg");

	if (seflg == -1)
		seflg = TRUE;
	CHAR_setWorkInt(meindex, NPC_WORK_SEFLG, seflg);

	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPEBUS);

	CHAR_setWorkInt(meindex, NPC_WORK_MODE, 0);
	CHAR_setWorkInt(meindex, NPC_WORK_ROUTEPOINT, 2);
	CHAR_setWorkInt(meindex, NPC_WORK_ROUNDTRIP, 0);
	CHAR_setWorkInt(meindex, NPC_WORK_CURRENTROUTE, 0);

	CHAR_setInt(meindex, CHAR_LOOPINTERVAL,
				NPC_BUS_WAITINGMODE_WAITTIME);

	/* 現在時間をセット  (设置当前时间) */
	CHAR_setWorkInt(meindex, NPC_WORK_CURRENTTIME, NowTime.tv_sec);

	for (i = 0; i < CHAR_PARTYMAX; i++) {
		CHAR_setWorkInt(meindex, CHAR_WORKPARTYINDEX1 + i, -1);
	}

	/* リストを比較する  (比较列表) */
	{
		int rev;
		int r = CHAR_getWorkInt(meindex, NPC_WORK_ROUTEMAX);
		CHAR_setWorkInt(meindex, NPC_WORK_CURRENTROUTE, RAND(1, r));
		// print( "route:%d\n",CHAR_getWorkInt( meindex, NPC_WORK_CURRENTROUTE));

		/*   スタート  (开始) */
		rev = NPC_Util_GetNumFromStrWithDelim(argstr, "reverse");

		if (rev == 1) {
			int num = NPC_BusGetRoutePointNum(meindex, argstr);

			if (num <= 0) {
				print("npcbus:真奇怪！\n");
				return FALSE;
			}
			CHAR_setWorkInt(meindex, NPC_WORK_ROUTEPOINT, num - 1);
			CHAR_setWorkInt(meindex, NPC_WORK_ROUNDTRIP, 1);
		}
		/* リストをセットする  (设置列表) */
		NPC_BusSetPoint(meindex, argstr);
		/* 設定先にする  (设为目的地) */
		NPC_BusSetDestPoint(meindex, argstr);
	}

	return TRUE;
}

/*********************************
 *   しかけられた時の値  (被设置时的值)
 *********************************/
void NPC_BusTalked(int meindex, int talkerindex, char *szMes,
				   int color) {
	int i;
	int partyflg = FALSE;

	/* プレイヤーかどうか調べる  (检查是否玩家) */
	if (CHAR_getInt(talkerindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) {
		return;
	}
	/* 戦闘のパスメンバーいるかどうか調べる  (战斗路径成员是否存在) */
	for (i = 0; i < CHAR_PARTYMAX; i++) {
		int index = CHAR_getWorkInt(meindex, CHAR_WORKPARTYINDEX1 + i);
		if (CHAR_CHECKINDEX(index)) {
			if (index == talkerindex) {
				partyflg = TRUE;
			}
		}
	}
	if (!partyflg) {
		// NPC_BusCheckJoinParty( meindex, talkerindex, TRUE);
	} else {
		if (CHAR_getWorkInt(meindex, NPC_WORK_MODE) == 0) {
			int i;
			//		#define NPC_BUS_DEBUGROUTINTG	"routingtable:"
			if (strstr(szMes, "出发") ||
				strstr(szMes, "出发") ||
				strstr(szMes, "Go") ||
				strstr(szMes, "go")) {
				CHAR_setWorkInt(meindex, NPC_WORK_MODE, 1);

				/* ループ関数の呼び出しを待機に変更する  (将循环函数的调用改为待机) */
				CHAR_setInt(meindex, CHAR_LOOPINTERVAL, NPC_BUS_LOOPTIME);
				/* SE   出すマンモスの叫び  (发出猛犸的叫声)   */
				if (CHAR_getWorkInt(meindex, NPC_WORK_SEFLG)) {
					CHAR_sendSEoArroundCharacter(
						CHAR_getInt(meindex, CHAR_FLOOR),
						CHAR_getInt(meindex, CHAR_X),
						CHAR_getInt(meindex, CHAR_Y),
						60,
						TRUE);
				}
				/* 待つ時のメッセージ  (等待时的消息)*/
				for (i = 1; i < CHAR_PARTYMAX; i++) {
					int partyindex = CHAR_getWorkInt(meindex, CHAR_WORKPARTYINDEX1 + i);
					if (CHAR_CHECKINDEX(partyindex)) {
						NPC_BusSendMsg(meindex, partyindex, NPC_BUS_MSG_START);
					}
				}
			}
		}
#if 0
		else if( strstr( szMes, "停止" )  ||
			strstr( szMes, "停止" )  ||
			strstr( szMes, "stop" )  ||
			strstr( szMes, "Stop" ))
		{
			CHAR_setWorkInt( meindex, NPC_WORK_MODE,2);

			/* ループ関数のポインタ配列を登録する  (注册循环函数指针数组)  */
			CHAR_setInt( meindex, CHAR_LOOPINTERVAL, 
						NPC_BUS_WAITINGMODE_WAITTIME);
		    /* 現在時間をセット  (设置当前时间) */
		    CHAR_setWorkInt( meindex, NPC_WORK_CURRENTTIME, NowTime.tv_sec);
		}
		else if( strstr( szMes, NPC_BUS_DEBUGROUTINTG )) {
			/* デバッグ  (调试) */
			char *p = strstr( szMes,NPC_BUS_DEBUGROUTINTG);
			char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

			NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
			if( p) {
				int a = atoi( p+strlen(NPC_BUS_DEBUGROUTINTG));
				if( a <0 ) a = 1;
				CHAR_setWorkInt( meindex, NPC_WORK_CURRENTROUTE, a);
			}
			//print( "route:%d\n",CHAR_getWorkInt( meindex, NPC_WORK_CURRENTROUTE));
			/* リストをセットする  (设置列表) */
			NPC_BusSetPoint( meindex, argstr);
		}
#endif
	}
}
/**************************************
 * ループ関数  (循环函数)
 **************************************/
void NPC_BusLoop(int meindex) {
	int i;
	switch (CHAR_getWorkInt(meindex, NPC_WORK_MODE)) {
	case 0:
		/* スタートの時の時間をチェックする  (开始时检查时间) */
		/* 時間が過ぎたので待つ  (时间已过所以等待) */
		if (CHAR_getWorkInt(meindex, NPC_WORK_CURRENTTIME) + CHAR_getWorkInt(meindex, NPC_WORK_WAITTIME) < NowTime.tv_sec) {
			/* SE   出すマンモスの叫び  (发出猛犸的叫声)   */
			if (CHAR_getWorkInt(meindex, NPC_WORK_SEFLG)) {
				CHAR_sendSEoArroundCharacter(
					CHAR_getInt(meindex, CHAR_FLOOR),
					CHAR_getInt(meindex, CHAR_X),
					CHAR_getInt(meindex, CHAR_Y),
					60,
					TRUE);
			}
			/* 待つ時のメッセージ  (等待时的消息)*/
			for (i = 1; i < CHAR_PARTYMAX; i++) {
				int partyindex = CHAR_getWorkInt(meindex, CHAR_WORKPARTYINDEX1 + i);
				if (CHAR_CHECKINDEX(partyindex)) {
					NPC_BusSendMsg(meindex, partyindex, NPC_BUS_MSG_START);
				}
			}

			CHAR_setWorkInt(meindex, NPC_WORK_MODE, 1);
			/* ループ関数の呼び出しを待機に変更する  (将循环函数的调用改为待机) */
			CHAR_setInt(meindex, CHAR_LOOPINTERVAL, NPC_BUS_LOOPTIME);
		}
		return;
	case 1:
		/* 待機  (待机) */
		NPC_Bus_walk(meindex);
	case 2:
		/* 反転しているスタート  (正在反转的开始) */
		/* 時間が過ぎたので待つ  (时间已过所以等待) */
		if (CHAR_getWorkInt(meindex, NPC_WORK_CURRENTTIME) + (CHAR_getWorkInt(meindex, NPC_WORK_WAITTIME) / 3) < NowTime.tv_sec) {
			CHAR_setWorkInt(meindex, NPC_WORK_MODE, 1);
			/* ループ関数の呼び出しを待機に変更する  (将循环函数的调用改为待机) */
			CHAR_setInt(meindex, CHAR_LOOPINTERVAL, NPC_BUS_LOOPTIME);
		}
		return;
	case 3:
		/* 失敗にするプロトコルの用スタートの際に  (设为失败、协议用时在开始时) これからここでデータを変更する  (接下来在这里修改数据)
		 */
		if (CHAR_getWorkInt(meindex, NPC_WORK_CURRENTTIME) + 3 < NowTime.tv_sec) {
			char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

			NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));
			/* ループ関数の呼び出しを待機にする  (将循环函数的调用改为待机) */
			CHAR_setInt(meindex, CHAR_LOOPINTERVAL,
						NPC_BUS_WAITINGMODE_WAITTIME);

			/* リストを比較する  (比较列表) */
			{
				int r = CHAR_getWorkInt(meindex, NPC_WORK_ROUTEMAX);
				CHAR_setWorkInt(meindex, NPC_WORK_CURRENTROUTE, RAND(1, r));
				// print( "route:%d\n",CHAR_getWorkInt( meindex, NPC_WORK_CURRENTROUTE));
			}
			/* 設定ポイント読込  (读取设定点) */
			CHAR_setWorkInt(meindex, NPC_WORK_ROUNDTRIP,
							CHAR_getWorkInt(meindex, NPC_WORK_ROUNDTRIP) ^ 1);

			/* 次のノードの分岐  (下一节点的分支) */
			/* ポイントは終端  (点是终点)   */
			if (CHAR_getWorkInt(meindex, NPC_WORK_ROUNDTRIP) == 1) {
				/* そのリストのノード数を  (该列表的节点数) */
				int num = NPC_BusGetRoutePointNum(meindex, argstr);
				CHAR_setWorkInt(meindex, NPC_WORK_ROUTEPOINT, num - 1);
			} else {
				CHAR_setWorkInt(meindex, NPC_WORK_ROUTEPOINT,
								CHAR_getWorkInt(meindex, NPC_WORK_ROUTEPOINT) + 1);
			}
			/* 次のノードの座標をセットする  (设置下一节点坐标) */
			NPC_BusSetPoint(meindex, argstr);
			/* 設定先にする  (设为目的地) */
			NPC_BusSetDestPoint(meindex, argstr);
			/* パーティを解散する  (解散队伍) */
			CHAR_DischargeParty(meindex, 0);
			/* 現在時間をセット  (设置当前时间) */
			CHAR_setWorkInt(meindex, NPC_WORK_CURRENTTIME, NowTime.tv_sec);
			/* モードクリア  (清除模式) */
			CHAR_setWorkInt(meindex, NPC_WORK_MODE, 0);
		}
		return;
	default:
		break;
	}
}
/**************************************
 * 待機  (待机)
 **************************************/
static void NPC_Bus_walk(int meindex) {
	POINT start, end;
	int dir;
	int ret;
	int i;

	/* 待機関係  (待机相关) */
	/* 失敗した時の値  (失败时的值)   */
	start.x = CHAR_getInt(meindex, CHAR_X);
	start.y = CHAR_getInt(meindex, CHAR_Y);
	end.x = CHAR_getWorkInt(meindex, NPC_WORK_ROUTETOX);
	end.y = CHAR_getWorkInt(meindex, NPC_WORK_ROUTETOY);

	/* 失敗した時の次のノードに  (失败时到下一节点) */
	if (start.x == end.x && start.y == end.y) {
		int add = 1;
		char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

		NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

		if (CHAR_getWorkInt(meindex, NPC_WORK_ROUNDTRIP) == 1) {
			add *= -1;
		}
		CHAR_setWorkInt(meindex, NPC_WORK_ROUTEPOINT,
						CHAR_getWorkInt(meindex, NPC_WORK_ROUTEPOINT) + add);
		if (NPC_BusSetPoint(meindex, argstr) == FALSE) {
			/*     に失敗  (失败)*/
			/* スタートにする  (设为开始) */
			CHAR_setWorkInt(meindex, NPC_WORK_MODE, 3);

			/* SE   出すマンモスの叫び  (发出猛犸的叫声)   */
			if (CHAR_getWorkInt(meindex, NPC_WORK_SEFLG)) {
				CHAR_sendSEoArroundCharacter(
					CHAR_getInt(meindex, CHAR_FLOOR),
					CHAR_getInt(meindex, CHAR_X),
					CHAR_getInt(meindex, CHAR_Y),
					60,
					TRUE);
			}
			/* した時のメッセージ  (时的消息)*/
			for (i = 1; i < CHAR_PARTYMAX; i++) {
				int partyindex = CHAR_getWorkInt(meindex, CHAR_WORKPARTYINDEX1 + i);
				if (CHAR_CHECKINDEX(partyindex)) {
					NPC_BusSendMsg(meindex, partyindex, NPC_BUS_MSG_END);
				}
			}
			/* 現在時間をセット  (设置当前时间) */
			CHAR_setWorkInt(meindex, NPC_WORK_CURRENTTIME, NowTime.tv_sec);
			return;
		} else {
			return;
		}
	}
	/*-------------------------------------------------------*/
	/* 待ちする値  (等待的值)   */

	/*   値を求める  (求值) */
	dir = NPC_Util_getDirFromTwoPoint(&start, &end);

	/* 動く場所の際パス用に使う  (移动场所时用作路径)   */
	end.x = CHAR_getInt(meindex, CHAR_X);
	end.y = CHAR_getInt(meindex, CHAR_Y);

	/* 逆向きになった時の際の値  (变成反向时的值)   */
	for (i = 0; i < 100; i++) {
		if (dir < 0) {
			dir = RAND(0, 7);
		}
		dir = NPC_Util_SuberiWalk(meindex, dir);
		if (dir >= 0 && dir <= 7)
			break;
	}

	if (dir >= 0 && dir <= 7) {
		/* 待機  (待机) */
		ret = CHAR_walk(meindex, dir, 0);

		if (ret == CHAR_WALKSUCCESSED) {
			/* 戦闘が終わったら仲間を待ちする  (战斗结束后等待同伴) */
			int i;
			for (i = 1; i < CHAR_PARTYMAX; i++) {
				int toindex = CHAR_getWorkInt(meindex, i + CHAR_WORKPARTYINDEX1);
				if (CHAR_CHECKINDEX(toindex)) {
					int parent_dir;
					/* 直の先で戻のス号の先から値を求める  (从前端/返回的编号前端求值) */
					/* 待機  (待机) */
					start.x = CHAR_getInt(toindex, CHAR_X);
					start.y = CHAR_getInt(toindex, CHAR_Y);
					parent_dir = NPC_Util_getDirFromTwoPoint(&start, &end);
					/* パラメータオプションを読む際に  (读取参数选项时) 次の直が前の直を修正するようにする  (使下一节点修正前一节点)
					 */
					end = start;
					if (parent_dir != -1) {
						CHAR_walk(toindex, parent_dir, 0);
					}
				}
			}
		}
	}
}
/**************************************
 * 次の場所をセットする  (设置下一场所)
 **************************************/
static int NPC_BusSetPoint(int meindex, char *argstr) {
	char buf[4096];
	char buf2[256];
	char buf3[256];
	int ret;
	char routetostring[64];

	snprintf(routetostring, sizeof(routetostring), "routeto%d",
			 CHAR_getWorkInt(meindex, NPC_WORK_CURRENTROUTE));

	if (NPC_Util_GetStrFromStrWithDelim(argstr, routetostring, buf, sizeof(buf)) == NULL) {
		print("npcbus:nothing route \n");
		return FALSE;
	}
	ret = getStringFromIndexWithDelim(buf, ";",
									  CHAR_getWorkInt(meindex, NPC_WORK_ROUTEPOINT),
									  buf2, sizeof(buf2));

	if (ret == FALSE)
		return FALSE;

	ret = getStringFromIndexWithDelim(buf2, ",", 1,
									  buf3, sizeof(buf3));
	if (ret == FALSE)
		return FALSE;
	CHAR_setWorkInt(meindex, NPC_WORK_ROUTETOX, atoi(buf3));

	ret = getStringFromIndexWithDelim(buf2, ",", 2,
									  buf3, sizeof(buf3));
	if (ret == FALSE)
		return FALSE;
	CHAR_setWorkInt(meindex, NPC_WORK_ROUTETOY, atoi(buf3));
	return TRUE;
}
/**************************************
 * route  route Noから名前が同じだったら名前を  (从route No、若名字相同则) 所有との合わせにセットする  (设为与所有匹配)
 **************************************/
static void NPC_BusSetDestPoint(int meindex, char *argstr) {
	char buf[256];
	char routename[256];

	snprintf(routename, sizeof(routename), "routename%d",
			 CHAR_getWorkInt(meindex, NPC_WORK_CURRENTROUTE));

	if (NPC_Util_GetStrFromStrWithDelim(argstr, routename, buf, sizeof(buf)) != NULL) {
		CHAR_setChar(meindex, CHAR_OWNTITLE, buf);
		CHAR_sendCToArroundCharacter(CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX));
	}
}
/**************************************
 * 指定したアイテムを使用しているかチェックする  (检查是否使用指定物品) 使用している場合拒否  (使用则拒绝)
 **************************************/
static BOOL NPC_BusCheckDeniedItem(int meindex, int charaindex, char *argstr) {
	char buf[1024];
	BOOL found = TRUE;

	if (NPC_Util_GetStrFromStrWithDelim(argstr, "denieditem", buf, sizeof(buf)) != NULL) {
		int i;
		int ret;
		for (i = 1;; i++) {
			int itemid;
			char buf2[64];
			int j;
			ret = getStringFromIndexWithDelim(buf, ",", i, buf2, sizeof(buf2));
			if (ret == FALSE)
				break;
			itemid = atoi(buf2);
			for (j = 0; j < CHAR_MAXITEMHAVE; j++) {
				int itemindex = CHAR_getItemIndex(charaindex, j);
				if (ITEM_CHECKINDEX(itemindex)) {
					if (ITEM_getInt(itemindex, ITEM_ID) == itemid) {
						found = FALSE;
						break;
					}
				}
			}
		}
	}
	return found;
}
/**************************************
 * 指定したアイテムを使用しているかチェックする  (检查是否使用指定物品) 使用していない場合拒否  (未使用则拒绝)
 **************************************/
BOOL NPC_BusCheckAllowItem(int meindex, int charaindex, BOOL pickupmode) {
	char buf[1024];
	BOOL found = TRUE;
	BOOL pickup = FALSE;
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	if (NPC_Util_GetStrFromStrWithDelim(argstr, "pickupitem", buf, sizeof(buf)) != NULL) {
		pickup = TRUE;
	}
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "allowitem", buf, sizeof(buf)) != NULL) {
		int i;
		int ret;
		for (i = 1;; i++) {
			int itemid;
			char buf2[64];
			int j;
			BOOL getflg;
			ret = getStringFromIndexWithDelim(buf, ",", i, buf2, sizeof(buf2));
			if (ret == FALSE)
				break;
			itemid = atoi(buf2);
			getflg = FALSE;
			for (j = 0; j < CHAR_MAXITEMHAVE; j++) {
				int itemindex = CHAR_getItemIndex(charaindex, j);
				if (ITEM_CHECKINDEX(itemindex)) {
					if (ITEM_getInt(itemindex, ITEM_ID) == itemid) {
						if (pickupmode && pickup && !getflg) {
							CHAR_DelItem(charaindex, j);
							getflg = TRUE;
						}
						break;
					}
				}
			}
			if (j == CHAR_MAXITEMHAVE) {
				found = FALSE;
				break;
			}
		}
	}
	return found;
}

/**************************************
 * 指定したレベル満たしかチェックする  (检查是否达到指定等级)
 **************************************/
static BOOL NPC_BusCheckLevel(int meindex, int charaindex, char *argstr) {
	int level;

	/* 引数なしの関数のチェック  (检查无参数的函数) */
	level = NPC_Util_GetNumFromStrWithDelim(argstr, "needlevel");
	if (level == -1) {
		return TRUE;
	}
	if (CHAR_getInt(charaindex, CHAR_LV) >= level)
		return TRUE;

	return FALSE;
}
/**************************************
 * お金をチェックする  (检查金钱) -1... 0動き...  ？必要なStone
 **************************************/
static int NPC_BusCheckStone(int meindex, int charaindex, char *argstr) {
	int gold;

	/* 引数なしの関数のチェック  (检查无参数的函数) */
	gold = NPC_Util_GetNumFromStrWithDelim(argstr, "needstone");
	if (gold == -1) {
		return 0;
	}
	if (CHAR_getInt(charaindex, CHAR_GOLD) >= gold)
		return gold;

	return -1;
}
/**************************************
 * メッセージを取る  (取消息) 関数のメッセージを引数にあるリストメッセージを取る  (取函数消息中参数列表的消息)
 **************************************/
static void NPC_BusSendMsg(int meindex, int talkerindex, int tablenum) {
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[256];
	char msg[256];
	if (tablenum < 0 || tablenum >= arraysizeof(busmsg))
		return;

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	if (NPC_Util_GetStrFromStrWithDelim(argstr, busmsg[tablenum].option, buf, sizeof(buf)) != NULL) {
		strcpy(msg, buf);
	} else {
		snprintf(msg, sizeof(msg), busmsg[tablenum].defaultmsg);
	}
	CHAR_talkToCli(talkerindex, meindex, msg, CHAR_COLORYELLOW);
}
/**************************************
 * リストの要素ノードの数を取得する  (获取列表元素节点数)
 **************************************/
static int NPC_BusGetRoutePointNum(int meindex, char *argstr) {
	int i;
	char buf[4096];
	char buf2[256];
	int ret;
	char routetostring[64];

	snprintf(routetostring, sizeof(routetostring), "routeto%d",
			 CHAR_getWorkInt(meindex, NPC_WORK_CURRENTROUTE));

	if (NPC_Util_GetStrFromStrWithDelim(argstr, routetostring, buf, sizeof(buf)) == NULL) {
		print("npcbus:nothing route \n");
		return -1;
	}
	for (i = 1;; i++) {
		ret = getStringFromIndexWithDelim(buf, ";", i, buf2, sizeof(buf2));
		if (ret == FALSE)
			break;
	}
	return (i - 1);
}
BOOL NPC_BusCheckJoinParty(int meindex, int charaindex, BOOL msgflg) {
	// int		fd;
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	int ret;
	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	/* グリッド移動のみ  (仅网格移动) */
	if (!NPC_Util_charIsInFrontOfChar(charaindex, meindex, 1))
		return FALSE;
	/*     メンバーか表示する  (显示是否成员) */
	if (CHAR_getWorkInt(meindex, NPC_WORK_MODE) != 0) {
		if (msgflg)
			NPC_BusSendMsg(meindex, charaindex, NPC_BUS_MSG_GETTINGON);
		return FALSE;
	}
	/* パーティ化していない場合拒否  (未组队则拒绝) */
	if (CHAR_getWorkInt(charaindex, CHAR_WORKPARTYMODE) != CHAR_PARTY_NONE) {
		if (msgflg)
			NPC_BusSendMsg(meindex, charaindex, NPC_BUS_MSG_NOTPARTY);
		return FALSE;
	}
	/* パスの人数をチェックする  (检查路径人数) */
	if (CHAR_getEmptyPartyArray(meindex) == -1) {
		if (msgflg)
			NPC_BusSendMsg(meindex, charaindex, NPC_BUS_MSG_OVERPARTY);
		return FALSE;
	}
	/* アイテムのチェックをする  (进行物品检查)(禁止アイテム) */
	if (!NPC_BusCheckDeniedItem(meindex, charaindex, argstr)) {
		if (msgflg)
			NPC_BusSendMsg(meindex, charaindex, NPC_BUS_MSG_DENIEDITEM);
		return FALSE;
	}
#ifdef _ITEM_CHECKWARES
	if (CHAR_CheckInItemForWares(charaindex, 0) == FALSE) {
		CHAR_talkToCli(charaindex, -1, "无法携带货物上车。", CHAR_COLORYELLOW);
		return FALSE;
	}
#endif
	/* アイテムのチェックをする  (进行物品检查)(必要アイテム) */
	if (!NPC_BusCheckAllowItem(meindex, charaindex, FALSE)) {
		if (msgflg)
			NPC_BusSendMsg(meindex, charaindex, NPC_BUS_MSG_ALLOWITEM);
		return FALSE;
	}
	/* レベルをチェックする  (检查等级) */
	if (!NPC_BusCheckLevel(meindex, charaindex, argstr)) {
		if (msgflg)
			NPC_BusSendMsg(meindex, charaindex, NPC_BUS_MSG_LEVEL);
		return FALSE;
	}
	/* イベントかチェックする  (检查事件) */
	//	if( CHAR_getInt( charaindex, CHAR_NOWEVENT) != 0 ||
	//		CHAR_getInt( charaindex, CHAR_NOWEVENT2) != 0 ||
	//		CHAR_getInt( charaindex, CHAR_NOWEVENT3) != 0 )
	//	{
	//		if( msgflg) NPC_BusSendMsg( meindex, charaindex, NPC_BUS_MSG_EVENT);
	//		return FALSE;
	//	}
	/* お金のチェックをして、金を取ったら終了チェックにする  (检查金钱、取钱后设为结束检查)   */
	ret = NPC_BusCheckStone(meindex, charaindex, argstr);
	if (ret == -1) {
		if (msgflg)
			NPC_BusSendMsg(meindex, charaindex, NPC_BUS_MSG_GOLD);
		return FALSE;
	}
	if (ret != 0) {
		char msgbuf[128];
		/* お金をとる  (拿钱) */
		CHAR_setInt(charaindex, CHAR_GOLD,
					CHAR_getInt(charaindex, CHAR_GOLD) - ret);
		/* 送信  (发送) */
		CHAR_send_P_StatusString(charaindex, CHAR_P_STRING_GOLD);
		snprintf(msgbuf, sizeof(msgbuf), "支付了%d Stone！", ret);
		CHAR_talkToCli(charaindex, -1, msgbuf, CHAR_COLORYELLOW);
	}
	/* パーティに加える  (加入队伍) */
	// CHAR_JoinParty_Main( charaindex, meindex);

	// fd = getfdFromCharaIndex( charaindex );

	// lssproto_PR_send( fd, 1, 1);

	return TRUE;
}
