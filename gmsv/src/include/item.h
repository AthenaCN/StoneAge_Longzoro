
#ifndef __ITEM_H__
#define __ITEM_H__

#include "char.h"

#define NULLITEM "0"

typedef enum {
	ITEM_FIST = 0,
	ITEM_AXE,
	ITEM_CLUB,
	ITEM_SPEAR,
	ITEM_BOW,
	ITEM_SHIELD,
	ITEM_HELM,
	ITEM_ARMOUR,

	ITEM_BRACELET = 8,
	ITEM_MUSIC,
	ITEM_NECKLACE,
	ITEM_RING,
	ITEM_BELT,
	ITEM_EARRING,
	ITEM_NOSERING,
	ITEM_AMULET,
	/* ****** */
	ITEM_OTHER = 16,
	ITEM_BOOMERANG,	 // 回旋标
	ITEM_BOUNDTHROW, // 投掷斧头
	ITEM_BREAKTHROW, // 投掷石
	ITEM_DISH = 20,
#ifdef _ITEM_INSLAY
	ITEM_METAL,
	ITEM_JEWEL,
#endif
#ifdef _ITEM_CHECKWARES
	ITEM_WARES, // 货物
#endif

#ifdef _ITEM_EQUITSPACE
	ITEM_WBELT,	  // 腰带
	ITEM_WSHIELD, // 盾
	ITEM_WSHOES,  // 鞋子
#endif
#ifdef _EQUIT_NEWGLOVE
	ITEM_WGLOVE, // 手套
#endif

#ifdef _ALCHEMIST
	ITEM_ALCHEMIST = 30,
#endif

#ifdef _ANGEL_SUMMON
// ITEM_ANGELTOKEN,
// ITEM_HEROTOKEN,
#endif

	ITEM_CATEGORYNUM,

} ITEM_CATEGORY;

typedef enum {
	ITEM_FIELD_ALL,
	ITEM_FIELD_BATTLE,
	ITEM_FIELD_MAP,
} ITEM_FIELDTYPE;

typedef enum {
	ITEM_TARGET_MYSELF,
	ITEM_TARGET_OTHER,
	ITEM_TARGET_ALLMYSIDE,
	ITEM_TARGET_ALLOTHERSIDE,
	ITEM_TARGET_ALL,
} ITEM_TARGETTYPE;

