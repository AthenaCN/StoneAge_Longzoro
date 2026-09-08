#ifndef __READNPCTEMPLATE_H__
#define __READNPCTEMPLATE_H__

#include "util.h"
#include "char_base.h"

#define NPC_TEMPLATEFILEMAGIC "NPCTEMPLATE\n"

#undef EXTERN
#ifdef __NPCTEMPLATE__
#define EXTERN
#else
#define EXTERN extern
#endif /*__NPCTEMPLATE__*/

typedef struct tagNPC_haveItem {
	int itemnumber;
	int haverate;
	int havenum;
} NPC_haveItem;

typedef enum {
	NPC_TEMPLATENAME,	  /*    テンプレートの名前  (模板的名称)  */
	NPC_TEMPLATECHARNAME, /*    名前  (名称)    */

	NPC_TEMPLATEINITFUNC,	  /*  CHAR_INITFUNCに付く  (附加到CHAR_INITFUNC) */
	NPC_TEMPLATEWALKPREFUNC,  /*  CHAR_WALKPREFUNC    */
	NPC_TEMPLATEWALKPOSTFUNC, /*  CHAR_WALKPOSTFUNC   */
	NPC_TEMPLATEPREOVERFUNC,  /*  CHAR_PREOVERFUNC    */
	NPC_TEMPLATEPOSTOVERFUNC, /*  CHAR_POSTOVERFUNC   */
	NPC_TEMPLATEWATCHFUNC,	  /*  CHAR_WATCHFUNC      */
	NPC_TEMPLATELOOPFUNC,	  /*  CHAR_LOOPFUNC   */
	NPC_TEMPLATEDYINGFUNC,	  /*  CHAR_DYINGFUNC  */
	NPC_TEMPLATETALKEDFUNC,	  /*  CHAR_TALKEDFUNC */

	NPC_TEMPLATEPREATTACKEDFUNC,  /*  CHAR_PREATTACKEDFUNC    */
	NPC_TEMPLATEPOSTATTACKEDFUNC, /*  CHAR_POSTATTACKEDFUNC    */

	NPC_TEMPLATEOFFFUNC,	 /*  CHAR_OFFFUNC    */
	NPC_TEMPLATELOOKEDFUNC,	 /*  CHAR_LOOKEDFUNC  */
	NPC_TEMPLATEITEMPUTFUNC, /*  CHAR_ITEMPUTFUNC    */

	NPC_TEMPLATESPECIALTALKEDFUNC, /*  CHAR_SPECIALTALKEDFUNC   */
	NPC_TEMPLATEWINDOWTALKEDFUNC,  /*  CHAR_WINDOWTALKEDFUNC   */
#ifdef _USER_CHARLOOPS
	NPC_TEMPLATELOOPFUNCTEMP1,	// CHAR_LOOPFUNCTEMP1,
	NPC_TEMPLATELOOPFUNCTEMP2,	// CHAR_LOOPFUNCTEMP2,
	NPC_TEMPLATEBATTLEPROPERTY, // CHAR_BATTLEPROPERTY,
#endif
	NPC_TEMPLATECHARNUM,
} NPC_TEMPLATECHAR;

typedef enum {
	NPC_TEMPLATEMAKEATNOBODY, /* 誰もいない時にも作るかどうか  (无人在场时也创建吗) */
	NPC_TEMPLATEMAKEATNOSEE,  /* 見えない所で作るかどうか  (在不可见处创建吗)    */
	NPC_TEMPLATEIMAGENUMBER,  /*   画像番号  (图像编号)    */
	NPC_TEMPLATETYPE,		  /*     呼び出される時に呼び出す  (被调用时调用)        */

	NPC_TEMPLATEMINHP, /* HP   */

	NPC_TEMPLATEMINMP, /* MP   */

	NPC_TEMPLATEMINSTR, /* STR  */

	NPC_TEMPLATEMINTOUGH, /* TOUGH    */

	NPC_TEMPLATEISFLYING, /*  飛んでるかどうか  (是否飞行中)  */

	NPC_TEMPLATEITEMNUM, /*   落ちうるアイテムの数  (可能掉落的物品数量)    */

	NPC_TEMPLATELOOPFUNCTIME,  /*
								* 何ミリ秒ごとにループ関数  (每隔多少毫秒的循环函数)
								* を呼ぶか  (调用)
								*/
	NPC_TEMPLATEFUNCTIONINDEX, /*
								* fucntionSet の何の  (fucntionSet 的哪个)
								* インデックスか  (索引)
								*/

	NPC_TEMPLATEINTNUM,
} NPC_TEMPLATEINT;

typedef struct tagNPC_Template {
	STRING64 chardata[NPC_TEMPLATECHARNUM];
	int intdata[NPC_TEMPLATEINTNUM];
	int randomdata[NPC_TEMPLATEINTNUM]; /*  ランダムの大きさ  (随机大小)
											が  呼び出している  (正在调用)  */
	int hash;
	NPC_haveItem *haveitem;
} NPC_Template;

EXTERN NPC_Template *NPC_template;
EXTERN int NPC_templatenum;
EXTERN int NPC_template_readindex;

INLINE int NPC_CHECKTEMPLATEINDEX(int index);

BOOL NPC_copyFunctionSetToChar(int id, Char *ch);

BOOL NPC_readNPCTemplateFiles(char *topdirectory, int templatesize);
int NPC_templateGetTemplateIndex(char *templatename);

#endif
/*__READNPCTEMPLATE_H__*/
