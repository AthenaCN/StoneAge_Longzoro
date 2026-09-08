#include "version.h"
#define _NPC_MSG_C_

#include "char.h"
#include "char_base.h"
#include "npcutil.h"

/*
  典型的な話は何でもよい  (典型的话什么都可以)
  とりあえずどの話題からでも話せるようにする  (总之让任何话题都能聊)

  init, looked のみに実行  (仅对 init、looked 执行)

  */
BOOL NPC_MsgInit(int meindex) {
	// CHAR_setInt( meindex , CHAR_HP , 0 );

	// CHAR_setInt( meindex , CHAR_MP , 0 );
	// CHAR_setInt( meindex , CHAR_MAXMP , 0 );

	// CHAR_setInt( meindex , CHAR_STR , 0 );
	// CHAR_setInt( meindex , CHAR_TOUGH, 0 );

	// CHAR_setInt( meindex , CHAR_LV , 0 );

	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPETOWNPEOPLE);
	// CHAR_setFlg( meindex , CHAR_ISOVERED , 0 );
	// CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );  /*   撃されないように   (使其不被攻击) */

	return TRUE;
}

/*
 * 話られたら話すだけ  (被搭话就只是回应)
 */
void NPC_MsgLooked(int meindex, int lookedindex) {
	char arg[NPC_UTIL_GETARGSTR_BUFSIZE];

	NPC_Util_GetArgStr(meindex, arg, sizeof(arg));
	CHAR_talkToCli(lookedindex, -1, arg, CHAR_COLORWHITE);
}
