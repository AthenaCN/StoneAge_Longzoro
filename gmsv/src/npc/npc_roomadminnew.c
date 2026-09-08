#include "version.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "object.h"
#include "char.h"
#include "char_base.h"
#include "char_data.h"
#include "net.h"
#include "npcutil.h"
#include "handletime.h"
#include "readmap.h"
#include "npc_roomadminnew.h"
#include "npc_door.h"

/*
 * ランキングデータ管理 By Kawata
 * ファイル形式が変わった時の読み書き処理
 */

enum {
	/* オークション区  (拍卖区) */
	CHAR_WORKINT_RANGE = CHAR_NPCWORKINT1,
	/* 指定の締め切り時間  (指定的截止时间) */
	CHAR_WORKINT_NEXTTIME = CHAR_NPCWORKINT2,
	/*   表示用＞締め切り数分前＝  (显示用「截止前几分钟」) */
	CHAR_WORKINT_PRINTCLOSETIME = CHAR_NPCWORKINT3,
	/*   検索した部屋の鍵  (搜索到的房间钥匙) */
	CHAR_WORKINT_SEARCHROOMTIME = CHAR_NPCWORKINT4,

};
enum {
	/* 部屋の前  (房间前) */
	CHAR_WORKCHAR_MYNAME = CHAR_NPCWORKCHAR1,
	/* 指定の部屋  (指定的房间) */
	CHAR_WORKCHAR_NEXTROOM = CHAR_NPCWORKCHAR2,

};

/* ランキング表示用ワーク  (排行榜显示用工作区)*/
enum {
	/* 部屋の前  (房间前) */
	CHAR_WORKCHAR_ADMNAME = CHAR_NPCWORKCHAR1,
};

/* データファイルを開くディレクトリ（ls2/src/lss からの相対指定）  (打开数据文件的目录（相对ls2/src/lss指定）) */
#define NPC_ROOMADMINNEW_DATADIR "roomadmin/"
/* データファイルの拡張子  (数据文件的扩展名) */
#define NPC_ROOMADMINNEW_EXTENSION ".room"
/* ランキングデータファイルの拡張子（テンポラリ）  (排行榜数据文件的扩展名（临时）)   */
#define NPC_ROOMADMINNEW_RANKING_EXTENSION ".score"
#define NPC_ROOMADMINNEW_RANKING_EXTENSION_TMP ".score.tmp"
/*   最高入札者ランキングファイル  (最高竞标者排行榜文件) */
#define NPC_ROOMADMINNEW_SCOREFILE ".score"
/* 締め切りの数分前からメッセージ  (截止前数分钟发送消息) */
#define NPC_ROOMADMINNEW_PRINTCLOSETIME_DEFAULT 5

#define NPC_ROOMADMINNEW_MSG_CLOSE_BEFORE \
	"%s %s %s的活动，截止日%d分前。"
#define NPC_ROOMADMINNEW_MSG_CLOSE \
	"。。。已截止。"
#define NPC_ROOMADMINNEW_MSG_NONE \
	"很可惜本次没有人获得房间的拥有权。"
#define NPC_ROOMADMINNEW_MSG_CONGRATULATION \
	"恭喜您！%s获得房间的所有权！%s的金钱全部被徵收了。%s请向管理员询问房间暗号。"
#define NPC_ROOMADMINNEW_MSG_PRINTPASSWD_NONE \
	"由於你未持有房间因此没有暗号。"
#define NPC_ROOMADMINNEW_MSG_DSPPASSWD1 \
	"你的房间是 %s %s %s。"
#define NPC_ROOMADMINNEW_MSG_DSPPASSWD2 \
	"房间的暗号是 「%s 」。只要在对着门说话时在与尾加上「...」，别人就看不见了唷！"
#define NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION_NONE \
	"本日的活动结束。"
#define NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION1 \
	"接下来的房间是 %s %s %s。"
#define NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION2 \
	"截止日是 %d:%d 。到截止日为止还有%2d小时%02d分。"
#define NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION3 \
	"最低得标金额是 %s 。"

#define NPC_ROOMADMINNEW_MSG_RANKING_INFO \
	"这是高额得标者排行榜。"
#define NPC_ROOMADMINNEW_MSG_RANKING \
	"Rank %2d:%4d/%02d/%02d %8d GOLD %s%s "

/* このNPCのメッセージ情報  (此NPC的消息信息) */
#define NPC_ROOMADMINNEW_MSGCOLOR CHAR_COLORWHITE
#define NPC_RANKING_MSGCOLOR CHAR_COLORWHITE
#define NPC_ROOMADMINNEW_2WEEK 60 * 60 * 24 * 14

static void NPC_RoomAdminNew_DspNextAuction(int meindex, int talkerindex);
static void NPC_RoomAdminNew_DspRoomPasswd(int meindex, int adminindex, int talkerindex);
static BOOL NPC_RoomAdminNew_SetNextAuction(int meindex);
static BOOL NPC_RoomAdminNew_WriteDataFile(int meindex, int pindex, char *roomname,
										   char *passwd, BOOL flg);
static void NPC_RoomAdminNew_CreatePasswd(char *passwd, int passlen);
static void NPC_RoomAdminNew_SendMsgThisFloor(int index, char *buff);
static int NPC_RoomAdminNew_SearchPlayerHasMostMoney(int meindex);
static void NPC_RoomAdminNew_CloseProc(int meindex);
static int NPC_RoomAdminNew_WriteScoreFile(int meindex, int pindex);
static void NPC_RoomAdminNew_ReadScoreFile(int meindex, int talkerindex,
										   char *admname);

/************************************************
 * 初期化  (初始化)
 ************************************************/
