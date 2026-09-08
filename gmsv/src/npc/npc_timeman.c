#include "version.h"
#include <string.h>
#include "object.h"
#include "char_base.h"
#include "char.h"
#include "util.h"
#include "npcutil.h"
#include "npc_oldman.h"
#include "title.h"
#include "char_data.h"
#include "readmap.h"
#include "lssproto_serv.h"
#include "log.h"
#include "handletime.h"

enum {
	E_INT_GRAPHIC = CHAR_NPCWORKINT1,	 /*   胸の画像  (胸部的图像) */
	E_INT_GRAPHIC2 = CHAR_NPCWORKINT2,	 /*   身の画像  (身体的图像) */
	E_INT_BORN = CHAR_NPCWORKINT3,		 /* 誕生時間  (诞生时间) */
	E_INT_DEAD = CHAR_NPCWORKINT4,		 /* 消える時間  (消失时间) */
	E_INT_MODE = CHAR_NPCWORKINT5,		 /* 現在のモード  (当前模式) */
	E_INT_NOWGRAPHIC = CHAR_NPCWORKINT6, /* 現在のグラフィックナンバー  (当前图像编号) */

};

typedef struct {
	char arg[32];
	int born;
	int dead;
} NPC_TimeMan;

#define YOAKE 700
#define NICHIBOTU 300
#define SHOUGO 125
#define YONAKA 500

static NPC_TimeMan TimeTble[] = {

	{"ALLNIGHT", NICHIBOTU + 1, YOAKE}, /* 月が開いていない時  (月亮未升起时) */
	{"ALLNOON", YOAKE + 1, NICHIBOTU},	/* 月が開いている時  (月亮升起时) */
	{"AM", YONAKA + 1, SHOUGO},			/* AM 時のみ  (仅AM时段) */
	{"PM", SHOUGO + 1, YONAKA},			/* PM 時のみ  (仅PM时段) */
	{"FORE", YOAKE + 1, SHOUGO},		/* 月が出てから昼まで  (月亮出来后到中午) */
	{"AFTER", SHOUGO + 1, NICHIBOTU},	/* 昼から月が沈むまで  (从中午到月亮落下) */
	{"EVNING", NICHIBOTU + 1, YONAKA},	/* 月が沈んでから真夜まで  (月亮落下后到深夜) */
	{"MORNING", YONAKA + 1, YOAKE},		/* 真夜から月が昇るまで  (从深夜到月亮升起) */
	{"FREE", 0, 1024},
};

BOOL NPC_TimeManInit(int meindex) {

	int i = 0;
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[32];

	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("GetArgStrErr");
		return FALSE;
	}

	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPETOWNPEOPLE);
	if (NPC_Util_GetStrFromStrWithDelim(argstr, "change_no", buf, sizeof(buf)) != NULL) {
		if (strstr(buf, "CLS") != NULL) {
			CHAR_setWorkInt(meindex, E_INT_GRAPHIC2, 9999);
		} else {
			CHAR_setWorkInt(meindex, E_INT_GRAPHIC2, atoi(buf));
		}
	} else {
		CHAR_setWorkInt(meindex, E_INT_GRAPHIC2, 9999);
	}

	CHAR_setWorkInt(meindex, E_INT_GRAPHIC, CHAR_getInt(meindex, CHAR_BASEIMAGENUMBER));

	if (NPC_Util_GetStrFromStrWithDelim(argstr, "time", buf, sizeof(buf)) != NULL) {
		for (i = 0; i < 9; i++) {
			if (strstr(buf, TimeTble[i].arg) != NULL) {
				CHAR_setWorkInt(meindex, E_INT_BORN, TimeTble[i].born);
				CHAR_setWorkInt(meindex, E_INT_DEAD, TimeTble[i].dead);
				return TRUE;
			}
		}
	}

	return FALSE;
}

void NPC_TimeManTalked(int meindex, int talkerindex, char *msg, int color) {
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char token[512];
	char buf[512];
	int tokennum = 0;
	int i;

	/*--  目の前にあるかどうか  (是否在眼前) */
	if (NPC_Util_isFaceToFace(meindex, talkerindex, 2) == FALSE) {
		/* グリッド移動のみ   (仅网格移动) */
		if (NPC_Util_CharDistance(talkerindex, meindex) > 1)
			return;
	}

	if (CHAR_getInt(meindex, CHAR_BASEIMAGENUMBER) == 9999) {
		/*--消えているモード--  (消失状态) */
		return;
	}

	if (NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr)) == NULL) {
		print("GetArgStrErr");
		return;
	}

	if (CHAR_getInt(talkerindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER)
		return;

	if (NPC_Util_charIsInFrontOfChar(talkerindex, meindex, 3) == FALSE)
		return;

	if (CHAR_getWorkInt(meindex, E_INT_MODE) == 0) {
		/*--メッセージを返す  (返回消息) */
		NPC_Util_GetStrFromStrWithDelim(argstr, "main_msg", buf, sizeof(buf));

	} else {
		/*グラフィックを変えただけなので違うメッセージを返す  (因为只是改了图像，所以返回不同消息) */
		NPC_Util_GetStrFromStrWithDelim(argstr, "change_msg", buf, sizeof(buf));
	}

	tokennum = 1;
	/* コンマで区切られたトークンが何個あるか数える  (统计逗号分隔的标记数量) */
	for (i = 0; buf[i] != '\0'; i++) {
		if (buf[i] == ',')
			tokennum++;
	}

	/* ランダムでどれを選ぶか決めてそのトークンを取り出す  (随机决定选哪个并取出该标记) */
	getStringFromIndexWithDelim(buf, ",", rand() % tokennum + 1, token, sizeof(token));
	/*--しゃべる--  (说话) */
	CHAR_talkToCli(talkerindex, meindex, token, CHAR_COLORWHITE);
}

