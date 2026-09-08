#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "npc_windowhealer.h"

/* ボディランゲージを覚えるNPC  (学会肢体语言的NPC)
 * しかけられたらそのプレイヤーにこのNPCのインデックスを保持する  (被触发时让该玩家持有本NPC的索引)
 * 周りでアクションがあったらアクションを起こしたプレイヤーが  (周围有动作时，发起动作的玩家)
 * NPCのインデックスを保持しているかチェック  (检查是否持有本NPC索引)
 * チェックならアクションのシーケンスをチェックする  (若是则检查动作序列)
 * 望んだアクションをしていたらプレイヤーのカウントをアップさせる  (若做出期望动作则提升玩家计数)
 * カウントアップなら指定の場所へワープさせる  (计数满则传送到指定地点) */

enum {
	BODYLAN_E_COMMANDNUM = CHAR_NPCWORKINT1, // コマンドの数  (命令数)
};

// ウインドウモード  (窗口模式)
enum {
	BODYLAN_WIN_FIRST,
	BODYLAN_WIN_LAST_GOOD,
	BODYLAN_WIN_LAST_NG,
	BODYLAN_WIN_GOOD_NO,
	BODYLAN_WIN_ALREADY,
	BODYLAN_WIN_NOT_PREEVENT,
	BODYLAN_WIN_END
};

static void NPC_BodyLan_Profit(int meindex, int playerindex);

static void NPC_BodyLan_Window(
	int meindex,
	int talkerindex,
	int mode);

/*********************************
 * 初期化  (初始化)
 *********************************/
BOOL NPC_BodyLanInit(int meindex) {

	char szP[256], szArg[4096];
	char buf[256];
	int i, needSeq;

	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPEEVENT);

	if (NPC_Util_GetArgStr(meindex, szArg, sizeof(szArg)) == NULL) {
		print("npc_bodylan.c:没有引数(%s)\n",
			  CHAR_getChar(meindex, CHAR_NPCARGUMENT));
		return FALSE;
	}

	// シーケンスを取得  (获取序列)
	if (NPC_Util_GetStrFromStrWithDelim(szArg, "Act", szP, sizeof(szP)) == NULL) {
		print("npc_bodylan:动作文字列尚未设定(%s)\n", szArg);
		return FALSE;
	}

	for (i = 0;; i++) {
		//   必要な分  (所需部分)
		if (getStringFromIndexWithDelim(szP, ",", i, buf, sizeof(buf)) != FALSE) {
			needSeq = atoi(buf);
			// マイナスだったらここまで  (若是负数则到此为止)
			if (needSeq < 0) {
				if (i <= 0) {
					print("npc_bodylan:动作列尚未设定(%s)\n", szArg);
				}
				//     数をセット  (设置数量)
				CHAR_setWorkInt(meindex, BODYLAN_E_COMMANDNUM, i);
				break;
			} else {
				// ループして数える  (循环计数)
			}
		} else {
			if (i <= 0) {
				print("npc_bodylan:动作列尚未设定(%s)\n", szArg);
			}
			//     数をセット  (设置数量)
			CHAR_setWorkInt(meindex, BODYLAN_E_COMMANDNUM, i);
			break;
		}
	}

	return TRUE;
}

/*********************************
 *   しかけられた時の値  (被设置时的值)
 *********************************/