typedef enum {
	ITEM_ID,
	ITEM_BASEIMAGENUMBER,
	ITEM_COST,
	ITEM_TYPE,
	ITEM_ABLEUSEFIELD,
	ITEM_TARGET,
	ITEM_LEVEL, /*  LEVEL  */
#ifdef _ITEM_MAXUSERNUM
	ITEM_DAMAGEBREAK, // 物品使用次数
#endif

#ifdef _ITEMSET4_TXT
	ITEM_USEPILENUMS, // 物品堆叠次数
	ITEM_CANBEPILE,	  // 是否可堆叠

	ITEM_NEEDSTR,
	ITEM_NEEDDEX,
	ITEM_NEEDTRANS,
	ITEM_NEEDPROFESSION,
#endif

#ifdef _TAKE_ITEMDAMAGE
	ITEM_DAMAGECRUSHE,
	ITEM_MAXDAMAGECRUSHE,
#endif

#ifdef _ADD_DEAMGEDEFC
	ITEM_OTHERDAMAGE,
	ITEM_OTHERDEFC,
#endif

#ifdef _SUIT_ITEM
	ITEM_SUITCODE,
#endif

	ITEM_ATTACKNUM_MIN, /*    最小  攻撃回数  (最小攻击次数)  */
	ITEM_ATTACKNUM_MAX, /*    最大  攻撃回数  (最大攻击次数)  */
	ITEM_MODIFYATTACK,	/*    攻撃    修正  (攻击修正)  */
	ITEM_MODIFYDEFENCE, /*    防御    修正  (防御修正)  */
	ITEM_MODIFYQUICK,	/*  QUICK  修正  (QUICK修正)  */

	ITEM_MODIFYHP,			/*  HP  祭汹    */
	ITEM_MODIFYMP,			/*  MP  祭汹    */
	ITEM_MODIFYLUCK,		/*  LUCK  祭汹    */
	ITEM_MODIFYCHARM,		/*  CHARM  祭汹    */
	ITEM_MODIFYAVOID,		/*  荚    膜恳    */
	ITEM_MODIFYATTRIB,		/*  箪岭膜恳 */
	ITEM_MODIFYATTRIBVALUE, /*  箪岭膜恳袄 */
	ITEM_MAGICID,			/*  热诸  寞 */
	ITEM_MAGICPROB,			/*  热诸       */
	ITEM_MAGICUSEMP,		/*  壅  MP */

#ifdef _ITEMSET5_TXT
	ITEM_MODIFYARRANGE,
	ITEM_MODIFYSEQUENCE,

	ITEM_ATTACHPILE,
	ITEM_HITRIGHT, // 额外命中
#endif
#ifdef _ITEMSET6_TXT
	ITEM_NEGLECTGUARD,
//	ITEM_BEMERGE,
#endif
	/*  ステータス修復用 (状态修复用) */ ITEM_POISON, /*   歩くたびにダメージ  (每走一步受伤害)          */
	ITEM_PARALYSIS,									  /* しびれ｝1の移動ができない  (麻痹，1格内无法移动)［ */
	ITEM_SLEEP,										  /* 眠り［移動できない  (睡眠，无法移动)            */
	ITEM_STONE,										  /* 石［移動できない  (石化，无法移动)              */
	ITEM_DRUNK,										  /* 酔う［命中が下がる  (醉酒，命中下降)     */
	ITEM_CONFUSION,									  /* 混乱［攻撃を繰り返す  (混乱，反复攻击)     */

	ITEM_CRITICAL, /* クリティカル修復用  (暴击修复用) */

	ITEM_USEACTION,	   /* 使用した時のアクション  (使用时的动作) */
	ITEM_DROPATLOGOUT, /* ログアウトする時に落とすかどうか  (登出时是否掉落)  */
	ITEM_VANISHATDROP, /* 落とした時に消えるかどうか  (掉落时是否消失) */
	ITEM_ISOVERED,	   /* 下に覆いかぶさられるかどうか  (是否被覆盖)［*/
	ITEM_CANPETMAIL,   /* ペットメールで送れるか  (能否用宠物邮件寄送) */
	ITEM_CANMERGEFROM, /* 合成元になれるか  (能否作合成材料) */
	ITEM_CANMERGETO,   /* 合成先になれるか  (能否作合成目标) */

	ITEM_INGVALUE0, /* 値(5個)  (值(5个)) */
	ITEM_INGVALUE1,
	ITEM_INGVALUE2,
	ITEM_INGVALUE3,
	ITEM_INGVALUE4,

	ITEM_PUTTIME,	 /*  アイテムが置かれた時間  (物品放置的时间) */
	ITEM_LEAKLEVEL,	 /* 密がどれだけばれたか  (秘密泄露程度)  */
	ITEM_MERGEFLG,	 /*  合成されたアイテムかどうか  (是否为合成物品) */
	ITEM_CRUSHLEVEL, /*  壊れにくさ 0..2 で壊れやすい 2で全壊  (易碎度) */

	ITEM_VAR1, /*    迕综仅       */
	ITEM_VAR2, /*    迕综仅       */
	ITEM_VAR3, /*    迕综仅       */
	ITEM_VAR4, /*    迕综仅       */

	ITEM_DATAINTNUM,

} ITEM_DATAINT;