BOOL NPC_RoomAdminNewInit(int meindex) {
	int i;
	char buf[256];
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

	/* 部屋のドアが開くまでこのNPCは動作されない  (房间门打开前此NPC不运行) */
	for (i = 3;; i++) {
		if (getStringFromIndexWithDelim(argstr, "|", i, buf, sizeof(buf))) {
			if (NPC_DoorSearchByName(buf) < 0) {
				print("admin not created.在密码门做好之前先等待。\n");
				return FALSE;
			}
		} else {
			break;
		}
	}

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	/* 初期化  (初始化)   */

	/* typeセット  (设置type) */
	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPEROOMADMIN);
	/*   有効期限取得  (获取有效期)   */
	getStringFromIndexWithDelim(argstr, "|", 1, buf, sizeof(buf));
	CHAR_setWorkInt(meindex, CHAR_WORKINT_RANGE, atoi(buf));
	/* 部屋の前を取得  (获取房间前)   */
	if (!getStringFromIndexWithDelim(argstr, "|", 2, buf, sizeof(buf))) {
		print("arg err.admin not created.\n");
		return (FALSE);
	}
	CHAR_setWorkChar(meindex, CHAR_WORKCHAR_MYNAME, buf);

	/*   表示用＞締め切り数分前＝初期化  (显示用「截止前几分钟」的初始化) */
	CHAR_setWorkInt(meindex, CHAR_WORKINT_PRINTCLOSETIME, 0);

	/* 指定の部屋とオークション時間をセット  (设置指定房间与拍卖时间) */
	NPC_RoomAdminNew_SetNextAuction(meindex);

	return TRUE;
}
/************************************************
 * メインループ
 * 呼び出し頻度は高くなくてもいい
 * でも絶対に分けることにする
 ************************************************/
void NPC_RoomAdminNewLoop(int meindex) {
	int hun;
	/* もしも時間が決まっていなかったら取り直す  (若时间未确定则重新获取) */
	if (CHAR_getWorkInt(meindex, CHAR_WORKINT_NEXTTIME) == 0) {
		/* 前回検索した部屋が違えば検索し直す  (若上次搜索的房间不同则重新搜索) */
		struct tm tm1, tm2;
		time_t t;
		memcpy(&tm1, localtime((time_t *)&NowTime.tv_sec), sizeof(tm1));
		t = CHAR_getWorkInt(meindex, CHAR_WORKINT_SEARCHROOMTIME);
		memcpy(&tm2, localtime(&t),
			   sizeof(tm2));
		if (tm1.tm_year != tm2.tm_year ||
			tm1.tm_mday != tm2.tm_mday ||
			tm1.tm_mon != tm2.tm_mon) {
			NPC_RoomAdminNew_SetNextAuction(meindex);
		}
		return;
	}

	hun = ceil((double)(CHAR_getWorkInt(meindex, CHAR_WORKINT_NEXTTIME) -
						NowTime.tv_sec) /
			   60.0);
	/* もしものためにhunがマイナスの値なら0にする  (以防万一，hun为负值时置0) */
	if (hun < 0)
		hun = 0;

	if (hun < NPC_ROOMADMINNEW_PRINTCLOSETIME_DEFAULT) {
		/* 締め切り近くになったら「締め切りの数分前です」と伝える  (临近截止时告知“还有几分钟截止”) */
		if (hun != CHAR_getWorkInt(meindex, CHAR_WORKINT_PRINTCLOSETIME)) {
			char buff[256];
			if (hun != 0) {
				char buf2[1024];
				char msgbuff[16];
				char argstr2[NPC_UTIL_GETARGSTR_BUFSIZE];
				int doorindex;

				/*   部屋データ取得  (获取房间数据)   */
				doorindex = NPC_DoorSearchByName(CHAR_getWorkChar(meindex, CHAR_WORKCHAR_NEXTROOM));
				NPC_Util_GetArgStr(doorindex, argstr2, sizeof(argstr2));
				getStringFromIndexWithDelim(argstr2, "|", 9,
											buf2, sizeof(buf2));
				/*   部屋データ  (房间数据)  */
				if (strlen(CHAR_getChar(doorindex, CHAR_NAME)) != 0) {
					strcpy(msgbuff, "号室");
				} else {
					msgbuff[0] = '\0';
				}
				/* ＞締め切りの数分前です＝  (「截止前几分钟」) */
				snprintf(buff, sizeof(buff),
						 NPC_ROOMADMINNEW_MSG_CLOSE_BEFORE,
						 buf2,
						 CHAR_getChar(doorindex, CHAR_NAME),
						 msgbuff,
						 hun);

				CHAR_setWorkInt(meindex, CHAR_WORKINT_PRINTCLOSETIME, hun);
			} else {
				/* ＞締め切りました＝  (已截止) */
				strcpy(buff, NPC_ROOMADMINNEW_MSG_CLOSE);
				CHAR_setWorkInt(meindex, CHAR_WORKINT_PRINTCLOSETIME, 0);
			}
			/* この部屋にいる人達に送信  (发送给此房间内的人) */
			NPC_RoomAdminNew_SendMsgThisFloor(meindex, buff);
		}
	}
	/* 締め切った時の値  (截止时的值)   */
	if (hun == 0) {
		NPC_RoomAdminNew_CloseProc(meindex);
	}
}

/************************************************
 *   しかけられた時の値  (被设置时的值)
 ************************************************/
void NPC_RoomAdminNewTalked(int meindex, int talkerindex, char *msg,
							int color) {

	/* プレイヤーがドアマンの1グリッド動いたなら実行する  (玩家距门卫1格时执行) */
	if (NPC_Util_CharDistance(talkerindex, meindex) > 1)
		return;

	/*   しかけられた部屋が鍵を閉めていたらパスワードを表示する  (所在房间上锁时显示密码) */
	/*NPC_RoomAdminNew_DspRoomPasswd( meindex, talkerindex);*/

	/* 指定の時間で鍵を替える  (在指定时间更换钥匙) */
	NPC_RoomAdminNew_DspNextAuction(meindex, talkerindex);
}
/*-----------------------------------------------
 * 締め切りになった時の値  (截止时的值)
 *----------------------------------------------*/
