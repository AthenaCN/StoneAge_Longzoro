#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "npc_windowhealer.h"

/*--定数--  (常量) */
#define RATE 1000

/*
 * 設定されたウィンドウを開くNPC  (打开设定窗口的NPC)
 * 簡単なテキストアドベンチャーくらいなら作れるかも  (简单的文字冒险大概能做出来)
 *
 */

enum {
	CHAR_WORK_LEVEL = CHAR_NPCWORKINT1,
	CHAR_WORK_RANGE = CHAR_NPCWORKINT2,
	CHAR_WORK_HP = CHAR_NPCWORKINT3,
	CHAR_WORK_MP = CHAR_NPCWORKINT4,

};

static void NPC_WindowHealer_selectWindow(int meindex, int toindex, int num);
void NPC_WindowHealerAllHeal(int talker, int mode);
BOOL NPC_WindowHealerLevelCheck(int meindex, int talker);
BOOL NPC_WindowMoneyCheck(int meindex, int talker, int mode);
int NPC_WindowCostCheck(int meindex, int talker);
BOOL NPC_PetHealerCheck(int talker);
int NPC_WindowCostCheckMp(int meindex, int talker);

/*********************************
 * 初期化  (初始化)
 *********************************/
BOOL NPC_WindowHealerInit(int meindex) {

	char *npcarg;
	char buf2[256];
	int range = 1;
	int rate;
	double drate;
	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPEHEALER);

	npcarg = CHAR_getChar(meindex, CHAR_NPCARGUMENT);

	/*--回復するときのお金を取るレベルを記憶  (记忆恢复时收取费用的等级) */
	/*--  できなければ作られない--  (做不到则不生成) */
	if (getStringFromIndexWithDelim(npcarg, "|", 1, buf2, sizeof(buf2)) != FALSE) {
		CHAR_setWorkInt(meindex, CHAR_WORK_LEVEL, atoi(buf2));

	} else {
		return FALSE;
	}
	/*--回復  レベル--  (恢复等级) */
	if (getStringFromIndexWithDelim(npcarg, "|", 2, buf2, sizeof(buf2)) != FALSE) {
		drate = atof(buf2);
		if (drate == 0) {
			rate = 500;
		} else {
			rate = (int)(drate * RATE);
		}
		CHAR_setWorkInt(meindex, CHAR_WORK_HP, rate);
	}
	if (getStringFromIndexWithDelim(npcarg, "|", 3, buf2, sizeof(buf2)) != FALSE) {
		drate = atof(buf2);
		if (drate == 0) {
			rate = 2000;
		} else {
			rate = (int)(drate * RATE);
		}
		CHAR_setWorkInt(meindex, CHAR_WORK_MP, rate);
	}
	if (getStringFromIndexWithDelim(npcarg, "|", 4, buf2, sizeof(buf2)) != FALSE) {
		range = atoi(buf2);
		if (range == 0) {
			range = 1;
		}
	}
	CHAR_setWorkInt(meindex, CHAR_WORK_RANGE, range);
	return TRUE;
}

