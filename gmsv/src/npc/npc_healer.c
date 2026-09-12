#include "version.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "object.h"
#include "char_base.h"
#include "char.h"
#include "util.h"
#include "npcutil.h"
#include "item.h"
#include "readmap.h"

#define RANGE 2
void NPC_HealerSpeak(int index, int talker);
void NPC_HealerAllHeal(int talker);
BOOL NPC_MoneyCheck(int meindex, int talker);
int NPC_CostCheck(int talker);
void NPC_CharCheckPoint(int meindex, int talker);
int NPC_WorkInput(int meindex, int talker);

/**********************************
初期化  (初始化)
************************************/
BOOL NPC_HealerInit(int meindex) {
	// タイプをヒールに設定  (将类型设置为治疗)
	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPEHEALER);
	// 殴撃できないようにする  (使其不会被攻击)
	CHAR_setFlg(meindex, CHAR_ISATTACKED, 0);
	// オーバーされないように  (使其不被覆盖)
	//    CHAR_setFlg( meindex , CHAR_ISOVERED , 0 );

	return TRUE;
}

/*-------------------------------------------
 *
 *   全員のHP,MPを回復する  (恢复全员的HP、MP)
 *
 --------------------------------------------*/
void NPC_HealerTalked(int meindex, int talker, char *msg, int color) {

	char *npcarg;
	char token[32];
	int msgNo;

	npcarg = CHAR_getChar(meindex, CHAR_NPCARGUMENT);

	getStringFromIndexWithDelim(npcarg, "|", 1, token, sizeof(token));
	msgNo = atoi(token);

	/*---プレイヤーに対してだけ実行する---  (仅对玩家执行) */
	if (CHAR_getInt(talker, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER)
		return;

	/*---移動しているか？または死んでいるか？---  (在移动吗？还是已死亡？) */
	/* グリッド移動のみ   (仅网格移动) */
	if (NPC_Util_CharDistance(talker, meindex) > 2)
		return;

	if ((CHAR_getWorkInt(talker, CHAR_WORKPARTYMODE) == 0) || (CHAR_getWorkInt(talker, CHAR_WORKPARTYMODE) == 2)) {
		/*--全回復させましょう--  (来全部恢复吧) */
		NPC_HealerAllHeal(talker);
		if (msgNo == 1) {
			CHAR_talkToCli(talker, meindex,
						   "\xd2\xd1\xbe\xad\xc8\xab\xb2\xbf\xbb\xd8\xb8\xb4\xa1\xa3\xc7\xeb\xd4\xda\xcf\xc2\xb4\xce\xb5\xc4\xb1\xc8\xc8\xfc\xd6\xd0\xbc\xd3\xd3\xcd\xe0\xa1\xa3\xa1", CHAR_COLORWHITE);

		} else if (msgNo == 2) {
			CHAR_talkToCli(talker, meindex,
						   "\xd3\xc9\xec\xb6\xc4\xe3\xba\xdc\xb3\xcf\xca\xb5\xa3\xac\xc8\xc3\xce\xd2\xb0\xef\xc4\xe3\xbb\xd8\xb8\xb4\xb0\xc9\xa3\xa1", CHAR_COLORWHITE);
		}

	} else {
		int i = 0;
		int otherindex;

		for (i = 0; i < CHAR_PARTYMAX; i++) {
			otherindex = CHAR_getWorkInt(talker, CHAR_WORKPARTYINDEX1 + i);
			if (otherindex != -1) {

				NPC_HealerAllHeal(otherindex);
				if (msgNo == 1) {
					CHAR_talkToCli(otherindex, meindex,
								   "\xd2\xd1\xbe\xad\xc8\xab\xb2\xbf\xbb\xd8\xb8\xb4\xa1\xa3\xc7\xeb\xd4\xda\xcf\xc2\xb4\xce\xb5\xc4\xb1\xc8\xc8\xfc\xd6\xd0\xbc\xd3\xd3\xcd\xe0\xa1\xa3\xa1", CHAR_COLORWHITE);

				} else if (msgNo == 2) {
					CHAR_talkToCli(otherindex, meindex,
								   "\xd3\xc9\xec\xb6\xc4\xe3\xba\xdc\xb3\xcf\xca\xb5\xa3\xac\xc8\xc3\xce\xd2\xb0\xef\xc4\xe3\xbb\xd8\xb8\xb4\xb0\xc9\xa3\xa1", CHAR_COLORWHITE);
				}
			}
		}
	}
}

/*----------------------*/
/* 全回復   (全部恢复) */
/*-----------------------*/
void NPC_HealerAllHeal(int talker) {
	int i;
	int petindex;
	char petsend[3];
	char msgbuf[5];

	CHAR_setInt(talker, CHAR_HP, CHAR_getWorkInt(talker, CHAR_WORKMAXHP));
	CHAR_setInt(talker, CHAR_MP, CHAR_getWorkInt(talker, CHAR_WORKMAXMP));

	for (i = 0; i < CHAR_MAXPETHAVE; i++) {
		petindex = CHAR_getCharPet(talker, i);

		if (petindex == -1)
			continue;

		/* キャラのチェック    (检查角色) */
		if (!CHAR_CHECKINDEX(talker))
			continue;

		/* ペットのindexチェックをする  (检查宠物index) */
		if (CHAR_CHECKINDEX(petindex) == FALSE)
			continue;

		/*--全回復--  (全部恢复) */
		CHAR_setFlg(petindex, CHAR_ISDIE, 0);
		CHAR_setInt(petindex, CHAR_HP, CHAR_getWorkInt(petindex, CHAR_WORKMAXHP));
		CHAR_setInt(petindex, CHAR_MP, CHAR_getWorkInt(petindex, CHAR_WORKMAXMP));

		/*--パラメータ調節--  (参数调节) */
		CHAR_complianceParameter(petindex);
		sprintf(petsend, "K%d", i);
		CHAR_sendStatusString(talker, petsend);
	}

	/*---仲間がいれば仲間にも送信---  (若有同伴则也向同伴发送) */
	if (CHAR_getWorkInt(talker, CHAR_WORKPARTYMODE) != CHAR_PARTY_NONE) {
		int topartyarray = -1;
		int oyaindex = CHAR_getWorkInt(talker, CHAR_WORKPARTYINDEX1);

		if (CHAR_CHECKINDEX(oyaindex)) {
			int i;

			/* 回復分と回復したやつの仲間の座標を取る   (获取已恢复者的同伴坐标) */
			for (i = 0; i < CHAR_PARTYMAX; i++) {
				int workindex = CHAR_getWorkInt(oyaindex, CHAR_WORKPARTYINDEX1 + i);
				if (CHAR_CHECKINDEX(workindex)) {
					if (workindex == talker) {
						topartyarray = i;
						break;
					}
				}
			}

			for (i = 0; i < CHAR_PARTYMAX; i++) {
				int otherindex = CHAR_getPartyIndex(talker, i);
				/* 仲間パラメータを送る  (发送同伴参数) */
				if (CHAR_CHECKINDEX(otherindex)) {
					snprintf(msgbuf, sizeof(msgbuf), "N%d", topartyarray);
					if (otherindex != talker) {
						CHAR_sendStatusString(otherindex, msgbuf);
					}
				}
			}
		}
	}

	/*--パラメータ送り--  (参数发送) */
	CHAR_send_P_StatusString(talker, CHAR_P_STRING_HP);
	CHAR_send_P_StatusString(talker, CHAR_P_STRING_MP);
}
