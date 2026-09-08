#include "version.h"
#include "common.h"
#include "readmap.h"
#include "char.h"
#include "map_util.h"
#include "lssproto_serv.h"
#include "net.h"
#include "anim_tbl.h"

/*#define MAPEDITORTROUBLE*/
#define SPR_kmydamY CG_HIT_MARK_10 // anim_tbl.h に載せられるまでのエフェクト  (在anim_tbl.h加载前的特效)

/*------------------------------------------------------------
 * 動けるかどうか判断する。キャラが飛んでいるかどうか番号がある  (判断能否移动。有角色是否飞行的编号)
 * 番号  (编号)
 *  ff          int     floor
 *  fx          int     x座標
 *  fy          int     y座標
 *  isfly       int       飛んでいるかどうか  (是否飞行)
 * 返り値  (返回值)
 *  動ける      TRUE(1)  (可移动)
 *  動けない    FALSE(0)  (不可移动)
 ------------------------------------------------------------*/
BOOL MAP_walkAbleFromPoint(int ff, int fx, int fy, BOOL isfly) {
	int map[2];

	if (!MAP_getTileAndObjData(ff, fx, fy, &map[0], &map[1])) {
		return FALSE;
	}

	if (isfly) {
		int i;
		for (i = 0; i < 2; i++)
			if (MAP_getImageInt(map[i], MAP_HAVEHEIGHT) == TRUE) {
				return FALSE;
			}
		return TRUE;
	} else {
		switch (MAP_getImageInt(map[1], MAP_WALKABLE)) {
		case 0:
			return FALSE;
			break;
		case 1:
			if (MAP_getImageInt(map[0], MAP_WALKABLE) == 1) {
				return TRUE;
			} else {
				return FALSE;
			}
			break;
		case 2:
			return TRUE;
			break;
		default:
			return FALSE;
			break;
		}
	}

	return FALSE;
}

/*------------------------------------------------------------
 * 動けるかどうか判断する  (判断能否移动)
 * 番号  (编号)
 *  index       int     Char    での Index  (Char 的索引)
 *  ff          int     floor id
 *  fx          int     x座標
 *  fy          int     y座標
 * 返り値  (返回值)
 *  動いてよい      TRUE(1)  (可移动)
 *  動いてはダメ    FALSE(0)  (不可移动)
 ------------------------------------------------------------*/
BOOL MAP_walkAble(int index, int ff, int fx, int fy) {
	/*      キャラならOK    (若是角色则OK) */
	if (CHAR_getFlg(index, CHAR_ISTRANSPARENT))
		return TRUE;

	// Arminius 7.9 Airplane ok
	if (CHAR_getInt(index, CHAR_WHICHTYPE) == CHAR_TYPEBUS)
		return TRUE;
	if (CHAR_getWorkInt(index, CHAR_WORKPARTYMODE) == CHAR_PARTY_CLIENT) {
		int oyaindex = CHAR_getWorkInt(index, CHAR_WORKPARTYINDEX1);
		if (CHAR_CHECKINDEX(oyaindex)) {
			if (CHAR_getInt(oyaindex, CHAR_WHICHTYPE) == CHAR_TYPEBUS)
				return TRUE;
		}
	}

	return MAP_walkAbleFromPoint(ff, fx, fy, CHAR_getFlg(index, CHAR_ISFLYING) | CHAR_getWorkInt(index, CHAR_WORKSKYWALKER));
}

/*------------------------------------------------------------
 * 座標を指定してそこのオブジェクトに高さがあるかどうか  (指定坐标，判断该处物体是否有高度)
 * 番号  (编号)
 *  fl              int     フロア  (楼层)
 *  x               int     x 座標
 *  y               int     y 座標
 * 返り値  (返回值)
 *  高さがある  TRUE  (有高度)
 *  高さがない  FALSE  (无高度)
 ------------------------------------------------------------*/