typedef enum {
	ITEM_NAME,		   /*    名前   の  名前     (名称的名称) */
	ITEM_SECRETNAME,   /* 名前を変更した時の記録  (改名记录)    */
	ITEM_EFFECTSTRING, /*  状態データ  (状态数据)    */
	ITEM_ARGUMENT,	   /*  アイテムの引数  (物品参数)  */
#ifdef _ITEM_INSLAY
	ITEM_TYPECODE,
	ITEM_INLAYCODE,
#endif
	ITEM_CDKEY, /*  アイテムの名前を秘密に変更した時の  (物品秘密改名时)           */
#ifdef _ITEM_FORUSERNAMES
	ITEM_FORUSERNAME,
	ITEM_FORUSERCDKEY,
#endif
// CoolFish: 2001/10/11
#ifdef _UNIQUE_P_I
	ITEM_UNIQUECODE, /* 物品编码 */
#endif

	ITEM_INGNAME0, /*  値の  名前(5個)  (值的名称(5个)) */
	ITEM_INGNAME1,
	ITEM_INGNAME2,
	ITEM_INGNAME3,
	ITEM_INGNAME4,

	ITEM_INITFUNC, /* 戻り値  (返回值)
					* ITEM_Item*
					* 戻り値 BOOL  (返回值BOOL)
					* 戻り値の型は CHAR_INITFUNC  (返回类型为CHAR_INITFUNC)
					* と同じ  (与...相同)  */
	ITEM_FIRSTFUNCTION = ITEM_INITFUNC,
	ITEM_PREOVERFUNC,  /* CHAR_PREOVERFUNC を代替  (替代CHAR_PREOVERFUNC) */
	ITEM_POSTOVERFUNC, /* CHAR_POSTOVERFUNC を代替  (替代CHAR_POSTOVERFUNC) */
	ITEM_WATCHFUNC,	   /* CHAR_WATCHFUNC を代替  (替代CHAR_WATCHFUNC) */
	ITEM_USEFUNC,	   /* 戻り値は｝  (返回值)
						* int charaindex キャラインデックス  (角色索引)
						* int charitemindex 自分の  (自己的)
						*              アイテムの何  (物品的哪个位置)
						*              を呼び出したか  (是否被调用)
						*/
	ITEM_ATTACHFUNC,   /* 戻り値は｝  (返回值)
						* int charaindex キャラインデックス  (角色索引)
						* int itemindex  アイテムインデックス  (物品索引)
						*      キャラクタの持っているアイテム  (角色持有的物品)
						*      のアイテムでのインデックス  (物品栏索引)
						*      ではない場合に注意［  (非物品时注意)
						*/
	ITEM_DETACHFUNC,   /* 戻り値は｝  (返回值)
						* int charaindex キャラインデックス  (角色索引)
						* int itemindex  アイテムインデックス  (物品索引)
						*      キャラクタの持っているアイテム  (角色持有的物品)
						*      のアイテムでのインデックス  (物品栏索引)
						*      ではない場合に注意［  (非物品时注意)
						*/
	ITEM_DROPFUNC,	   /* 落としたとき  (掉落时)
						* 戻り値は  (返回值)
						*  int charaindex   落としたキャラ  (掉落物品的角色)
						*  int itemindex アイテムインデックス  (物品索引)
						*/
	ITEM_PICKUPFUNC,   /* アイテムを拾った時  (拾取物品时)
						* 戻り値は  (返回值)
						*  int charaindex  拾ったキャラindex  (拾取的角色索引)
						*  int itemindex アイテムインデックス  (物品索引)
						*/
#ifdef _Item_ReLifeAct
	ITEM_DIERELIFEFUNC, /*ANDY_ADD
						复活道具
					 */
#endif

#ifdef _CONTRACT
	ITEM_CONTRACTTIME,
	ITEM_CONTRACTARG,
#endif

	ITEM_LASTFUNCTION,

	ITEM_DATACHARNUM = ITEM_LASTFUNCTION,

#ifdef _ANGEL_SUMMON
	ITEM_ANGELMISSION = ITEM_INGNAME0,
	ITEM_ANGELINFO = ITEM_INGNAME1,
	ITEM_HEROINFO = ITEM_INGNAME2,
#endif

} ITEM_DATACHAR;

typedef enum {
	ITEM_WORKOBJINDEX,
	ITEM_WORKCHARAINDEX,
#ifdef _MARKET_TRADE
	ITEM_WORKTRADEINDEX,
	ITEM_WORKTRADETYPE,
	ITEM_WORKTRADESELLINDEX,
#endif
#ifdef _ITEM_ORNAMENTS
	ITEM_CANPICKUP,
#endif
#ifdef _ITEM_TIME_LIMIT
	ITEM_WORKTIMELIMIT,
#endif
	ITEM_WORKDATAINTNUM,
} ITEM_WORKDATAINT;

typedef struct tagItem {
	int data[ITEM_DATAINTNUM];
	STRING64 string[ITEM_DATACHARNUM];
	int workint[ITEM_WORKDATAINTNUM];

	void *functable[ITEM_LASTFUNCTION - ITEM_FIRSTFUNCTION];
} ITEM_Item;

typedef struct tagITEM_table {
	int use;
	ITEM_Item itm;
	int randomdata[ITEM_DATAINTNUM];
} ITEM_table;

typedef struct tagITEM_exists {
	BOOL use;
	ITEM_Item itm;
} ITEM_exists;

#ifdef _CONTRACT
#define MAX_CONTRACTTABLE 10
typedef struct tagITEM_contract {
	int used;
	char detail[2048];
	int argnum;
} ITEM_contractTable;
#endif

#define ITEM_CHECKINDEX(index) \
	_ITEM_CHECKINDEX(__FILE__, __LINE__, index)
INLINE BOOL _ITEM_CHECKINDEX(char *file, int line, int index);

BOOL ITEM_initExistItemsArray(int num);
BOOL ITEM_endExistItemsArray(void);
#define ITEM_initExistItemsOne(itm) \
	_ITEM_initExistItemsOne(__FILE__, __LINE__, itm)
int _ITEM_initExistItemsOne(char *file, int line, ITEM_Item *itm);