void NPC_WindowHealerTalked(int meindex, int talkerindex, char *szMes, int color) {
	if (CHAR_getInt(talkerindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) {
		return;
	}

	if (NPC_Util_CharDistance(talkerindex, meindex) > CHAR_getWorkInt(meindex, CHAR_WORK_RANGE)) {
		return;
	}
	if ((CHAR_getWorkInt(talkerindex, CHAR_WORKPARTYMODE) == 0) || (CHAR_getWorkInt(talkerindex, CHAR_WORKPARTYMODE) == 2)) {
		NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);
	} else {
		int i = 0;
		int otherindex;

		for (i = 0; i < CHAR_PARTYMAX; i++) {
			otherindex = CHAR_getWorkInt(talkerindex, CHAR_WORKPARTYINDEX1 + i);
			if (otherindex != -1) {
				NPC_WindowHealer_selectWindow(meindex, otherindex, 0);
			}
		}
	}
}
void NPC_WindowHealerLooked(int meindex, int lookedindex) {
	if (CHAR_getInt(lookedindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) {
		return;
	}
	if (NPC_Util_CharDistance(lookedindex, meindex) > CHAR_getWorkInt(meindex, CHAR_WORK_RANGE)) {
		return;
	}
	NPC_WindowHealer_selectWindow(meindex, lookedindex, 0);
}

static void NPC_WindowHealer_selectWindow(int meindex, int toindex, int num) {

	char token[1024];
	char escapedname[1024];
	int fd = getfdFromCharaIndex(toindex);
	int buttontype = 0;
	int windowtype = 0;
	int windowno = 0;

	/*--ウインドウタイプメッセージが多いので先に設定  (窗口类型消息较多所以先设定) */
	windowtype = WINDOW_MESSAGETYPE_MESSAGE;

	switch (num) {
	case 0:
		/*--選択 0--  (选项0) */
		sprintf(token, " 　　　哎呀！你受伤了吗？　 \n\n "
					   "　　　　　　 ＜ 耐久力回复＞　　　　　 \n"
					   " 　　　　　　＜  气力回复 ＞　　　　　 \n"
					   " 　　　　 ＜ 耐久力·气力回复 ＞　　 \n\n"
					   " 　　　　　 ＜宠物回复(免费)＞　　　　 ");

		buttontype = WINDOW_BUTTONTYPE_CANCEL;
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_STARTMSG;
		break;

	case 1:
		if (CHAR_getInt(toindex, CHAR_HP) == CHAR_getWorkInt(toindex, CHAR_WORKMAXHP)) {
			if (NPC_PetHealerCheck(toindex) == FALSE) {
				sprintf(token,
						"　　　　　　 ＜　耐久力回复　＞"
						"\n\n\n\n　　　　似乎没有必要回复唷！　");
			} else {
				sprintf(token,
						"　　　　　　 ＜　耐久力回复　＞"
						"\n\n　　　　      似乎没有必要回复唷！　"
						"\n\n　　　因为宠物好像也受伤了！"
						"\n                            先帮他回复吧！");

				NPC_WindowHealerAllHeal(toindex, 0);
			}

			buttontype = WINDOW_BUTTONTYPE_OK;
			windowno = CHAR_WINDOWTYPE_WINDOWHEALER_HPMSG;
			break;
		} else if (NPC_WindowHealerLevelCheck(meindex, toindex) == TRUE) {
			sprintf(token, "　　　　　　 ＜　耐久力回复　＞"
						   "\n\n\n　　　　　　　是要回复耐久力没错吧！　　　 "
						   "\n\n　现在的等级可以免费帮你回复唷！");

		} else {
			int gold;
			gold = NPC_WindowCostCheck(meindex, toindex);
			sprintf(token, "　　　　　　 ＜　耐久力回复　＞"
						   "\n\n\n　　　　　　　是要回复耐久力没错吧！　　　 "
						   "\n\n　　　　　收您%d的STONE 。",
					gold);
		}

		buttontype = WINDOW_BUTTONTYPE_YESNO;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_HPMSG;
		break;

	case 2:
		if (CHAR_getInt(toindex, CHAR_MP) == CHAR_getWorkInt(toindex, CHAR_WORKMAXMP)) {
			if (NPC_PetHealerCheck(toindex) == FALSE) {
				sprintf(token, "　　　　　　  ＜　气力回复　＞"
							   "\n\n\n\n　　　　似乎没有必要回复唷！　");
			} else {
				sprintf(token, "　　　　　　  ＜　气力回复　＞"
							   "\n\n　　　　      似乎没有必要回复唷！　"
							   "\n\n　　　因为宠物好像也受伤了！"
							   "\n                            先帮他回复吧！");
				NPC_WindowHealerAllHeal(toindex, 0);
			}
			buttontype = WINDOW_BUTTONTYPE_OK;
			windowno = CHAR_WINDOWTYPE_WINDOWHEALER_SPIRITMSG;
			break;
		}

		if (NPC_WindowHealerLevelCheck(meindex, toindex) == TRUE) {
			sprintf(token, "　　　　　　  ＜ 气力回复＞"
						   "\n\n　　　　　　　是要回复气力没错吧！　　　　 "
						   "\n\n 现在的等级可以免费帮你回复唷！");
		} else {
			int cost;
			cost = NPC_WindowCostCheckMp(meindex, toindex);
			sprintf(token, "　　　　　　  ＜ 气力回复＞"
						   "\n\n\n　　　　　　　是要回复气力没错吧！ "
						   "\n\n　　　　收您%d的STONE 。",
					cost);
		}
		buttontype = WINDOW_BUTTONTYPE_YESNO;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_SPIRITMSG;
		break;

	case 3:
		sprintf(token, "\n　　　　　　＜耐久力已回复＞"
					   "\n　　  ＜宠物也已回复一般状态＞"
					   "\n\n\n　　　　　  这样子就没问题了！　　　　");

		/*--回復のみ回復させる--  (仅恢复体力) */
		NPC_WindowHealerAllHeal(toindex, 1);
		buttontype = WINDOW_BUTTONTYPE_OK;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_OKHPMSG;
		break;

	case 4:
		sprintf(token, "\n　　　　　　＜气力已回复＞"
					   "\n　　 ＜宠物也已回复一般状态＞"
					   "\n\n\n　　　　　 这样子就没问题了！ 　　　");
		/*--気のみ回復させる---  (仅恢复法力) */
		NPC_WindowHealerAllHeal(toindex, 2);
		buttontype = WINDOW_BUTTONTYPE_OK;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_OKSPIRITMSG;
		break;

	case 6: {
		int cost = 0;

		if (CHAR_getInt(toindex, CHAR_HP) < CHAR_getWorkInt(toindex, CHAR_WORKMAXHP)) {
			cost += NPC_WindowCostCheck(meindex, toindex);
		}

		if (CHAR_getInt(toindex, CHAR_MP) < CHAR_getWorkInt(toindex, CHAR_WORKMAXMP)) {
			cost = cost + NPC_WindowCostCheckMp(meindex, toindex);
		}

		if (cost == 0) {
			if (NPC_PetHealerCheck(toindex) == FALSE) {
				sprintf(token,
						"　　　　  ＜　耐久力气力回复　＞"
						"\n\n\n\n　　　　似乎没有必要回复唷！　");
			} else {
				sprintf(token, "　　　　  ＜　耐久力气力回复　＞"
							   "\n\n　　　　      似乎没有必要回复唷！　"
							   "\n\n　　　因为宠物好像也受伤了！"
							   "\n                            先帮他回复吧！");
				NPC_WindowHealerAllHeal(toindex, 0);
			}

			buttontype = WINDOW_BUTTONTYPE_OK;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			windowno = CHAR_WINDOWTYPE_WINDOWHEALER_SPIRITMSG;
			break;

		} else {
			sprintf(token, "　　　  ＜　耐久力气力回复　＞"
						   "\n\n\n　　　　 是要回复耐久力气力没错吧！"
						   "\n\n　　　　　收您%d的STONE 。",
					cost);
		}
	}

		if (NPC_WindowHealerLevelCheck(meindex, toindex) == TRUE) {
			sprintf(token, "　　　　  ＜　耐久力气力回复　＞"
						   "\n\n\n　　　　 　是要回复耐久力气力没错吧！"
						   "\n\n　现在的等级可以免费帮你回复唷！");
		}

		buttontype = WINDOW_BUTTONTYPE_YESNO;
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_ALLMSG;
		break;

	case 7:
		sprintf(token, "　　　　 ＜耐久力气力已回复＞"
					   "\n　　  ＜宠物也已回复一般状态＞"
					   "\n\n\n　　　　这样一来就回复健康了！");

		NPC_WindowHealerAllHeal(toindex, 3);
		buttontype = WINDOW_BUTTONTYPE_OK;
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_OKALLMSG;
		break;

	case 8:
		sprintf(token, "\n\n\n\n 　　真可惜似乎所带的金钱不够唷！");

		buttontype = WINDOW_BUTTONTYPE_OK;
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_OKHPMSG;
		break;

	case 9:
		sprintf(token, "　　　　　　　＜宠物回复＞　　　　　　"
					   "\n\n\n　　　　　　　已经没问题了啦！　　　　　"
					   "\n\n　　但是太勉强的话也不行唷！　　");
		NPC_WindowHealerAllHeal(toindex, 0);
		buttontype = WINDOW_BUTTONTYPE_OK;
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_OKHPMSG;
		break;

	case 10:
		sprintf(token, "　　　　　　　＜宠物回复＞　　　　　　"
					   "\n\n\n　 似乎没有必要回复宠物的样子。　"
					   "\n\n　　但是太勉强的话也不行唷！　　");
		buttontype = WINDOW_BUTTONTYPE_OK;
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		windowno = CHAR_WINDOWTYPE_WINDOWHEALER_OKHPMSG;
		break;
	}

	makeEscapeString(token, escapedname, sizeof(escapedname));
	/*-ここで送信する--  (在此发送) */
	lssproto_WN_send(fd, windowtype,
					 buttontype,
					 windowno,
					 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
					 escapedname);
}

/*-----------------------------------------
クライアントから返ってきた時に呼び出される  (客户端返回时被调用)
-------------------------------------------*/
void NPC_WindowHealerWindowTalked(int meindex, int talkerindex,
								  int seqno, int select, char *data) {

	if (NPC_Util_CharDistance(talkerindex, meindex) > (CHAR_getWorkInt(meindex, CHAR_WORK_RANGE) + 4))
		return;

	switch (seqno) {
	case CHAR_WINDOWTYPE_WINDOWHEALER_STARTMSG:
		if (atoi(data) == 2) /*--選択した項目   1--  (所选项目1) */
		{
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 1);

		} else if (atoi(data) == 3) {
			/*--選択した項目   2--  (所选项目2) */
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 2);

		} else if (atoi(data) == 4) {
			/*--選択した項目   2--  (所选项目2) */
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 6);

		} else if (atoi(data) == 6) {
			/*--選択した項目   2--  (所选项目2) */
			if (NPC_PetHealerCheck(talkerindex) == TRUE) {
				NPC_WindowHealer_selectWindow(meindex, talkerindex, 9);
			} else {
				NPC_WindowHealer_selectWindow(meindex, talkerindex, 10);
			}
		} else if (select == WINDOW_BUTTONTYPE_CANCEL) {
			/*--ここまでくると終了なので何もしない--  (到这里就结束了所以什么都不做) */
		}
		break;

	/*-----回復回復を行う-----  (进行体力恢复) */
	case CHAR_WINDOWTYPE_WINDOWHEALER_HPMSG:
		if (select == WINDOW_BUTTONTYPE_OK) {
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);

		} else if (select == WINDOW_BUTTONTYPE_YES) {
			/*--お金のチェック--  (金钱检查) */
			if (NPC_WindowMoneyCheck(meindex, talkerindex, 1) == TRUE) {
				NPC_WindowHealer_selectWindow(meindex, talkerindex, 3);
			} else {
				NPC_WindowHealer_selectWindow(meindex, talkerindex, 8);
			}
		} else if (select == WINDOW_BUTTONTYPE_NO) {
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);
		}
		break;

	/*-----気回復を行う-----  (进行法力恢复) */
	case CHAR_WINDOWTYPE_WINDOWHEALER_SPIRITMSG:
		if (select == WINDOW_BUTTONTYPE_OK) {
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);

		} else if (select == WINDOW_BUTTONTYPE_YES) {
			/*--お金のチェック--  (金钱检查) */
			if (NPC_WindowMoneyCheck(meindex, talkerindex, 2) == TRUE) {
				NPC_WindowHealer_selectWindow(meindex, talkerindex, 4);
			} else {
				NPC_WindowHealer_selectWindow(meindex, talkerindex, 8);
			}

		} else if (select == WINDOW_BUTTONTYPE_NO) {
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);
		}

		break;

	/*-----回復回復の結果-----  (体力恢复结果) */
	case CHAR_WINDOWTYPE_WINDOWHEALER_OKHPMSG:
		if (select == WINDOW_BUTTONTYPE_OK) {
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);
		}
		break;

	case CHAR_WINDOWTYPE_WINDOWHEALER_OKSPIRITMSG:
		if (select == WINDOW_BUTTONTYPE_OK) {
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);
		}
		break;

	/*--全回復させる--  (完全恢复) */
	case CHAR_WINDOWTYPE_WINDOWHEALER_ALLMSG:
		if (select == WINDOW_BUTTONTYPE_OK) {
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);

		} else if (select == WINDOW_BUTTONTYPE_YES) {
			if (NPC_WindowMoneyCheck(meindex, talkerindex, 3) == TRUE) {
				NPC_WindowHealer_selectWindow(meindex, talkerindex, 7);
			} else {
				NPC_WindowHealer_selectWindow(meindex, talkerindex, 8);
			}

		} else if (select == WINDOW_BUTTONTYPE_NO) {
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);
		}
		break;

	/*--全回復の結果--  (完全恢复结果) */
	case CHAR_WINDOWTYPE_WINDOWHEALER_OKALLMSG:
		if (select == WINDOW_BUTTONTYPE_YES) {
			NPC_WindowHealer_selectWindow(meindex, talkerindex, 0);
		}
		break;
	}

	if (select == WINDOW_BUTTONTYPE_CANCEL && CHAR_getWorkInt(talkerindex, CHAR_WORKPARTYMODE) == 2) {
		CHAR_sendWatchEvent(CHAR_getWorkInt(talkerindex, CHAR_WORKOBJINDEX),
							CHAR_ACTPLEASURE, NULL, 0, TRUE);
		CHAR_setWorkInt(talkerindex, CHAR_WORKACTION, CHAR_ACTPLEASURE);
	}
}