/** 周りに何かが起こったときに呼ばれる  (周围发生什么时被调用) */
void NPC_TimeManWatch(int meobjindex, int objindex, CHAR_ACTION act,
					  int x, int y, int dir, int *opt, int optlen) {
	int meindex = 0;
	int index;
	LSTIME nowlstime;
	int born;
	int dead;

	if (OBJECT_getType(objindex) != OBJTYPE_CHARA)
		return;
	index = OBJECT_getIndex(objindex);

	/* プレイヤーのみ実行する  (仅对玩家执行) */
	if (CHAR_getInt(index, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER)
		return;

	meindex = OBJECT_getIndex(meobjindex);
	born = CHAR_getWorkInt(meindex, E_INT_BORN);
	dead = CHAR_getWorkInt(meindex, E_INT_DEAD);

	//	print("now_hour=%d",nowlstime.hour);

	/* 時間を見て状態を決める  (根据时间决定状态) */
	if (born < dead) {
		RealTimeToLSTime(NowTime.tv_sec, &nowlstime);
		if ((born < nowlstime.hour) && (dead > nowlstime.hour)) {
			if (CHAR_getWorkInt(meindex, E_INT_NOWGRAPHIC) == CHAR_getWorkInt(meindex, E_INT_GRAPHIC))
				return;

			CHAR_setInt(meindex, CHAR_BASEIMAGENUMBER,
						CHAR_getWorkInt(meindex, E_INT_GRAPHIC));
			CHAR_sendCToArroundCharacter(CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX));
			CHAR_setWorkInt(meindex, E_INT_MODE, 0);
			CHAR_setWorkInt(meindex, E_INT_NOWGRAPHIC,
							CHAR_getInt(meindex, CHAR_BASEIMAGENUMBER));
		} else {
			/*-消えている--  (正在消失) */
			if (CHAR_getWorkInt(meindex, E_INT_NOWGRAPHIC) == CHAR_getWorkInt(meindex, E_INT_GRAPHIC2))
				return;

			CHAR_setInt(meindex, CHAR_BASEIMAGENUMBER,
						CHAR_getWorkInt(meindex, E_INT_GRAPHIC2));
			CHAR_sendCToArroundCharacter(CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX));
			CHAR_setWorkInt(meindex, E_INT_MODE, 1);
			CHAR_setWorkInt(meindex, E_INT_NOWGRAPHIC,
							CHAR_getInt(meindex, CHAR_BASEIMAGENUMBER));
		}
	} else {
		RealTimeToLSTime(NowTime.tv_sec, &nowlstime);
		if ((born < nowlstime.hour && 1024 > nowlstime.hour) || (0 < nowlstime.hour && dead > nowlstime.hour)) {
			if (CHAR_getWorkInt(meindex, E_INT_NOWGRAPHIC) == CHAR_getWorkInt(meindex, E_INT_GRAPHIC))
				return;

			CHAR_setInt(meindex, CHAR_BASEIMAGENUMBER,
						CHAR_getWorkInt(meindex, E_INT_GRAPHIC));
			CHAR_sendCToArroundCharacter(CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX));
			CHAR_setWorkInt(meindex, E_INT_MODE, 0);
			CHAR_setWorkInt(meindex, E_INT_NOWGRAPHIC,
							CHAR_getInt(meindex, CHAR_BASEIMAGENUMBER));
		} else {
			if (CHAR_getWorkInt(meindex, E_INT_NOWGRAPHIC) == CHAR_getWorkInt(meindex, E_INT_GRAPHIC2))
				return;

			CHAR_setInt(meindex, CHAR_BASEIMAGENUMBER,
						CHAR_getWorkInt(meindex, E_INT_GRAPHIC2));
			CHAR_sendCToArroundCharacter(CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX));
			CHAR_setWorkInt(meindex, E_INT_MODE, 1);
			CHAR_setWorkInt(meindex, E_INT_NOWGRAPHIC,
							CHAR_getInt(meindex, CHAR_BASEIMAGENUMBER));
		}
	}
}