BOOL MAP_haveHeight(int fl, int x, int y) {
	int map[2];

	/*  マップデータを  持ってくる  (取地图数据) */
	if (!MAP_getTileAndObjData(fl, x, y, &map[0], &map[1]))
		return FALSE;

	return MAP_getImageInt(map[1], MAP_HAVEHEIGHT);
}

/*----------------------------------------
 * ステータス状態の  更新をする  (更新状态)
 * 番号  (编号)
 *  index   int     キャラのインデックス  (角色索引)
 *  map     int     マップデータ  (地图数据)
 *  outof   BOOL    進む時、戻る時ならば進む時である  (若是前进/返回则为前进时)
 * 返り値  (返回值)
 *  パラメータ化をした    TRUE  (已参数化)
 *  パラメータ化をしなかった  FALSE  (未参数化)
  ----------------------------------------*/
static BOOL MAP_changeCharStatusFromMapDataAndTime(int index,
												   int map, BOOL outof) {
#if 0
// ストーンエイジでは使わない  (石器时代中不使用)
    int i;
    int offset;
    BOOL    change=FALSE;
    static struct tagStatusInteractionOfBitAndDefine{
        int     mapdataindex;
        int     charadataindex;
    }statusInteraction[]={
        {MAP_INTOPOISON,        CHAR_POISON},
        {MAP_INTOPARALYSIS,     CHAR_PARALYSIS},
        {MAP_INTOSILENCE,       CHAR_SLEEP},
        {MAP_INTOSTONE,         CHAR_STONE},
        {MAP_INTODARKNESS,      CHAR_DRUNK},
        {MAP_INTOCONFUSION,     CHAR_CONFUSION},

        {MAP_OUTOFPOISON,       CHAR_POISON},
        {MAP_OUTOFPARALYSIS,    CHAR_PARALYSIS},
        {MAP_OUTOFSILENCE,      CHAR_SLEEP},
        {MAP_OUTOFSTONE,        CHAR_STONE},
        {MAP_OUTOFDARKNESS,     CHAR_DRUNK},
        {MAP_OUTOFCONFUSION,    CHAR_CONFUSION},
    };

    if( outof == FALSE )offset = 6;
    else                offset= 0;
    for( i = 0 ; i < 6 ; i ++ ){
        int     newdata;
        newdata = MAP_getImageInt(map,statusInteraction[i+offset].
                                  mapdataindex);
        if( newdata > 0 ){
            change=TRUE;
            /*  ステータスの設定    (状态设定) */
            CHAR_setInt( index,
                         statusInteraction[i+offset].charadataindex,
                         CHAR_getInt(index,
                                     statusInteraction[i+offset].
                                     charadataindex ) + newdata );
        }
    }
    return change;
#else
	return FALSE;
#endif
}

/*------------------------------------------------------------
 * Map イベントの pre postを処理する  (处理地图事件的pre、post)
 * 番号  (编号)
 *  index       int     移ろうとしているキャラのインデックス  (正在移动角色的索引)
 *  mode        BOOL    TRUEの時はin  FALSEの時はout に対応  (TRUE时对应in，FALSE时对应out)
 * 返り値なし  (无返回值)
 ------------------------------------------------------------*/
static void MAP_dealprepostevent(int index, BOOL mode) {
	int map[2];
	int i;
	int damaged = FALSE, statuschange = FALSE;

	if (CHAR_getFlg(index, CHAR_ISFLYING))
		/*    飛んでるやつには何もしない  (对飞行者不做任何事) */
		return;

	if (!MAP_getMapDataFromCharIndex(index, map))
		return;

	for (i = 0; i < 2; i++) {
		/*  damage */
		int damage;
		damage = MAP_getImageInt(map[i],
								 mode ? MAP_INTODAMAGE : MAP_OUTOFDAMAGE);
		if (damage != 0) {
			/*  ダメージ動を表で現すので番号を覚えておく    (用表呈现伤害动画，故记住编号) */
			int opt[2] = {SPR_kmydamY, -damage};
			damaged = TRUE;
			CHAR_setInt(index, CHAR_HP,
						CHAR_getInt(index, CHAR_HP) + damage);
			CHAR_complianceParameter(index);
			/*  ダメージ受けたエフェクト表示    (显示受到伤害的特效) */
			CHAR_sendWatchEvent(CHAR_getWorkInt(index, CHAR_WORKOBJINDEX),
								CHAR_ACTDAMAGE, opt, 2, TRUE);
		}
		if (MAP_changeCharStatusFromMapDataAndTime(
				index, map[i], mode ? TRUE : FALSE))
			statuschange = TRUE;
	}
	if (damaged)
		CHAR_sendStatusString(index, "M");
	if (statuschange) {
		CHAR_sendCToArroundCharacter(index);
		CHAR_sendStatusString(index, "P");
	}
}