/*--付いているのならTRUEを返す--  (若带有则返回TRUE) */
BOOL NPC_PetHealerCheck(int talker) {

	int petindex;
	int i;

	/*--ペットはおまけで回復してあげる--  (宠物顺带恢复) */
	for (i = 0; i < CHAR_MAXPETHAVE; i++) {
		petindex = CHAR_getCharPet(talker, i);

		if (petindex == -1)
			continue;

		/*  キャラの    チェック    (角色检查) */
		if (!CHAR_CHECKINDEX(talker))
			continue;

		if (CHAR_getInt(petindex, CHAR_HP) != CHAR_getWorkInt(petindex, CHAR_WORKMAXHP)) {
			return TRUE;
		}
	}

	return FALSE;
}

/*-------------------------------------
	  際に回復と気を回復させるところ  (恢复体力与法力之处)
---------------------------------------*/
void NPC_WindowHealerAllHeal(int talker, int mode) {
	int i;
	int petindex;
	char petsend[64];
	char msgbuf[64];

	/*--回復のみ回復--  (仅恢复体力) */
	if (mode == 1) {
		/*--HPが回復なのである--  (因为HP是体力) */
		CHAR_setInt(talker, CHAR_HP, CHAR_getWorkInt(talker, CHAR_WORKMAXHP));
	} else if (mode == 2) {
		/*--気のみ回復--  (仅恢复法力) */
		/*--MPが気なのである--  (因为MP是法力) */
		CHAR_setInt(talker, CHAR_MP, CHAR_getWorkInt(talker, CHAR_WORKMAXMP));
	} else if (mode == 3) {
		/*--全回復---  (完全恢复) */
		CHAR_setInt(talker, CHAR_HP, CHAR_getWorkInt(talker, CHAR_WORKMAXHP));
		CHAR_setInt(talker, CHAR_MP, CHAR_getWorkInt(talker, CHAR_WORKMAXMP));
	}

	/*--ペットはおまけで回復してあげる--  (宠物顺带恢复) */
	for (i = 0; i < CHAR_MAXPETHAVE; i++) {
		petindex = CHAR_getCharPet(talker, i);

		if (petindex == -1)
			continue;

		/*  キャラの    チェック    (角色检查) */
		if (!CHAR_CHECKINDEX(talker))
			continue;

		/*--回復--  (恢复) */
		CHAR_setFlg(petindex, CHAR_ISDIE, 0);
		CHAR_setInt(petindex, CHAR_HP, CHAR_getWorkInt(petindex, CHAR_WORKMAXHP));
		CHAR_setInt(petindex, CHAR_MP, CHAR_getWorkInt(petindex, CHAR_WORKMAXMP));

		/*--パラメータ調整--  (参数调整) */
		CHAR_complianceParameter(petindex);
		sprintf(petsend, "K%d", i);
		CHAR_sendStatusString(talker, petsend);
	}

	/*--パラメータ送り--  (参数发送) */
	CHAR_send_P_StatusString(talker, CHAR_P_STRING_HP);
	CHAR_send_P_StatusString(talker, CHAR_P_STRING_MP);

	/*---仲間がいれば仲間にも送信--  (有同伴则也发给同伴) */
	if (CHAR_getWorkInt(talker, CHAR_WORKPARTYMODE) != CHAR_PARTY_NONE) {
		int topartyarray = -1;
		int oyaindex = CHAR_getWorkInt(talker, CHAR_WORKPARTYINDEX1);

		if (CHAR_CHECKINDEX(oyaindex)) {
			int i;

			/* 自分と組んだやつの仲間の    場所を取る   (取自己与组队者的同伴位置) */
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
}

/*-------------------------------------
レベルチェック  (等级检查)
レベルを見て設定レベルより低ければ制限する  (查看等级，低于设定等级则限制)
  戻り値  (返回值)
	設定レベルより低い    	TRUE  (低于设定等级 TRUE)
	設定レベル以上なら	FALSE  (达到设定等级以上 FALSE)
---------------------------------------*/
BOOL NPC_WindowHealerLevelCheck(int meindex, int talker) {
	int level;

	level = CHAR_getWorkInt(meindex, CHAR_WORK_LEVEL);

	if (level > CHAR_getInt(talker, CHAR_LV)) {
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------
お金  持ってるかチェック  (检查是否有钱)
番号  (编号)
meindex	のインデックス  (meindex 的索引)
talker	キャラクタのインデックス  (角色索引)
---------------------------------------------*/
BOOL NPC_WindowMoneyCheck(int meindex, int talker, int mode) {
	int cost = 0;
	int level;

	level = CHAR_getWorkInt(meindex, CHAR_WORK_LEVEL);

	if (mode == 1) {
		if (level <= CHAR_getInt(talker, CHAR_LV)) {
			/*---お金を割り出す---  (算出金钱) */
			cost = NPC_WindowCostCheck(meindex, talker);
			/*--現在は仮設定---  (当前为临时设定) */
			/*---お金が足りるかどうかのチェック---  (检查金钱是否充足) */
			if (CHAR_getInt(talker, CHAR_GOLD) < cost) {
				return FALSE;
			}
			CHAR_DelGold(talker, cost);
		}
	}
	if (mode == 2) {
		if (level <= CHAR_getInt(talker, CHAR_LV)) {
			/*---お金を割り出す---  (算出金钱) */
			cost = NPC_WindowCostCheckMp(meindex, talker);

			/*--現在は仮設定---  (当前为临时设定) */
			/*---お金が足りるかどうかのチェック---  (检查金钱是否充足) */
			if (CHAR_getInt(talker, CHAR_GOLD) < cost) {
				return FALSE;
			}
			CHAR_DelGold(talker, cost);
		}
	}

	if (mode == 3) {
		if (level <= CHAR_getInt(talker, CHAR_LV)) {

			/*--回復のお金の計算--  (体力费用计算) */
			if (CHAR_getInt(talker, CHAR_HP) < CHAR_getWorkInt(talker, CHAR_WORKMAXHP)) {
				cost = NPC_WindowCostCheck(meindex, talker);
			}

			/*--気のお金の計算--  (法力费用计算) */
			if (CHAR_getInt(talker, CHAR_MP) < CHAR_getWorkInt(talker, CHAR_WORKMAXMP)) {
				cost = cost + NPC_WindowCostCheckMp(meindex, talker);
			}

			/*--仮設定--  (临时设定) */
			if (CHAR_getInt(talker, CHAR_GOLD) < cost) {
				return FALSE;
			}

			/*---お金を減らす---  (减少金钱) */
			CHAR_DelGold(talker, cost);
		}
	}

	CHAR_send_P_StatusString(talker, CHAR_P_STRING_GOLD);

	return TRUE;
}

/*-----------------------------------------
コストの計算  (成本计算)
------------------------------------------*/
int NPC_WindowCostCheck(int meindex, int talker) {
	int cost;
	double drate;
	int rate;

	rate = CHAR_getWorkInt(meindex, CHAR_WORK_HP);
	drate = (double)rate / 1000;
	cost = CHAR_getInt(talker, CHAR_LV);
	cost = (int)cost * drate;
	if (cost < 1)
		cost = 1;
	return cost;
}

/*-----------------------------------------
竣  気コストの計算  (法力成本计算)
------------------------------------------*/
int NPC_WindowCostCheckMp(int meindex, int talker) {
	int cost;
	double drate;
	int rate;

	rate = CHAR_getWorkInt(meindex, CHAR_WORK_MP);
	drate = (double)rate / 1000;
	cost = CHAR_getInt(talker, CHAR_LV);
	cost = (int)cost * drate;
	if (cost == 0)
		cost = 1;
	return cost;
}
