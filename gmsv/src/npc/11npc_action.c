#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "npc_action.h"

/*
 * プレイヤーのアクションに対して実行するNPC  (对玩家的动作执行的NPC)
 * 殴に返したりするだけだが  (仅对攻击作出回应)
 * 歩くまたは動いているのアクションには実行しない  (对行走或移动中的动作不执行)
 *
 * 引数  (参数)
 *      msgcol:		メッセージの色（デフォルトは黄色）  (消息颜色（默认黄色）)
 *      normal:		  殴に殴ってきた値や無効なアクションに対しての返答  (对攻击值和无效动作的回应)
 *		attack:		  殴撃アクションに対する返答  (对攻击动作的回应)
 *		damage:		ダメージを受けたアクションに対する返答  (对受到伤害动作的回应)
 *		down:		倒れるアクションに対する返答  (对倒地动作的回应)
 *		sit:		座るアクションに対しての返答  (对坐下动作的回应)
 *		hand:		手を振るアクションに対しての返答  (对手势动作的回应)
 *		pleasure:	喜ぶアクションに対しての返答  (对高兴动作的回应)
 *		angry:		怒るアクションに対しての返答  (对生气动作的回应)
 *		sad:		  悲しむアクションに対しての返答  (对悲伤动作的回应)
 *		guard:		ガードするアクションに対しての返答  (对防御动作的回应)
 */

#define NPC_ACTION_MSGCOLOR_DEFAULT CHAR_COLORYELLOW

enum {
	CHAR_WORK_MSGCOLOR = CHAR_NPCWORKINT1,
};

/*********************************
 * 初期化  (初始化)
 *********************************/
BOOL NPC_ActionInit(int meindex) {
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	int tmp;

	tmp = NPC_Util_GetNumFromStrWithDelim(argstr, "msgcol");
	if (tmp == -1)
		tmp = NPC_ACTION_MSGCOLOR_DEFAULT;
	CHAR_setWorkInt(meindex, CHAR_WORK_MSGCOLOR, tmp);

	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPEACTION);

	return TRUE;
}

/*********************************
 *   しかけられた時の値  (被设置时的值)
 *********************************/
void NPC_ActionTalked(int meindex, int talkerindex, char *szMes,
					  int color) {
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[64];

	/* プレイヤーに対してだけ実行する  (仅对玩家执行) */
	if (CHAR_getInt(talkerindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) {
		return;
	}
	/* グリッド移動のみ  (仅网格移动) */
	if (!NPC_Util_charIsInFrontOfChar(talkerindex, meindex, 1))
		return;

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	if (NPC_Util_GetStrFromStrWithDelim(argstr,
										"normal",
										buf, sizeof(buf)) != NULL) {
		CHAR_talkToCli(talkerindex, meindex, buf,
					   CHAR_getWorkInt(meindex, CHAR_WORK_MSGCOLOR));
	}
}
/*********************************
 * watch値  (值)
 *********************************/
void NPC_ActionWatch(int meobjindex, int objindex, CHAR_ACTION act,
					 int x, int y, int dir, int *opt, int optlen) {
	int meindex;
	int index;
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[64];
	int i;
	struct {
		CHAR_ACTION act;
		char *string;
	} searchtbl[] = {
		{CHAR_ACTATTACK, "attack"},
		{CHAR_ACTDAMAGE, "damage"},
		{CHAR_ACTDOWN, "down"},
		{CHAR_ACTSIT, "sit"},
		{CHAR_ACTHAND, "hand"},
		{CHAR_ACTPLEASURE, "pleasure"},
		{CHAR_ACTANGRY, "angry"},
		{CHAR_ACTSAD, "sad"},
		{CHAR_ACTGUARD, "guard"},
		{CHAR_ACTNOD, "nod"},
		{CHAR_ACTTHROW, "throw"},
	};

	if (OBJECT_getType(objindex) != OBJTYPE_CHARA)
		return;
	index = OBJECT_getIndex(objindex);
	/* プレイヤーにのみ実行する  (仅对玩家执行) */
	if (CHAR_getInt(index, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER)
		return;

	meindex = OBJECT_getIndex(meobjindex);

	/* 向き合ってグリッドでないと実行しない  (非面对面网格则不执行) */
	if (NPC_Util_isFaceToFace(meindex, index, 1) != TRUE)
		return;

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	for (i = 0; i < arraysizeof(searchtbl); i++) {
		if (searchtbl[i].act == act) {
			if (NPC_Util_GetStrFromStrWithDelim(argstr,
												searchtbl[i].string,
												buf, sizeof(buf)) != NULL) {
				CHAR_talkToCli(index, meindex, buf,
							   CHAR_getWorkInt(meindex, CHAR_WORK_MSGCOLOR));
				break;
			}
		}
	}
}
