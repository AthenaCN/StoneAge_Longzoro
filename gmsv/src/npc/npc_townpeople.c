#include "version.h"
#include "object.h"
#include "char_base.h"
#include "char.h"
#include "util.h"
#include "handletime.h"
#include "anim_tbl.h"
#include "npc_door.h"
#include "lssproto_serv.h"
#include "npcutil.h"

/*
 *  まちのひとびと  (镇上的居民)  by nakamura
 *  タイプ  “TownPeople  (类型 “TownPeople)
 *    しかけられたときにNPCARGUMENTの値をしかけてきた相手に渡す  (被设置时把NPC参数值传给设置者)
 *
 *  またnpcargに こんにちは、今は天気がいいですね  (还有，npcarg里有"你好，今天天气真好呢")
 *  のように セミコロンをデリミタとして  数のメッセージを  (像这样用分号作分隔符，把多条消息)
 *  書くことができ、その場合ランダムでその中からしゃべる  (可以书写多条，此时随机从中说一条)
 *  npcgen.perl では MANである。MSGとほとんどおなじだなあ  (在npcgen.perl中为MAN，与MSG几乎相同)
 *
 */

/*
 *   しかけられたときはNPCARGUMENTをそのまま渡す  (被设置时原样传递NPC参数)
 */
void NPC_TownPeopleTalked(int index, int talker, char *msg, int color) {
	char arg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];
	int i, tokennum;

	/* グリッド移動の場合だけ返答する  (仅网格移动时才回应) */

	if (CHAR_getInt(talker, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER && NPC_Util_charIsInFrontOfChar(talker, index, 3)) {

		NPC_Util_GetArgStr(index, arg, sizeof(arg));

		tokennum = 1;
		/* コンマで区切られたトークンが何個あるか数える  (统计逗号分隔的标记数量) */
		for (i = 0; arg[i] != '\0'; i++) {
			if (arg[i] == ',')
				tokennum++;
		}

		/* ランダムでどれを選ぶか決めてそのトークンを取り出す  (随机决定选哪个并取出该标记) */
		getStringFromIndexWithDelim(arg, ",",
									rand() % tokennum + 1, token, sizeof(token));

		CHAR_talkToCli(talker, index, token, CHAR_COLORWHITE);
	}
}

/*
 * 初期化する  (初始化)
 */
BOOL NPC_TownPeopleInit(int meindex) {

	// CHAR_setInt( meindex , CHAR_HP , 0 );
	// CHAR_setInt( meindex , CHAR_MP , 0 );
	// CHAR_setInt( meindex , CHAR_MAXMP , 0 );
	// CHAR_setInt( meindex , CHAR_STR , 0 );
	// CHAR_setInt( meindex , CHAR_TOUGH, 0 );
	// CHAR_setInt( meindex , CHAR_LV , 0 );

	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPETOWNPEOPLE);
	// CHAR_setFlg( meindex , CHAR_ISOVERED , 1 );
	// CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );  /*   撃されないように   (使其不被攻击) */

	return TRUE;
}
