#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"

// 現在の進捗フラグの数  (当前进度旗标数)
#define MAXEVENTFLG 96

/*
 * イベントのフラグをチェックする  (检查事件旗标)
 *
 */

static void NPC_CheckMan_selectWindow(int meindex, int toindex, int num);
int NPC_NowFlgCheck(int meindex, int talker, int now[MAXEVENTFLG]);
int NPC_EndFlgCheck(int meindex, int talker, int nowflg[MAXEVENTFLG]);
BOOL NPC_FlgCheckMain(int meindex, int talker, int nowindex, int now[MAXEVENTFLG], char *work2);

/*********************************
 * 初期質  (初始性质)
 *********************************/
BOOL NPC_CheckManInit(int meindex) {
	/*--キャラクターのタイプを修正  (修正角色类型)--*/
	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPECHECKMAN);

	return TRUE;
}

/*********************************
 *   しかけられた時の値  (被设置时的值)
 *********************************/
void NPC_CheckManTalked(int meindex, int talkerindex, char *szMes, int color) {

	/* プレイヤーかどうか調べる  (检查是否玩家) */
	if (CHAR_getInt(talkerindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) {
		return;
	}

	/*--  の前にいるかどうか  (是否在前面)--*/
	if (NPC_Util_isFaceToFace(meindex, talkerindex, 2) == FALSE) {
		/* グリッド移動のみ  (仅网格移动) */
		if (NPC_Util_isFaceToChara(talkerindex, meindex, 1) == FALSE)
			return;
	}

	//  ウィンドウに  (到窗口)
	NPC_CheckMan_selectWindow(meindex, talkerindex, 0);
}

/*
 * 選択ウィンドウ  (选择窗口)
 */
static void NPC_CheckMan_selectWindow(int meindex, int talker, int num) {

	char token[1024];
	char work[256];
	char work2[512];

	char escapedname[1024];
	int fd = getfdFromCharaIndex(talker);
	int buttontype = 0;
	int windowtype = 0;
	int windowno = 0;
	int now[MAXEVENTFLG];
	int nowindex;
	int i;
	int page;

	work[0] = 0;
	work2[0] = 0;
	token[0] = 0;

	now[0] = 0;

	/*--ウィンドウタイプメッセージを送信時に修正  (发送窗口类型消息时修正)--*/
	windowtype = WINDOW_MESSAGETYPE_MESSAGE;

	switch (num) {

	case 0:
		CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANT, 0);
		CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC, 0);

		/*--メニュー  (菜单)--*/
		sprintf(token, "3\n ｛｛｛｛＞＞｛检查菜单｛＞＞ "
					   "\n｛｛｛进行当前旗标检查"
					   "\n"
					   "\n｛｛｛｛ Ｕ｛NOW旗标检查｛Ｖ "
					   "\n｛｛｛｛ Ｕ｛END旗标检查｛Ｖ "
					   "\n\n｛｛ Ｕ｛NOW旗标检查  选择  Ｖ "
					   "\n｛｛ Ｕ｛END旗标检查  选择  Ｖ ");

		buttontype = WINDOW_BUTTONTYPE_CANCEL;
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		windowno = CHAR_WINDOWTYPE_CHECKMAN_START;
		break;

		// NOWフラグの個数  (NOW旗标数量)
	case 1:

		// NOWフラグのチェック  (检查NOW旗标)
		nowindex = NPC_NowFlgCheck(meindex, talker, now);

		// ページを  (页)
		page = CHAR_getWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC);

		if (page == 0 || page == 1) {
			i = 0;
		} else {
			i = 83;
		}

		// ページ  (页)
		for (; i < nowindex; i++) {
			sprintf(work, "%d,", now[i]);
			strcat(work2, work);
		}

		sprintf(token, "｛｛｛｛｛＞＞｛检查菜单｛＞＞ "
					   "\n｛｛当前设置中的NOW事件"
					   "\n%s",
				work2);

		if (page != 2 && nowindex > 83) {
			// ボタン  (按钮)
			CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANT, 1);
			// ページ  (页)
			CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC, 2);

			buttontype = WINDOW_BUTTONTYPE_NEXT;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			windowno = CHAR_WINDOWTYPE_CHECKMAN_MAIN;

		} else {
			buttontype = WINDOW_BUTTONTYPE_OK;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		}

		break;

		// ENDフラグの個数  (END旗标数量)
	case 2:
		// 済みフラグのチェック  (检查已完成旗标)
		nowindex = NPC_EndFlgCheck(meindex, talker, now);

		// ページを  (页)
		page = CHAR_getWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC);

		if (page == 0 || page == 1) {
			i = 0;
		} else {
			i = 83;
		}
		work2[0] = 0;

		// ページ  (页)
		for (; i < nowindex; i++) {
			sprintf(work, "%d,", now[i]);
			strcat(work2, work);
		}
		sprintf(token, "｛｛｛｛｛＞＞｛检查菜单｛＞＞ "
					   "\n｛｛当前设置中的END事件"
					   "\n%s",
				work2);

		if (page != 2 && nowindex > 83) {
			// ボタン  (按钮)
			CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANT, 2);
			// ページ  (页)
			CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC, 2);

			buttontype = WINDOW_BUTTONTYPE_NEXT;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			windowno = CHAR_WINDOWTYPE_CHECKMAN_MAIN;

		} else {
			buttontype = WINDOW_BUTTONTYPE_OK;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		}

		break;

		// NOWフラグの選択  (选择NOW旗标)
	case 4: {

		// NOWフラグチェック  (检查NOW旗标)
		nowindex = NPC_NowFlgCheck(meindex, talker, now);

		if (NPC_FlgCheckMain(meindex, talker, nowindex, now, work2) == FALSE) {
			return;
		}

		sprintf(token, "｛｛｛｛｛＞＞｛检查菜单｛＞＞ "
					   "\n｛｛当前设置中的NOW事件"
					   "\n%s",
				work2);

		page = CHAR_getWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC);
		nowindex = (nowindex / ((6 * page) + 1));

		if (page != 16 && nowindex != 0) {
			// ボタン  (按钮)
			CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANT, 4);
			// ページ  (页)
			page = CHAR_getWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC);
			page++;
			CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC, page);

			buttontype = WINDOW_BUTTONTYPE_NEXT;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			windowno = CHAR_WINDOWTYPE_CHECKMAN_MAIN;
		} else {
			buttontype = WINDOW_BUTTONTYPE_OK;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		}

	} break;

		// ENDフラグの選択  (选择END旗标)
	case 5: {
		// ENDフラグのチェック  (检查END旗标)
		nowindex = NPC_EndFlgCheck(meindex, talker, now);

		if (NPC_FlgCheckMain(meindex, talker, nowindex, now, work2) == FALSE) {
			return;
		}

		sprintf(token, "｛｛｛｛｛＞＞｛检查菜单｛＞＞ "
					   "\n｛｛当前设置中的END事件"
					   "\n%s",
				work2);

		page = CHAR_getWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC);

		nowindex = (nowindex / ((6 * page) + 1));

		if (page != 16 && nowindex != 0) {
			// ボタン  (按钮)
			CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANT, 5);
			// ページ  (页)
			page = CHAR_getWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC);
			page++;
			CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC, page);

			buttontype = WINDOW_BUTTONTYPE_NEXT;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			windowno = CHAR_WINDOWTYPE_CHECKMAN_MAIN;
		} else {
			buttontype = WINDOW_BUTTONTYPE_OK;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		}
	}
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
クライアントから返ってきた時に呼び出される  (客户端返回时被调用)［
-------------------------------------------*/
void NPC_CheckManWindowTalked(int meindex, int talkerindex,
							  int seqno, int select, char *data) {
	int datano;

	if (NPC_Util_CharDistance(talkerindex, meindex) > 2)
		return;

	datano = atoi(data);

	if (select == WINDOW_BUTTONTYPE_OK) {
		NPC_CheckMan_selectWindow(meindex, talkerindex, 0);
	} else if (select == WINDOW_BUTTONTYPE_CANCEL) {
		return;
	}

	switch (CHAR_getWorkInt(talkerindex, CHAR_WORKSHOPRELEVANT)) {
	case 1:
		if (CHAR_getWorkInt(talkerindex, CHAR_WORKSHOPRELEVANTSEC) == 2) {
			NPC_CheckMan_selectWindow(meindex, talkerindex, 1);
		}
		break;

	case 2:
		if (CHAR_getWorkInt(talkerindex, CHAR_WORKSHOPRELEVANTSEC) == 2) {
			NPC_CheckMan_selectWindow(meindex, talkerindex, 2);
		}
		break;

	case 4:
		if (CHAR_getWorkInt(talkerindex, CHAR_WORKSHOPRELEVANTSEC) >= 2) {
			NPC_CheckMan_selectWindow(meindex, talkerindex, 4);
		}

	case 5:
		if (CHAR_getWorkInt(talkerindex, CHAR_WORKSHOPRELEVANTSEC) >= 2) {
			NPC_CheckMan_selectWindow(meindex, talkerindex, 5);
		}

		break;
	}

	switch (datano) {
	case 1:
		NPC_CheckMan_selectWindow(meindex, talkerindex, 1);
		break;

	case 2:
		NPC_CheckMan_selectWindow(meindex, talkerindex, 2);
		break;

	case 4:
		NPC_CheckMan_selectWindow(meindex, talkerindex, 4);
		break;

	case 5:
		NPC_CheckMan_selectWindow(meindex, talkerindex, 5);
		break;
	}
}