void NPC_BodyLanTalked(int meindex, int talkerindex, char *szMes, int color) {
	char szP[256], szArg[4096];
	int EventNo = -1, Pre_Event = -1;

	/* プレイヤーに対してだけ処理する  (只处理玩家) */
	if (CHAR_getInt(talkerindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) {
		return;
	}

	/* グリッド移動のみ  (仅网格移动) */
	if (NPC_Util_CharDistance(talkerindex, meindex) > 2) {
		return;
	}

	// 引数  (参数)
	if (NPC_Util_GetArgStr(meindex, szArg, sizeof(szArg)) == NULL) {
		print("npc_bodylan.c:没有引数(%s)\n",
			  CHAR_getChar(meindex, CHAR_NPCARGUMENT));
		return;
	} else {
		// イベント番号取得  (获取事件编号)
		if (NPC_Util_GetStrFromStrWithDelim(szArg, "EventNo", szP, sizeof(szP)) != NULL) {
			EventNo = atoi(szP);
		}
		// 以前に必要イベント番号取得  (获取此前必需的事件编号)
		if (NPC_Util_GetStrFromStrWithDelim(szArg, "Pre_Event", szP, sizeof(szP)) != NULL) {
			Pre_Event = atoi(szP);
		}
	}

	// 以前に必要イベント番号があれば  (若有此前必需的事件编号)
	if (Pre_Event >= 0) {
		// イベントに対してどうなっているかチェック  (检查事件进行状态)
		if (NPC_EventCheckFlg(talkerindex, Pre_Event) == FALSE) {
			//   必要イベントをクリアしていなかったらこのセリフ  (未清除必需事件则显示此台词)
			NPC_BodyLan_Window(meindex, talkerindex, BODYLAN_WIN_NOT_PREEVENT);
			return;
		}
	}
	// イベント番号があれば  (若有事件编号)
	if (EventNo >= 0) {
		// イベントに対してどうなっているかチェック  (检查事件进行状态)
		if (NPC_EventCheckFlg(talkerindex, EventNo) == TRUE) {
			//   していたらこのセリフ  (若已发生则此台词)
			NPC_BodyLan_Window(meindex, talkerindex, BODYLAN_WIN_ALREADY);
			return;
		}
	}

	// 前にこのプレイヤーは自分と取引していたか  (此前该玩家是否与自己交易过)
	if (CHAR_getWorkInt(talkerindex, CHAR_WORKTRADER) == meindex) {

		// さらにコマンドが超えていたら  (若已超过命令数)
		if (CHAR_getWorkInt(talkerindex, CHAR_WORKSHOPRELEVANT) >= CHAR_getWorkInt(meindex, BODYLAN_E_COMMANDNUM)) {
			// 超えていたら  (若已超过)＂
			NPC_BodyLan_Window(meindex, talkerindex, BODYLAN_WIN_LAST_GOOD);
			return;
		} else {
			// 撩
			NPC_BodyLan_Window(meindex, talkerindex, BODYLAN_WIN_LAST_NG);
			return;
		}
	} else {
		// 初めてなので作る  (因为是初次所以创建)
		// プレイヤーに自分のインデックスを保持させる  (让玩家持有自己的索引)
		CHAR_setWorkInt(talkerindex, CHAR_WORKTRADER, meindex);
		// シーケンスは最初からに初期化  (序列从头初始化)
		CHAR_setWorkInt(talkerindex, CHAR_WORKSHOPRELEVANT, 1);

		// その際セリフとか考えるべきかなあ  (此时是否该考虑台词之类)
		NPC_BodyLan_Window(meindex, talkerindex, BODYLAN_WIN_FIRST);
		return;
	}
}

/*=======================================
 * watch 中  (监视中)
 *======================================*/
void NPC_BodyLanWatch(
	int objmeindex,
	int objmoveindex,
	CHAR_ACTION act,
	int x,
	int y,
	int dir,
	int *opt,
	int optlen) {
	char szP[256], szArg[4096];
	char buf[256];
	int actindex;
	int meindex;
	int seqNo, needSeq;

	// キャラクタタイプはリターン  (角色类型则返回)
	if (OBJECT_getType(objmoveindex) != OBJTYPE_CHARA)
		return;
	actindex = OBJECT_getIndex(objmoveindex);
	// プレイヤータイプはリターン  (玩家类型则返回)
	if (CHAR_getInt(actindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER)
		return;

	// 自分のインデックス  (自己的索引)
	meindex = OBJECT_getIndex(objmeindex);

	// 自分のINDEXを保持していなければリターン  (若未持有自己的INDEX则返回)
	if (CHAR_getWorkInt(actindex, CHAR_WORKTRADER) != meindex) {
		return;
	}

	// どこまでアクションをしているか  (进行到哪个动作了)
	seqNo = CHAR_getWorkInt(actindex, CHAR_WORKSHOPRELEVANT);
	//   なんであれ最初から  (无论如何从头开始)
	if (seqNo < 1)
		seqNo = 1;

	// 引数  (参数)
	if (NPC_Util_GetArgStr(meindex, szArg, sizeof(szArg)) == NULL) {
		print("npc_bodylan.c:没有引数(%s)\n",
			  CHAR_getChar(meindex, CHAR_NPCARGUMENT));
		return;
	}

	// シーケンスを取得  (获取序列)
	if (NPC_Util_GetStrFromStrWithDelim(szArg, "Act", szP, sizeof(szP)) == NULL) {
		print("npc_bodylan:动作文字列尚未设定(%s)\n", szArg);
		return;
	}

	//   必要な分  (所需部分)
	if (getStringFromIndexWithDelim(szP, ",", seqNo, buf, sizeof(buf)) != FALSE) {
		needSeq = atoi(buf);
	} else {
		// なぜかなかったら最初から  (若没有则从头开始)
		CHAR_setWorkInt(actindex, CHAR_WORKSHOPRELEVANT, 1);

		//		print( "做过头了。回到最初。\n", seqNo, needSeq );
		// 何か考えるべきか  (该想点什么吧)’’’
		return;
	}

	// 今必要なアクションが一致した  (当前必需动作一致时)
	if (needSeq == act) {
		// 一致したこれがか  (一致了就是这个)
		//		print( "成功\(%d次数是%d)\n", seqNo, needSeq );
		seqNo++;
		if (seqNo >= CHAR_getWorkInt(meindex, BODYLAN_E_COMMANDNUM)) {
			//			print( "在此结束。\n" );
		}
		// この位置を保持  (保持此位置)
		CHAR_setWorkInt(actindex, CHAR_WORKSHOPRELEVANT, seqNo);
	} else {
		// 失したんで最初からやり直し  (失败了所以从头重做)
		CHAR_setWorkInt(actindex, CHAR_WORKSHOPRELEVANT, 1);
		//		print( "(%d次数是%d)\n", seqNo, needSeq );
	}
}

//********* ご褒美の  (奖励的)     *********
static void NPC_BodyLan_Profit(int meindex, int playerindex) {
	char szArg[4096], szP[256];
	int fl, x, y, pmode, i, subindex, parent;

	// 引数  (参数)
	if (NPC_Util_GetArgStr(meindex, szArg, sizeof(szArg)) == NULL) {
		print("npc_bodylan.c:没有引数(%s)\n",
			  CHAR_getChar(meindex, CHAR_NPCARGUMENT));
		return;
	}

	//*********************************************
	//
	//   そのワープ修正  (传送修正)
	//
	//*********************************************
	if (NPC_Util_GetStrFromStrWithDelim(szArg, "Warp", szP, sizeof(szP)) != NULL) {
		// ワープ座標取得  (获取传送坐标)
		if (sscanf(szP, "%d,%d,%d", &fl, &x, &y) == 3) {
		} else {
			print("npc_bodylan: 无法读取空间座标(%s)\n", szP);
			return;
		}
		// パーティで連れているメンバーもワープ  (队伍中随行成员也传送)
		pmode = CHAR_getWorkInt(playerindex, CHAR_WORKPARTYMODE);
		switch (pmode) {
		case 1: // 自分が親  (自己是队长)
			parent = playerindex;
			break;
		case 2: // 自分がメンバー親を取得  (自己作为成员取队长)
			parent = CHAR_getWorkInt(playerindex, CHAR_WORKPARTYINDEX1);
			break;
		default:
			// パーティじゃなかったら自分だけワープ  (若非队伍则仅自己传送)
			CHAR_warpToSpecificPoint(playerindex, fl, x, y);
			return;
		}
		// 全員ワープ  (全体传送)
		for (i = 0; i < CHAR_PARTYMAX; i++) {
			subindex = CHAR_getWorkInt(parent, CHAR_WORKPARTYINDEX1 + i);
			if (CHAR_CHECKINDEX(subindex) == FALSE)
				continue;
			// パーティ全員ワープ  (队伍全体传送)
			CHAR_warpToSpecificPoint(subindex, fl, x, y);
		}
	}
}

#if 1
static void NPC_BodyLan_Window(
	int meindex,
	int talkerindex,
	int mode) {
	char token[1024];
	char escapedname[2048];
	char szArg[4096];
	char szP[256];
	int fd;
	int buttontype = 0, windowtype = 0, windowno = 0;

	if (CHAR_CHECKINDEX(talkerindex) == FALSE)
		return;
	fd = getfdFromCharaIndex(talkerindex);

	// 引数  (参数)
	if (NPC_Util_GetArgStr(meindex, szArg, sizeof(szArg)) == NULL) {
		print("npc_bodylan.c:没有引数(%s)\n",
			  CHAR_getChar(meindex, CHAR_NPCARGUMENT));
		return;
	}

	szP[0] = 0;

	switch (mode) {
	case BODYLAN_WIN_FIRST:
		//   指定されたシーンのセリフ  (指定场景的台词)
		if (NPC_Util_GetStrFromStrWithDelim(szArg, "First", szP, sizeof(szP)) == NULL) {
			print("npc_bodylan:一开始讲话的文字没有输入(%s)\n", szArg);
			return;
		}
		sprintf(token, "%s", szP);
		buttontype = WINDOW_BUTTONTYPE_YES | WINDOW_BUTTONTYPE_NO;
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = mode;
		break;

	case BODYLAN_WIN_LAST_GOOD:
		//     で正解だったら  (答对时)
		if (NPC_Util_GetStrFromStrWithDelim(szArg, "Good", szP, sizeof(szP)) == NULL) {
			print("npc_bodylan:答对时说的文字没有输入(%s)\n", szArg);
			return;
		}
		sprintf(token, "%s", szP);
		buttontype = WINDOW_BUTTONTYPE_YESNO; // YES|NO
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = mode;
		break;

	case BODYLAN_WIN_LAST_NG:
		//     で不正解だったら  (答错时)
		if (NPC_Util_GetStrFromStrWithDelim(szArg, "Ng", szP, sizeof(szP)) == NULL) {
			print("npc_bodylan:答错时说的文字没有输入(%s)\n", szArg);
			return;
		}
		sprintf(token, "%s", szP);
		buttontype = WINDOW_BUTTONTYPE_OK; // OK
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = mode;
		break;

	case BODYLAN_WIN_GOOD_NO:
		// 正解時にキャンセル  (答对时取消)
		if (NPC_Util_GetStrFromStrWithDelim(szArg, "Good_No", szP, sizeof(szP)) == NULL) {
			print("npc_bodylan:答对时取消奖品的文字没有输入(%s)\n", szArg);
			return;
		}
		sprintf(token, "%s", szP);
		buttontype = WINDOW_BUTTONTYPE_OK; // OK
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = mode;
		break;

	case BODYLAN_WIN_ALREADY:
		// すでにイベントを終えていたシーン  (已结束事件的场景)
		if (NPC_Util_GetStrFromStrWithDelim(szArg, "Good_No", szP, sizeof(szP)) == NULL) {
			print("npc_bodylan:事件结束时的文字没有输入(%s)\n", szArg);
			return;
		}
		sprintf(token, "%s", szP);
		buttontype = WINDOW_BUTTONTYPE_OK; // OK
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = mode;
		break;

	case BODYLAN_WIN_NOT_PREEVENT:
		// 以前に必要なイベントをこなしていないシーン  (未完成所需事件的场景)
		if (NPC_Util_GetStrFromStrWithDelim(szArg, "Pre_Not", szP, sizeof(szP)) == NULL) {
			print("npc_bodylan:?事前事件结束的文字没有输入(%s)\n", szArg);
			return;
		}
		sprintf(token, "%s", szP);
		buttontype = WINDOW_BUTTONTYPE_OK; // OK
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = mode;
		break;
	default:
		return;
	}

	makeEscapeString(token, escapedname, sizeof(escapedname));
	/*-ここで送信する  (在此发送)--*/
	lssproto_WN_send(fd, windowtype,
					 buttontype,
					 windowno,
					 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
					 escapedname);
}

/*-----------------------------------------
クライアントから返ってきた時に呼び出される  (客户端返回时被调用)
-------------------------------------------*/
void NPC_BodyLanWindowTalked(
	int meindex,
	int talkerindex,
	int seqno,
	int select,
	char *data) {

	if (NPC_Util_CharDistance(talkerindex, meindex) > 2)
		return;

	switch (seqno) {
	case BODYLAN_WIN_LAST_GOOD: // 正解時にOK押されたら  (答对时按下OK)
		if (select == WINDOW_BUTTONTYPE_YES) {
			NPC_BodyLan_Profit(meindex, talkerindex);
			// プレイヤーに自分のインデックスを忘れさせる  (让玩家忘记自己的索引)
			CHAR_setWorkInt(talkerindex, CHAR_WORKTRADER, -1);
			// シーケンスは最初からに初期化  (序列从头初始化)
			CHAR_setWorkInt(talkerindex, CHAR_WORKSHOPRELEVANT, 1);
		} else if (select == WINDOW_BUTTONTYPE_NO) {
			// 正解時にキャンセルされたら  (答对时被取消)
			NPC_BodyLan_Window(meindex, talkerindex, BODYLAN_WIN_GOOD_NO);
			// プレイヤーに自分のインデックスを忘れさせる  (让玩家忘记自己的索引)
			CHAR_setWorkInt(talkerindex, CHAR_WORKTRADER, -1);
			// シーケンスは最初からに初期化  (序列从头初始化)
			CHAR_setWorkInt(talkerindex, CHAR_WORKSHOPRELEVANT, 1);
		}
		break;
	default:
		break;
	}
}

#endif