/*----------------------------------------
 * そのタイルに移る前に呼ばれる。ここでそこから戻るときの煙の  (移动到该地块前被调用。在这里设定从那里返回时的烟雾)
 * 絵を表示する  (显示画面)
 * 番号  (编号)
 *  index       int     移ろうとしているキャラのインデックス  (正在移动角色的索引)
 *  flooor      int     フロアID  (楼层ID)
 *  fx          int     x座標
 *  fy          int     y座標
 * 返り値  (返回值)
 *  なし  (无)
 ----------------------------------------*/
void MAP_preovered(int index) {
	MAP_dealprepostevent(index, FALSE);
}

/*----------------------------------------
 * そのタイルに移った  に呼ばれる  (移动到该地块时被调用)
 * 番号  (编号)
 *  index       int     移ろうとしているキャラのインデックス  (正在移动角色的索引)
 * 返り値  (返回值)
 *  なし  (无)
 ----------------------------------------*/
void MAP_postovered(int index) {
	MAP_dealprepostevent(index, TRUE);
}

/*----------------------------------------
 * キャラの周りのマップをすべて送る  (发送角色周围全部地图)
 * 番号  (编号)
 *  fd          int
 *  charaindex  int     キャラのインデックス  (角色索引)
 * 返り値  (返回值)
 *  成功      TRUE(1)  (成功)
 *  失敗      FALSE(0)  (失败)
 ----------------------------------------*/
BOOL MAP_sendArroundCharNeedFD(int fd, int charaindex) {
	char *stringdata;
	int x = CHAR_getInt(charaindex, CHAR_X);
	int y = CHAR_getInt(charaindex, CHAR_Y);
	int fl = CHAR_getInt(charaindex, CHAR_FLOOR);
	int size = MAP_CHAR_DEFAULTSEESIZ;
	RECT seekr, retr;
	seekr.x = x - (int)(size / 2);
	seekr.y = y - (int)(size / 2);
	seekr.width = size;
	seekr.height = size;
#if 1
	{
		int tilesum, objsum, eventsum;
		stringdata = MAP_getChecksumFromRECT(fl, &seekr, &retr, &tilesum, &objsum, &eventsum);
		if (stringdata == NULL)
			return FALSE;

		lssproto_MC_send(fd, fl,
						 retr.x, retr.y,
						 retr.x + retr.width, retr.y + retr.height,
						 tilesum,
						 objsum,
						 eventsum,
						 stringdata);
	}
#else
	stringdata = MAP_getdataFromRECT(fl, &seekr, &retr);
	if (stringdata == NULL)
		return FALSE;

	lssproto_M_send(fd, fl,
					retr.x, retr.y,
					retr.x + retr.width, retr.y + retr.height,
					stringdata);
#endif
	return TRUE;
}

/*----------------------------------------
 * マップデータを  取る  (取地图数据)
 * 番号  (编号)
 *  charaindex      int     キャラのインデックス  (角色索引)
 * 返り値  (返回值)
 *  成功      TRUE(1)  (成功)
 *  失敗      FALSE(0)  (失败)
 ----------------------------------------*/
BOOL MAP_sendArroundChar(int charaindex) {
	int fd;
	fd = getfdFromCharaIndex(charaindex);
	if (fd == -1)
		return FALSE;

	return MAP_sendArroundCharNeedFD(fd, charaindex);
}
