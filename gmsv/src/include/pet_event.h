#ifndef __PET_EVENT_H__
#define __PET_EVENT_H__

/*
 * ペット関連の修正です  (宠物相关修正)
 * ペットイベント関連  (宠物事件相关)
 */

void PET_Talkfunc(int meindex, int talkerindex, char *msg, int color);

void PET_Watchfunc(int objmeindex, int objmoveindex,
				   CHAR_ACTION act, int x, int y, int dir,
				   int *opt, int optlen);
int PET_CleanPetdeletetime(int objmeindex);
void PET_CleanFreePetAll();

#endif
