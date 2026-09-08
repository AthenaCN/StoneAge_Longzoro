#ifndef __MAGIC_H__
#define __MAGIC_H__

/* 热诸 */
int MAGIC_Use(int charaindex, int haveitemindex, int toindex);
int MAGIC_Recovery(int charaindex, int toindex, int marray, int mp);
int MAGIC_OtherRecovery(int charaindex, int toindex, int marray, int mp);
int MAGIC_FieldAttChange(int charaindex, int toindex, int marray, int mp);
int MAGIC_StatusChange(int charaindex, int toindex, int marray, int mp);
#ifdef _MAGIC_DEEPPOISON
int MAGIC_StatusChange2(int charaindex, int toindex, int marray, int mp);
#endif
int MAGIC_StatusRecovery(int charaindex, int toindex, int marray, int mp);
int MAGIC_MagicDef(int charaindex, int toindex, int marray, int mp);
int MAGIC_Ressurect(int charaindex, int toindex, int marray, int mp);
int MAGIC_AttReverse(int charaindex, int toindex, int marray, int mp);
int MAGIC_ResAndDef(int charaindex, int toindex, int marray, int mp);
#ifdef _OTHER_MAGICSTAUTS
int MAGIC_MagicStatusChange(int charaindex, int toindex, int marray, int mp);
#endif
#ifdef __ATTACK_MAGIC
int MAGIC_AttMagic(int charaindex, int toindex, int marray, int mp);
#endif
#ifdef _ITEM_METAMO
int MAGIC_Metamo(int charaindex, int toindex, int marray, int mp);
#endif

#ifdef _ITEM_ATTSKILLMAGIC
int MAGIC_AttSkill(int charaindex, int toindex, int marray, int mp);
#endif
#ifdef _MAGIC_WEAKEN // vincent  精灵:虚弱
int MAGIC_Weaken(int charaindex, int toindex, int marray, int mp);
#endif
#ifdef _MAGIC_BARRIER // vincent  精灵:魔障
int MAGIC_Barrier(int charaindex, int toindex, int marray, int mp);
#endif
#ifdef _MAGIC_NOCAST // vincent  精灵:沉默
int MAGIC_Nocast(int charaindex, int toindex, int marray, int mp);
#endif
#ifdef _MAGIC_TOCALL
int MAGIC_ToCallDragon(int charaindex, int toindex, int marray, int mp);
#endif

//----------------------------------------------------------------------
// アイテムの位置から魔法番号を返す  (从物品位置返回魔法编号)
//
int MAGIC_GetArrayNo(
	int charaindex,	  // キャラクタインデックス  (角色索引)
	int haveitemindex //    持っているアイテムの位置  (所持物品的位置)
);
//
//----------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  魔法を呼び出す  (调用魔法)
//
int MAGIC_DirectUse(
	int charaindex, // 呼び出すキャラのインデックス  (调用角色的索引)
	int marray,		// 呼び出す魔法のインデックス  (调用魔法的索引)
	int toindex,	// 誰に呼び出す  (对谁调用)
	int itemnum);
//
//-------------------------------------------------------------------

#endif