static void NPC_RoomAdminNew_CloseProc(int meindex) {
	int pindex;
	char buff[256];
	char pname[32];
	int failed = TRUE;

	/* お金を抜いて登録しているプレイヤーを探す  (查找登记了提款的玩家) */
	pindex = NPC_RoomAdminNew_SearchPlayerHasMostMoney(meindex);
	/* 該当者あり  (有符合条件者) */
	if (pindex != -1) {
		/* 0件で回るループです  (循环至0件) */
		while (1) {
			char passwd[9];
			int doorindex;
			doorindex = NPC_DoorSearchByName(
				CHAR_getWorkChar(meindex, CHAR_WORKCHAR_NEXTROOM));
			/* パスワードを検索する  (搜索密码) */
			NPC_RoomAdminNew_CreatePasswd(passwd, sizeof(passwd));
			/* ファイルにこの部屋のキー・キャラ・鍵・パスを書き込む  (将房间钥匙、角色、锁、密码写入文件) */
			if (NPC_RoomAdminNew_WriteDataFile(meindex, pindex,
											   CHAR_getWorkChar(meindex, CHAR_WORKCHAR_NEXTROOM),
											   passwd, 1) == FALSE) {
				break;
			}
			/* ドアにパスをセット  (给门设置密码) */
			if (NPC_DoorSetPasswd(doorindex, passwd) == -1) {
				break;
			}
			/*   有効期限をセット  (设置有效期) */
			CHAR_setWorkInt(doorindex, CHAR_WORKDOOREXPIRETIME,
							CHAR_getWorkInt(meindex, CHAR_WORKINT_NEXTTIME) + NPC_ROOMADMINNEW_2WEEK);
			/*   入札者ランキングチェック（ファイル書き込み）  (竞标者排行榜检查（写入文件）) */
			NPC_RoomAdminNew_WriteScoreFile(meindex, pindex);

			CHAR_setInt(pindex, CHAR_GOLD, 0);

			CHAR_send_P_StatusString(pindex, CHAR_P_STRING_GOLD);

			/* おめでとうメッセージ  (祝贺消息) */
			strcpy(pname, CHAR_getChar(pindex, CHAR_NAME));
			snprintf(buff, sizeof(buff),
					 NPC_ROOMADMINNEW_MSG_CONGRATULATION, pname, pname, pname);
			failed = FALSE;
			break;
		}
	}

	/* 該当者なしまたは不正な値  (无匹配者或无效值) */
	if (failed) {
		/* ファイルからこの鍵の部屋のデータを消す  (从文件中删除该上锁房间的数据) */
		NPC_RoomAdminNew_WriteDataFile(meindex, -1,
									   CHAR_getWorkChar(meindex, CHAR_WORKCHAR_NEXTROOM),
									   NULL, 0);
		/* 該当者はいませんメッセージ  (无匹配者消息) */
		snprintf(buff, sizeof(buff),
				 NPC_ROOMADMINNEW_MSG_NONE);
	}
	/* この部屋にいる人達に送信  (发送给此房间内的人) */
	NPC_RoomAdminNew_SendMsgThisFloor(meindex, buff);

	/* 指定の部屋とオークション時間をセット  (设置指定房间与拍卖时间) */
	NPC_RoomAdminNew_SetNextAuction(meindex);
}
/*-----------------------------------------------
 * お金を抜いて登録しているプレイヤーを探す  (查找登记了提款的玩家)
 *----------------------------------------------*/