#define ITEM_endExistItemsOne(index) \
	_ITEM_endExistItemsOne(index, __FILE__, __LINE__)

void _ITEM_endExistItemsOne(int index, char *file, int line);

#define ITEM_getInt(Index, element) _ITEM_getInt(__FILE__, __LINE__, Index, element)
INLINE int _ITEM_getInt(char *file, int line, int index, ITEM_DATAINT element);

#define ITEM_setInt(Index, element, data) _ITEM_setInt(__FILE__, __LINE__, Index, element, data)
INLINE int _ITEM_setInt(char *file, int line, int index, ITEM_DATAINT element, int data);

INLINE char *ITEM_getChar(int index, ITEM_DATACHAR element);
INLINE BOOL ITEM_setChar(int index, ITEM_DATACHAR element, char *new);

INLINE int ITEM_getWorkInt(int index, ITEM_WORKDATAINT element);
INLINE int ITEM_setWorkInt(int index, ITEM_WORKDATAINT element, int data);
INLINE int ITEM_getITEM_itemnum(void);
INLINE int ITEM_getITEM_UseItemnum(void);
INLINE BOOL ITEM_getITEM_use(int index);
void ITEM_constructFunctable(int itemindex);
void *ITEM_getFunctionPointer(int itemindex, int functype);
INLINE ITEM_Item *ITEM_getItemPointer(int index);
int ITEM_getItemMaxIdNum(void);

char *ITEM_makeStringFromItemData(ITEM_Item *one, int mode);
char *ITEM_makeStringFromItemIndex(int index, int mode);

BOOL ITEM_makeExistItemsFromStringToArg(char *src, ITEM_Item *item, int mode);
void ITEM_getDefaultItemSetting(ITEM_Item *itm);

INLINE BOOL ITEM_CHECKITEMTABLE(int number);
BOOL ITEM_readItemConfFile(char *filename);

CHAR_EquipPlace ITEM_getEquipPlace(int charaindex, int itmid);

char *ITEM_makeItemStatusString(int haveitemindex, int itemindex);
char *ITEM_makeItemFalseString(void);
char *ITEM_makeItemFalseStringWithNum(int haveitemindex);

BOOL ITEM_makeItem(ITEM_Item *itm, int number);
int ITEM_makeItemAndRegist(int number);

void ITEM_equipEffect(int index);

void Other_DefcharWorkInt(int index);

char *ITEM_getAppropriateName(int itemindex);
char *ITEM_getEffectString(int itemindex);

int ITEM_getcostFromITEMtabl(int itemid);

#define ITEM_getNameFromNumber(id) _ITEM_getNameFromNumber(__FILE__, __LINE__, id)
INLINE char *_ITEM_getNameFromNumber(char *file, int line, int itemid);

int ITEM_getlevelFromITEMtabl(int itemid);
int ITEM_getgraNoFromITEMtabl(int itemid);
char *ITEM_getItemInfoFromNumber(int itemid);

int ITEM_getdropatlogoutFromITEMtabl(int itemid);
int ITEM_getvanishatdropFromITEMtabl(int itemid);
int ITEM_getcanpetmailFromITEMtabl(int itemid);
int ITEM_getmergeItemFromFromITEMtabl(int itemid);

#ifdef _ITEM_CHECKWARES
BOOL CHAR_CheckInItemForWares(int charaindex, int flg);
#endif

BOOL ITEM_canuseMagic(int itemindex);
// Nuke +1 08/23 : For checking the validity of item target
int ITEM_isTargetValid(int charaindex, int itemindex, int toindex);

#ifdef _IMPOROVE_ITEMTABLE
BOOL ITEMTBL_CHECKINDEX(int ItemID);
int ITEM_getSIndexFromTransList(int ItemID);
int ITEM_getMaxitemtblsFromTransList(void);
int ITEM_getTotalitemtblsFromTransList(void);
#endif

int ITEMTBL_getInt(int ItemID, ITEM_DATAINT datatype);
char *ITEMTBL_getChar(int ItemID, ITEM_DATACHAR datatype);

int ITEM_getItemDamageCrusheED(int itemindex);
void ITEM_RsetEquit(int charaindex); // 自动卸除装备位置错误之物品
void ITEM_reChangeItemToPile(int itemindex);
void ITEM_reChangeItemName(int itemindex);

#ifdef _SIMPLIFY_ITEMSTRING
void ITEM_getDefaultItemData(int itemID, ITEM_Item *itm);
#endif

#ifdef _CONTRACT
BOOL ITEM_initContractTable();
#endif

#endif
