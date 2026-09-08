#ifndef __OBJECT_h__
#define __OBJECT_h__

#include "common.h"

typedef enum {
	OBJTYPE_NOUSE, /*  このインデックスは使用されていないという値を示す  (表示此索引未使用的值)*/
	OBJTYPE_CHARA, /*  キャラクタへのインデックス  (角色的索引)   */
	OBJTYPE_ITEM,  /*  アイテムへのインデックス  (物品的索引)  */
	OBJTYPE_GOLD,  /*  ゴールド    お金の額がindexに入っている  (金币金额存入index) */
#ifdef _NPCSERVER_NEW
	OBJTYPE_NPCSCHARA,
#endif
#ifdef _MAP_WARPPOINT
	OBJTYPE_WARPPOINT,
#endif
	OBJTYPE_NUM
} OBJTYPE;

typedef struct tagObject {
	OBJTYPE type;

	char objname[256];
	int npcsindex;
	int dir;
	int imagenum;

	int chartype;
	int index;
	int x;
	int y;
	int floor;
#ifdef _DEL_DROP_GOLD
	int time;
#endif
} Object;

BOOL CHECKOBJECT(int index);
BOOL CHECKOBJECTUSE(int index);
BOOL initObjectArray(int num);
BOOL endObjectArray(void);

INLINE int _initObjectOne(char *file, int line, Object *ob);
#define initObjectOne(ob) _initObjectOne(__FILE__, __LINE__, ob)

void endObjectOne(int index);
int initObjectFromObjectMember(OBJTYPE type, int index, int x, int y, int floor);

INLINE int OBJECT_getType(int index);
INLINE int OBJECT_setType(int index, int newvalue);
INLINE int OBJECT_getFloor(int index);
INLINE int OBJECT_setFloor(int index, int newvalue);
INLINE int OBJECT_getX(int index);
INLINE int OBJECT_setX(int index, int newvalue);
INLINE int OBJECT_getY(int index);
INLINE int OBJECT_setY(int index, int newvalue);
INLINE int OBJECT_getIndex(int index);
INLINE int OBJECT_setIndex(int index, int newvalue);
INLINE int OBJECT_getNum(void);

#ifdef _DEL_DROP_GOLD
INLINE int OBJECT_getTime(int index);
INLINE int OBJECT_setTime(int index, int newvalue);
#endif

#ifdef _NPCSERVER_NEW
INLINE int OBJECT_getNpcIndex(int index);
INLINE char *OBJECT_getName(int index);
INLINE int OBJECT_getNpcImage(int index);
INLINE int OBJECT_getNpcdir(int index);
#endif
INLINE int OBJECT_getchartype(int index);
INLINE void OBJECT_setchartype(int index, int flg);

#ifndef _SIMPLIFY_ITEMSTRING
BOOL storeObjects(char *dirname);
BOOL restoreObjects(char *dirname);
#endif

int searchObjectFromCharaIndex(int index);

#endif
/*__OBJECT_h__*/