/*
 *｛イベントのフラグをチェックする  (检查事件旗标)
 */
int NPC_NowFlgCheck(int meindex, int talker, int nowflg[MAXEVENTFLG]) {
	int i = 0;
	int j = 0;

	for (i = 0; i < MAXEVENTFLG; i++) {
		if (NPC_NowEventCheckFlg(talker, i) == TRUE) {
			nowflg[j] = i;
			j++;
		}
	}

	return j;
}

/*
 *｛イベント済みフラグをチェックする  (检查事件已完成旗标)
 */
int NPC_EndFlgCheck(int meindex, int talker, int nowflg[MAXEVENTFLG]) {
	int i = 0;
	int j = 0;

	for (i = 0; i < MAXEVENTFLG; i++) {

		if (NPC_EventCheckFlg(talker, i) == TRUE) {
			nowflg[j] = i;
			j++;
		}
	}

	return j;
}

/*
 * 選択  (选择)
 *
 */
BOOL NPC_FlgCheckMain(int meindex, int talker, int nowindex, int now[MAXEVENTFLG], char *work2) {
	int page;
	int max;
	int i;
	int shou;
	int j = 1;
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char work[512];
	char buf[40];
	char buf2[42];

	// ページを  (页)
	page = CHAR_getWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC);

	if (page == 0) {
		page = 1;
		CHAR_setWorkInt(talker, CHAR_WORKSHOPRELEVANTSEC, 1);
	}

	if (page == 1) {
		i = 0;
		if (nowindex > 6) {
			max = 7;
		} else {
			max = nowindex;
		}
	} else {
		max = (page * 6) + 1;
		i = max - 7;
		shou = nowindex / max;

		if (shou == 0) {
			max = nowindex;
		} else {
			i = max - 7;
		}
	}

	// イベントの選択からデータを取得する  (从事件选择获取数据)
	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("NPC_CheckMan:GetArgStrErr");
		return FALSE;
	}

	work[0] = 0;
	work2[0] = 0;

	// ページ  (页)
	for (; i < max; i++) {
		sprintf(work, "#%d:", now[i]);
		j = 1;
		while (getStringFromIndexWithDelim(argstr, "|", j, buf, sizeof(buf)) != FALSE) {
			j++;
			if (strstr(buf, work) != NULL) {
				sprintf(buf2, "%s\n", buf);
				strcat(work2, buf2);
				break;
			}
		}
	}

	return TRUE;
}