static int NPC_RoomAdminNew_SearchPlayerHasMostMoney(int meindex) {
	int i, j, range;
	int fl, x, y, gold;
	int doorindex;
	int topindex;

	fl = CHAR_getInt(meindex, CHAR_FLOOR);
	x = CHAR_getInt(meindex, CHAR_X);
	y = CHAR_getInt(meindex, CHAR_Y);
	range = CHAR_getWorkInt(meindex, CHAR_WORKINT_RANGE);
	topindex = -1;

	/*   鍵の部屋の前から高い入札金額を調べる  (在房间前查找最高竞标金额) */
	doorindex = NPC_DoorSearchByName(CHAR_getWorkChar(meindex,
													  CHAR_WORKCHAR_NEXTROOM));
	gold = atoi(CHAR_getWorkChar(doorindex, CHAR_WORKDOORGOLDLIMIT));
	/*   区域にいるキャラの数を調べる  (统计区域内角色数量) */
	for (i = x - range; i <= x + range; i++) {
		for (j = y - range; j <= y + range; j++) {
			OBJECT object;
			for (object = MAP_getTopObj(fl, i, j); object;
				 object = NEXT_OBJECT(object)) {
				int objindex = GET_OBJINDEX(object);
				if (OBJECT_getType(objindex) == OBJTYPE_CHARA) {
					int index;
					index = OBJECT_getIndex(objindex);
					if (CHAR_getInt(index,
									CHAR_WHICHTYPE) == CHAR_TYPEPLAYER) {
						if (CHAR_getInt(index, CHAR_GOLD) >= gold) {
							if (topindex == -1) {
								topindex = index;
							} else {
								/* トップのデータとお金が勝つ  (最高数据与金额胜出) */
								int nowgold, topgold;
								nowgold = CHAR_getInt(index, CHAR_GOLD);
								topgold = CHAR_getInt(topindex, CHAR_GOLD);
								if (nowgold > topgold) {
									topindex = index;
								} else if (nowgold == topgold) {
									/* 一緒ならレベルが高いデータ  (相同时取等级高的数据) */
									int nowlv, toplv;
									nowlv = CHAR_getInt(index, CHAR_LV);
									toplv = CHAR_getInt(topindex, CHAR_LV);
									if (nowlv < toplv) {
										topindex = index;
									}
									/* それでも解決しないならランダムで  (仍无法解决则随机)*/
									/* indexが先のデータがちょっと不公平だけど  (先取index数据略不公平) */
									else if (nowlv == toplv) {
										if (RAND(0, 1)) {
											topindex = index;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return (topindex);
}
/*-----------------------------------------------
 * 指定したindexのデータにいるプレイヤー達に
 * メッセージを送信する［
 *----------------------------------------------*/
static void NPC_RoomAdminNew_SendMsgThisFloor(int index, char *buff) {
	int i, fl;
	int playernum = CHAR_getPlayerMaxNum();

	fl = CHAR_getInt(index, CHAR_FLOOR);
	for (i = 0; i < playernum; i++) {
		if (CHAR_getInt(i, CHAR_FLOOR) == fl) {
			/*   実行のためのプレイヤーかチェック  (检查是否为执行玩家) */
			if (CHAR_getInt(i, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER) {
				CHAR_talkToCli(i, index, buff, NPC_ROOMADMINNEW_MSGCOLOR);
			}
		}
	}
}
/*-----------------------------------------------
 * パスワードを検索する（passlenが-1の時はランダム）
 * makecdn.cの説明［
 *----------------------------------------------*/
static void NPC_RoomAdminNew_CreatePasswd(char *passwd, int passlen) {
#define NPC_ROOMADMINNEW_RAND() ((rand() / 256) % 32768) /* 256で割るのは乱数の質のため  (除以256是为了随机数质量) */
	int i;
	char *candidates[] = {
		"A", "B", "D", "E", "F", "G", "H", "J", "K", "L", "M", "N", "P", "Q", "R",
		"T", "W", "X", "Y", "a", "b", "d", "e", "f", "g", "m", "n", "q", "r", "t",
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
	strcpy(passwd, "");
	for (i = 0; i < passlen - 1; i++) {
		strcat(passwd, candidates[NPC_ROOMADMINNEW_RAND() % (sizeof(candidates) / sizeof(candidates[0]))]);
	}
#undef NPC_ROOMADMINNEW_RAND
}
/*-----------------------------------------------
 * 部屋データをファイルに書き込む
 *
 * ファイルは"房间的名字.room"を参照
 *
 * ファイルのフォーマットは以下
 *
 * 一部のflgを1にして書き込む、0の時はファイル削除
 *    TRUE：保存
 *         FALSE：削除
 *----------------------------------------------*/
static BOOL NPC_RoomAdminNew_WriteDataFile(int meindex, int pindex,
										   char *roomname,
										   char *passwd, int flg) {
	char *cdkey;
	char charaname[32 * 2];
	char expiretime[64];
	char filename[128];
	char outbuf[1024];
	FILE *fp_src;

	/* ファイルを検索  (搜索文件) */
	strcpy(filename, NPC_ROOMADMINNEW_DATADIR);
	/*strcat( filename, CHAR_getWorkChar( meindex, CHAR_WORKCHAR_NEXTROOM));*/
	strcat(filename, roomname);
	strcat(filename, NPC_ROOMADMINNEW_EXTENSION);

	if (flg) {
		/*   検索したデータのキー  (搜索到的数据键) */
		// getcdkeyFromCharaIndex( pindex, cdkey, sizeof(cdkey ));
		cdkey = CHAR_getChar(pindex, CHAR_CDKEY);
		/*   検索したデータのキャラ  (搜索到的数据角色)   */
		makeEscapeString(CHAR_getChar(pindex, CHAR_NAME),
						 charaname, sizeof(charaname));
		/* 期限切れの時間  (过期时间) */
		snprintf(expiretime, sizeof(expiretime), "%d",
				 CHAR_getWorkInt(meindex, CHAR_WORKINT_NEXTTIME) + NPC_ROOMADMINNEW_2WEEK /* 2週間  (2周)   */
		);

		/* デリミタで括る  (用分隔符括起) */
		snprintf(outbuf, sizeof(outbuf), "%s|%s|%s|%s|",
				 cdkey, charaname, passwd, expiretime);
		fp_src = fopen(filename, "w");
		if (!fp_src) {
			print("[%s:%d] File Open Error\n", __FILE__, __LINE__);
			return FALSE;
		}
		fputs(outbuf, fp_src);
		fclose(fp_src);
	} else {
		unlink(filename);
	}

	return TRUE;
}
/*-----------------------------------------------
 * データファイルを検索して該当鍵があればデータを開く［
 *----------------------------------------------*/
BOOL NPC_RoomAdminNew_ReadFile(char *roomname, NPC_ROOMINFO *data) {
	FILE *fp;
	char line[1024];
	char buf2[128];
	char filename[128];

	/* ファイルを検索  (搜索文件) */
	strcpy(filename, NPC_ROOMADMINNEW_DATADIR);
	strcat(filename, roomname);
	strcat(filename, NPC_ROOMADMINNEW_EXTENSION);

	fp = fopen(filename, "r");
	if (!fp)
		return FALSE;
	fgets(line, sizeof(line), fp);
	/* データセット  (设置数据) */
	getStringFromIndexWithDelim(line, "|", 1, buf2, sizeof(buf2));
	strcpy(data->cdkey, buf2);
	getStringFromIndexWithDelim(line, "|", 2, buf2, sizeof(buf2));
	strcpy(data->charaname, makeStringFromEscaped(buf2));
	getStringFromIndexWithDelim(line, "|", 3, buf2, sizeof(buf2));
	strcpy(data->passwd, buf2);
	getStringFromIndexWithDelim(line, "|", 4, buf2, sizeof(buf2));
	if (strcmp(buf2, "max") == 0) {
		data->expire = 0xffffffff;
	} else {
		data->expire = atoi(buf2);
	}
	fclose(fp);
	return TRUE;
}
/*-----------------------------------------------
 * 指定のオークションの時間と部屋を次にセットする  (将指定拍卖的时间与房间设为下一个)
 *----------------------------------------------*/
static BOOL NPC_RoomAdminNew_SetNextAuction(int meindex) {
	int i, j;
	int hitindex = -1;
	int charnum = CHAR_getCharNum();
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	struct tm tmwk;

	/*     などを取得する  (获取等参数)*/
	memcpy(&tmwk, localtime((time_t *)&NowTime.tv_sec), sizeof(tmwk));

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	/* ドアを検索する（NPC_DoorSearchByName()を使えたら楽だけど）  (搜索门（能用NPC_DoorSearchByName()就好了）)
	 */
	for (i = CHAR_getPlayerMaxNum() + CHAR_getPetMaxNum();
		 i < charnum;
		 i++) {
		if (CHAR_getInt(i, CHAR_WHICHTYPE) != CHAR_TYPEDOOR) {
			continue;
		}
		if (CHAR_getWorkInt(i, CHAR_WORKDOORPASSFLG) == 0) {
			continue;
		}
		/*   部屋の前が部屋のものかチェックする  (检查房间前是否属于该房间) */
		for (j = 3;; j++) {
			int rc;
			char *doorname;
			char buf[256];
			int dataexist;
			NPC_ROOMINFO data;

			rc = getStringFromIndexWithDelim(argstr, "|", j,
											 buf, sizeof(buf));
			if (rc != TRUE) {
				break;
			}
			doorname = CHAR_getWorkChar(i, CHAR_WORKDOORNAME);
			/* ドアデータが一致した  (门数据一致) */
			if (strcmp(buf, doorname) != 0) {
				continue;
			}
			/*     データが一致する  (数据一致) */
			if (atoi(CHAR_getWorkChar(i, CHAR_WORKDOORWEEK)) != tmwk.tm_wday) {
				break;
			}
			/* ここで初期化  (在此初始化) */
			memset(&data, 0, sizeof(data));
			/* データファイルからデータを読む  (从数据文件读取数据) */
			if (NPC_RoomAdminNew_ReadFile(doorname, &data)) {
				struct tm tmdoor;
				/* ここで期限切れチェックをやってやる
				 * 期限が切れていたらデータを削除する
				 * 削除したらこの部屋の情報も更新する［
				 */
				if (data.expire < NowTime.tv_sec) {
					NPC_RoomAdminNew_WriteDataFile(meindex, -1, doorname, NULL, 0);
					/* ドアもきれいに  (门也一并整理)*/
					NPC_DoorSetPasswd(i, "8hda8iauia90494jasd9asodfasdf89");
					CHAR_setWorkInt(i, CHAR_WORKDOOREXPIRETIME, 0xffffffff);

					break;
				}
				/* ドアもきれいに閉じる  (门也一并关闭)     */
				memcpy(&tmdoor, localtime((time_t *)&data.expire), sizeof(tmdoor));
				if (tmdoor.tm_mon != tmwk.tm_mon ||
					tmdoor.tm_mday != tmwk.tm_mday) {
					break;
				}
				dataexist = TRUE;
			} else {
				dataexist = FALSE;
			}
			/* 現在の時間より先である場合  (晚于当前时间时)［ */
			if (tmwk.tm_hour > atoi(CHAR_getWorkChar(i, CHAR_WORKDOORHOUR))) {
				break;
			}
			if (tmwk.tm_hour == atoi(CHAR_getWorkChar(i, CHAR_WORKDOORHOUR))) {
				if (tmwk.tm_min >= atoi(CHAR_getWorkChar(i, CHAR_WORKDOORMINUTE))) {
					break;
				}
			}
			/* ここまで来たらとりあえずこのドアを指定時間の候補とする
			 * そして、その時のドアをチェックする［
			 */
			if (hitindex == -1) {
				hitindex = i;
			} else {
				/* 時間が経つまで現在開かれているのが優先順位  (时间未到则当前开放者优先) */
				if (atoi(CHAR_getWorkChar(hitindex, CHAR_WORKDOORHOUR)) > atoi(CHAR_getWorkChar(i, CHAR_WORKDOORHOUR))) {
					hitindex = i;
				} else if (atoi(CHAR_getWorkChar(hitindex, CHAR_WORKDOORHOUR)) == atoi(CHAR_getWorkChar(i, CHAR_WORKDOORHOUR))) {
					if (atoi(CHAR_getWorkChar(hitindex, CHAR_WORKDOORMINUTE)) > atoi(CHAR_getWorkChar(i, CHAR_WORKDOORMINUTE))) {
						hitindex = i;
					} else if (atoi(CHAR_getWorkChar(hitindex, CHAR_WORKDOORMINUTE)) == atoi(CHAR_getWorkChar(i, CHAR_WORKDOORMINUTE))) {
						NPC_ROOMINFO datawk;
						int hitindexdataexist;
						hitindexdataexist = NPC_RoomAdminNew_ReadFile(
							CHAR_getWorkChar(hitindex, CHAR_WORKDOORNAME),
							&datawk);
						if (hitindexdataexist < dataexist) {
							hitindex = i;
						}
						/* ここまで来るようなら先のindexのままとする  (走到这一步则保持先前的index)［*/
					}
				}
			}
		}
	}
	if (hitindex > 0) {
		/* 時間をセット  (设置时间) */
		tmwk.tm_sec = 0;
		tmwk.tm_min = atoi(CHAR_getWorkChar(hitindex, CHAR_WORKDOORMINUTE));
		tmwk.tm_hour = atoi(CHAR_getWorkChar(hitindex, CHAR_WORKDOORHOUR));
		CHAR_setWorkInt(meindex, CHAR_WORKINT_NEXTTIME, mktime(&tmwk));
		/*   部屋番号をセット  (设置房间号) */
		CHAR_setWorkChar(meindex, CHAR_WORKCHAR_NEXTROOM,
						 CHAR_getWorkChar(hitindex, CHAR_WORKDOORNAME));

	} else {
		CHAR_setWorkInt(meindex, CHAR_WORKINT_NEXTTIME, 0);
		/*   部屋番号をセット  (设置房间号) */
		CHAR_setWorkChar(meindex, CHAR_WORKCHAR_NEXTROOM, "");
	}
	/* 検索時間をセット  (设置检索时间) */
	CHAR_setWorkInt(meindex, CHAR_WORKINT_SEARCHROOMTIME, NowTime.tv_sec);
	return (TRUE);
}
/*-----------------------------------------------
 *   しかけられた部屋が鍵を閉めていたらパスワードを表示する  (所在房间上锁时显示密码)
 *----------------------------------------------*/
static void NPC_RoomAdminNew_DspRoomPasswd(int index, int adminindex, int talkerindex) {
	int i, rc, flg = FALSE;
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char argstr2[NPC_UTIL_GETARGSTR_BUFSIZE];
	char *cdkey;
	char buf[128];
	char buff[16];
	char buf2[1024];

	NPC_ROOMINFO info;

	// getcdkeyFromCharaIndex( talkerindex, cdkey, sizeof(cdkey));
	cdkey = CHAR_getChar(talkerindex, CHAR_CDKEY);

	NPC_Util_GetArgStr(adminindex, argstr, sizeof(argstr));
	for (i = 3;; i++) {
		if (getStringFromIndexWithDelim(argstr, "|", i, buf, sizeof(buf))) {
			rc = NPC_RoomAdminNew_ReadFile(buf, &info);
			if (rc == TRUE) {
				/* CDKEY CDKEYとキャラの部屋が一致したら  (CDKEY与角色房间一致时) */
				if (strcmp(info.cdkey, cdkey) == 0 &&
					strcmp(info.charaname,
						   CHAR_getChar(talkerindex, CHAR_NAME)) == 0) {
					int doorindex;
					char msgbuf[256];
					/*   部屋データ取得  (获取房间数据)   */
					doorindex = NPC_DoorSearchByName(buf);
					NPC_Util_GetArgStr(doorindex, argstr2, sizeof(argstr2));
					getStringFromIndexWithDelim(argstr2, "|", 9,
												buf2, sizeof(buf2));
					/*   部屋データ  (房间数据)  */
					if (strlen(CHAR_getChar(doorindex, CHAR_NAME)) != 0) {
						strcpy(buff, "号室");
					} else {
						buff[0] = '\0';
					}

					/* メッセージセット  (设置消息) */
					snprintf(msgbuf, sizeof(msgbuf),
							 NPC_ROOMADMINNEW_MSG_DSPPASSWD1,
							 buf2,
							 CHAR_getChar(doorindex, CHAR_NAME),
							 buff);
					/* メッセージ送信  (发送消息) */
					CHAR_talkToCli(talkerindex, index, msgbuf,
								   NPC_ROOMADMINNEW_MSGCOLOR);

					/* メッセージセット  (设置消息) */
					snprintf(msgbuf, sizeof(msgbuf),
							 NPC_ROOMADMINNEW_MSG_DSPPASSWD2,
							 CHAR_getWorkChar(doorindex, CHAR_WORKDOORPASSWD));
					/* メッセージ送信  (发送消息) */
					CHAR_talkToCli(talkerindex, index, msgbuf,
								   NPC_ROOMADMINNEW_MSGCOLOR);
					flg = TRUE;
				}
			}
		} else {
			break;
		}
	}
	if (!flg) {
		CHAR_talkToCli(talkerindex, index,
					   NPC_ROOMADMINNEW_MSG_PRINTPASSWD_NONE,
					   NPC_ROOMADMINNEW_MSGCOLOR);
	}
}
/*-----------------------------------------------
 * 指定の時間で鍵を替える  (在指定时间更换钥匙)
 *----------------------------------------------*/
static void NPC_RoomAdminNew_DspNextAuction(int meindex, int talkerindex) {
	char msgbuf[2048];
	time_t closetime;

	closetime = CHAR_getWorkInt(meindex, CHAR_WORKINT_NEXTTIME);
	/* 指定の時間が決まっていなければ「決まっていません」メッセージ  (未指定时间时发送“未指定”消息)*/
	if (closetime == 0) {
		snprintf(msgbuf, sizeof(msgbuf),
				 NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION_NONE);
		CHAR_talkToCli(talkerindex, meindex, msgbuf, NPC_ROOMADMINNEW_MSGCOLOR);
	} else {
		int doorindex;
		char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
		char explain[1024]; /*    */
		char buff[16];
		struct tm tmwk;
		int difftime;
		/*   部屋データを取得  (获取房间数据)   */
		doorindex = NPC_DoorSearchByName(
			CHAR_getWorkChar(meindex, CHAR_WORKCHAR_NEXTROOM));
		NPC_Util_GetArgStr(doorindex, argstr, sizeof(argstr));
		getStringFromIndexWithDelim(argstr, "|", 9, explain, sizeof(explain));
		/*   部屋データ  (房间数据)  */
		if (strlen(CHAR_getChar(doorindex, CHAR_NAME)) != 0) {
			strcpy(buff, "号室");
		} else {
			buff[0] = '\0';
		}
		snprintf(msgbuf, sizeof(msgbuf),
				 NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION1,
				 explain,
				 CHAR_getChar(doorindex, CHAR_NAME),
				 buff);
		CHAR_talkToCli(talkerindex, meindex, msgbuf, NPC_ROOMADMINNEW_MSGCOLOR);
		/* 締め切り時間  (截止时间) */
		memcpy(&tmwk, localtime(&closetime), sizeof(tmwk));
		difftime = closetime - NowTime.tv_sec;
		snprintf(msgbuf, sizeof(msgbuf),
				 NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION2,
				 tmwk.tm_hour,
				 tmwk.tm_min,
				 difftime / 3600,
				 (difftime % 3600) / 60);
		CHAR_talkToCli(talkerindex, meindex, msgbuf, NPC_ROOMADMINNEW_MSGCOLOR);

		/* 下限金額  (下限金额) */
		snprintf(msgbuf, sizeof(msgbuf),
				 NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION3,
				 CHAR_getWorkChar(doorindex, CHAR_WORKDOORGOLDLIMIT));
		CHAR_talkToCli(talkerindex, meindex, msgbuf, NPC_ROOMADMINNEW_MSGCOLOR);
	}
}
/*-----------------------------------------------
 *   入札者ランキングチェック（ファイル書き込み）
 *
 * ファイルは"管理者的名字.score"、
 * 一時ファイルは"管理者的名字.score.tmp"
 *
 * ファイルのフォーマットは以下
 *
 * 1件見つける
 *
 * 0：保存、1：ランキング上限超過
 *           -1：削除  “-1
 *----------------------------------------------*/
static int NPC_RoomAdminNew_WriteScoreFile(int meindex, int pindex) {
#define NPC_RANKING_DEFAULT 10
	int ret = 0;
	char *cdkey;
	char charaname[32 * 2], owntitle[32 * 2];
	char filename_dest[128], filename[128];
	char outbuf[1024], line[1024];
	FILE *fp_src, *fp_dest;

	/* ファイルを検索  (搜索文件) */
	strcpy(filename, NPC_ROOMADMINNEW_DATADIR);
	strcat(filename, CHAR_getWorkChar(meindex, CHAR_WORKCHAR_MYNAME));
	strcpy(filename_dest, filename);
	strcat(filename, NPC_ROOMADMINNEW_RANKING_EXTENSION);
	strcat(filename_dest, NPC_ROOMADMINNEW_RANKING_EXTENSION_TMP);

	/*   検索したデータのキー  (搜索到的数据键) */
	// getcdkeyFromCharaIndex( pindex , cdkey , sizeof(cdkey ));
	cdkey = CHAR_getChar(pindex, CHAR_CDKEY);

	/*   検索したデータのキャラ  (搜索到的数据角色)   */
	makeEscapeString(CHAR_getChar(pindex, CHAR_NAME),
					 charaname, sizeof(charaname));

	/*   検索したデータの番号  (搜索到的数据编号) */
	makeEscapeString(CHAR_getChar(pindex, CHAR_OWNTITLE),
					 owntitle, sizeof(owntitle));

	/* デリミタで括る  (用分隔符括起) */
	snprintf(outbuf, sizeof(outbuf), "%d|%d|%s|%s|%s|\n",
			 CHAR_getInt(pindex, CHAR_GOLD),
			 CHAR_getWorkInt(meindex, CHAR_WORKINT_NEXTTIME),
			 cdkey, charaname, owntitle);

	fp_dest = fopen(filename_dest, "w");
	if (!fp_dest)
		return -1;

	fp_src = fopen(filename, "r");
	if (!fp_src) {
		/*   最後の一件なのでそのまま書いて終わり  (最后一条数据直接写入结束) */
		fclose(fp_dest);
		fp_src = fopen(filename, "w");
		if (!fp_src)
			return -1;
		fputs(outbuf, fp_src);
		fclose(fp_src);
		return 1;
	} else {
		int cnt = 1;
		char buf[128];
		while (fgets(line, sizeof(line), fp_src)) {
			getStringFromIndexWithDelim(line, "|", 1, buf, sizeof(buf));
			if (ret == 0) {
				/*   入札金額が高い場合  (出价金额更高时) */
				if (atoi(buf) < CHAR_getInt(pindex, CHAR_GOLD)) {
					/* 金額が登録されているので挿入  (金额已登记故插入)   */
					fputs(outbuf, fp_dest);
					fputs(line, fp_dest);
					ret = cnt;
					cnt++;
				} else {
					/* それ以降はそのまま書く  (之后直接写入) */
					fputs(line, fp_dest);
				}
			} else {
				/* それ以降はそのまま書く  (之后直接写入) */
				fputs(line, fp_dest);
			}
			cnt++;
			if (cnt > NPC_RANKING_DEFAULT)
				break;
		}
		/* ランキングが1個そろっていない値  (排行榜不足1条时) */
		if (ret == 0 && cnt <= NPC_RANKING_DEFAULT) {
			fputs(outbuf, fp_dest);
			ret = cnt;
		}
	}
	fclose(fp_src);
	fclose(fp_dest);
	/*   入れ替え  (替换) */
	/* 誰かファイルコピーする簡単な方法知ってたら教えて下さい  (有谁有简单的文件复制方法请告诉我)［ */
	fp_src = fopen(filename_dest, "r");
	if (!fp_src)
		return -1;
	fp_dest = fopen(filename, "w");
	if (!fp_dest) {
		fclose(fp_src);
		return -1;
	}
	while (fgets(line, sizeof(line), fp_src)) {
		fputs(line, fp_dest);
	}
	fclose(fp_dest);
	fclose(fp_src);
	return (ret);
#undef NPC_RANKING_DEFAULT
}
/*-----------------------------------------------
 *   入札者ランキングファイル読込用
 *
 * 引数 meindex     キャラクター番号（ランキングNPCの番号）
 *      talkerindex   しかけられたデータのindex（それにtalkを送信する）
 *      admname     オークション管理人の名前
 *
 *----------------------------------------------*/
static void NPC_RoomAdminNew_ReadScoreFile(int meindex, int talkerindex,
										   char *admname) {
	int cnt;
	char filename[128];
	char line[1024];
	FILE *fp;

	/* ファイルを検索  (搜索文件) */
	strcpy(filename, NPC_ROOMADMINNEW_DATADIR);
	strcat(filename, admname);
	strcat(filename, NPC_ROOMADMINNEW_RANKING_EXTENSION);

	fp = fopen(filename, "r");
	if (!fp)
		return;
	cnt = 1;
	while (fgets(line, sizeof(line), fp)) {
		int gold, bidtime;
		char charaname[32], owntitle[32];
		char tmp[64];
		char outbuf[1024], buf[256];
		struct tm tmwk;

		getStringFromIndexWithDelim(line, "|", 1, buf, sizeof(buf));
		gold = atoi(buf);
		getStringFromIndexWithDelim(line, "|", 2, buf, sizeof(buf));
		bidtime = atoi(buf);
		getStringFromIndexWithDelim(line, "|", 4, buf, sizeof(buf));
		strcpy(charaname, makeStringFromEscaped(buf));
		getStringFromIndexWithDelim(line, "|", 5, buf, sizeof(buf));
		strcpy(owntitle, makeStringFromEscaped(buf));
		if (strlen(owntitle) != 0) {
			strcpy(tmp, "[");
			strcat(tmp, owntitle);
			strcat(tmp, "] ");
			strcpy(owntitle, tmp);
		} else {
			strcpy(owntitle, " ");
		}
		memcpy(&tmwk, localtime((time_t *)&bidtime), sizeof(tmwk));

		snprintf(outbuf, sizeof(outbuf),
				 NPC_ROOMADMINNEW_MSG_RANKING,
				 cnt,
				 tmwk.tm_year + 1900,
				 tmwk.tm_mon + 1,
				 tmwk.tm_mday,
				 gold, charaname, owntitle);
		CHAR_talkToCli(talkerindex, meindex, outbuf,
					   NPC_RANKING_MSGCOLOR);
		cnt++;
	}
	fclose(fp);
}
/************************************************
 * 初期化  (初始化)
 ************************************************/
BOOL NPC_RankingInit(int meindex) {
	char buf[256];
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	/* 初期化  (初始化)   */

	/* typeセット  (设置type) */
	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPERANKING);
	/* 部屋の前を取得  (获取房间前)   */
	if (!getStringFromIndexWithDelim(argstr, "|", 1, buf, sizeof(buf))) {
		print("arg err.rankingNPC not created.\n");
		return (FALSE);
	}
	CHAR_setWorkChar(meindex, CHAR_WORKCHAR_ADMNAME, buf);

	return TRUE;
}
/************************************************
 *   しかけられた時の値  (被设置时的值)
 ************************************************/
void NPC_RankingTalked(int meindex, int talkerindex, char *msg,
					   int color) {
	/* プレイヤーがドアマンの1グリッド動いたなら実行する  (玩家距门卫1格时执行) */
	if (NPC_Util_CharDistance(talkerindex, meindex) > 1)
		return;

	CHAR_talkToCli(talkerindex, meindex,
				   NPC_ROOMADMINNEW_MSG_RANKING_INFO,
				   NPC_RANKING_MSGCOLOR);

	NPC_RoomAdminNew_ReadScoreFile(meindex, talkerindex,
								   CHAR_getWorkChar(meindex, CHAR_WORKCHAR_ADMNAME));
}
/************************************************
 * 初期化  (初始化)
 ************************************************/
BOOL NPC_PrintpassmanInit(int meindex) {
	char buf[256];
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

	NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));

	/* 初期化  (初始化)   */

	/* typeセット  (设置type) */
	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPEPRINTPASSMAN);
	/* 部屋の前を取得  (获取房间前)   */
	if (!getStringFromIndexWithDelim(argstr, "|", 1, buf, sizeof(buf))) {
		print("arg err.passmanNPC not created.\n");
		return (FALSE);
	}
	CHAR_setWorkChar(meindex, CHAR_WORKCHAR_ADMNAME, buf);

	return TRUE;
}
/************************************************
 *   しかけられた時の値  (被设置时的值)
 ************************************************/
void NPC_PrintpassmanTalked(int meindex, int talkerindex, char *msg,
							int color) {
	int i;
	int charnum;
	/* プレイヤーがドアマンの1グリッド動いたなら実行する  (玩家距门卫1格时执行) */
	if (NPC_Util_CharDistance(talkerindex, meindex) > 2)
		return;

	/* 部屋用の部屋の前を検索する  (搜索房间专用的门前)*/
	charnum = CHAR_getCharNum();
	for (i = CHAR_getPlayerMaxNum() + CHAR_getPetMaxNum();
		 i < charnum;
		 i++) {

		if (CHAR_getCharUse(i) &&
			CHAR_getInt(i, CHAR_WHICHTYPE) == CHAR_TYPEROOMADMIN) {
			if (strcmp(CHAR_getWorkChar(i, CHAR_WORKCHAR_MYNAME),
					   CHAR_getWorkChar(meindex, CHAR_WORKCHAR_ADMNAME)) == 0) {
				break;
			}
		}
	}
	if (i < charnum) {
		NPC_RoomAdminNew_DspRoomPasswd(meindex, i, talkerindex);
	} else {
		CHAR_talkToCli(talkerindex, meindex, "。。。", NPC_RANKING_MSGCOLOR);
	}
}
/* end of roomadminnew */
