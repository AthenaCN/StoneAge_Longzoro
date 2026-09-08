#ifndef __NPC_ROOMADMINNEW_H__
#define __NPC_ROOMADMINNEW_H__

void NPC_RoomAdminNewTalked(int meindex, int talkerindex, char *msg, int color);
void NPC_RoomAdminNewLoop(int meindex);
BOOL NPC_RoomAdminNewInit(int meindex);

BOOL NPC_RankingInit(int meindex);
void NPC_RankingTalked(int meindex, int talkerindex, char *msg, int color);

BOOL NPC_PrintpassmanInit(int meindex);
void NPC_PrintpassmanTalked(int meindex, int talkerindex, char *msg, int color);

#if 0
typedef struct roomadmin_tag
{
    int index;              /*   屋のindex  (房间的index) */
    
    char doorname[256];     /* その屋のドアの前  (该房间门前) */
    char explanation[256];  /*      */
    char passwd[256];       /* パスワード  (密码) */
    int expire_time_mod;    /* 期限  (期限) */
    time_t expire_time;     /* 期限切れの時間  (到期时间) */
    int least_cost;         /*   最低費用  (最低费用) */
    
}NPC_ROOMINFO;
#endif

typedef struct npc_roomadminnew_tag {
	int expire;
	char cdkey[CDKEYLEN];
	char charaname[32];
	char passwd[9];
} NPC_ROOMINFO;

typedef struct npc_roomadminnew_ranking_tag {
	int gold;			  /*   金額  (金额) */
	int biddate;		  /*   入札時間  (出价时间) */
	char cdkey[CDKEYLEN]; /*   とした場合の    CDKEY  (时的CDKEY) */
	char charaname[32];	  /*   とした場合の名前  (时的名字) */
	char owntitle[32];	  /* 所有タイトル  (拥有称号)*/

} NPC_RANKING_INFO;

#endif /*__NPC_ROOMADMINNEW_H__*/

BOOL NPC_RoomAdminNew_ReadFile(char *roomname, NPC_ROOMINFO *data);
