#ifndef __PETMAIL_H__
#define __PETMAIL_H__
#include "version.h"
/*
 * ペット関連の修正です  (宠物相关修正)
 */
#define PETMAIL_OFFMSG_MAX 10000
#define PETMAIL_OFFMSG_TIMEOUT (3 * 24 * 3600)
#define PETMAIL_CHECK_OFFMSG_EXPIRE_INTERVAL 3600
#define PETMAIL_OFFMSG_TEXTLEN 512

/* ペットメールの出現効率の数、際の数より減らなくすること  (宠物邮件出现效率数，使其不低于此前数量) */
#define PETMAIL_EFFECTMAX 1

#define PETMAIL_SPOOLFLOOR 777
#define PETMAIL_SPOOLX 30
#define PETMAIL_SPOOLY 30

#define PETMAIL_LOOPINTERVAL1 500
#define PETMAIL_LOOPINTERVAL2 5000

/* オフラインのキャラへのメッセージを作る  (创建发给离线角色的消息) */
typedef struct
{
	int use;
	time_t send_tm;					   /* ユーザーがメッセージを送信した時間+TIMEOUT  (用户发送消息的时间+TIMEOUT) */
	int color;						   /*   テキストの色  (文本颜色) */
	char text[PETMAIL_OFFMSG_TEXTLEN]; /*   テキストの     (文本的) */
	char destcd[CDKEYLEN];			   /* 送信先CDKEY  (发送目标CDKEY) */
	char destcharname[CHARNAMELEN];	   /* 送信先のキャラ   (发送目标角色) */
	char srccd[CDKEYLEN];			   /* 送信元のcdkey  (发送来源cdkey) */
	char srccharname[CHARNAMELEN];	   /* 送信元のキャラ   (发送来源角色) */

} PETMAIL_offmsg;

void PETMAIL_Loopfunc(int index);
BOOL PETMAIL_CheckPlayerExist(int index, int mode);

BOOL PETMAIL_initOffmsgBuffer(int count);
BOOL PETMAIL_addOffmsg(int fromindex, char *tocdkey, char *tocharaname,
					   char *text, int color);
PETMAIL_offmsg *PETMAIL_getOffmsg(int offmsgindex);
BOOL PETMAIL_deleteOffmsg(int offmsgindex);
void PETMAIL_proc(void);
BOOL storePetmail(void);
BOOL PETMAIL_sendPetMail(int cindex, int aindex,
						 int havepetindex, int haveitemindex, char *text, int color);

#ifdef _PETMAIL_DEFNUMS
void CHAR_AutoPickupMailPet(int charaindex, int petindex);
#endif
int PETMAIL_getPetMailTotalnums(void);
void PETMAIL_delPetMailTotalnums(int numflg);
void PETMAIL_setPetMailTotalnums(int numflg);
int PETMAIL_CheckIsMyOffmsg(int fromindex, char *tocdkey, char *tocharaname);

#endif
