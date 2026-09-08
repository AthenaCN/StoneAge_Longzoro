#define __HANDLETIME_C__
#include "version.h"
#include <time.h>
#include <sys/time.h>

#include "common.h"
#include "handletime.h"

#define LSTIME_SECONDS_PER_DAY 5400 /* LSTIME一秒ぶんの現実の時間数 (LSTIME 1秒对应的现实时间数) */

/*
  LSTIME_SECONDS_PER_DAY を変えると時間の進み具合を変えることができる。 (改变LSTIME_SECONDS_PER_DAY可改变时间的推进速度)

  つまりLS時間で一秒あたりの現実の時間数 (即LS时间中1秒对应的现实时间数)
  9000 (  秒)   2.5 [hour]
  900           0.25[hour] = 15[min]
  90            0.025[hour] = 1.5[min] = 90[sec]
  9             9[sec]

*/

#define LSTIME_HOURS_PER_DAY 1024 /* LSTIME一秒ぶんのLSTIMEの時間数 (LSTIME 1秒对应的LSTIME时间数) */
#define LSTIME_DAYS_PER_YEAR 100  /* LSTIME一秒ぶんのLSTIMEの日数 (LSTIME 1秒对应的LSTIME日数) */

// WON REM
/*
// Nuke 0701: localtime

  struct timeval NowTime;
#ifdef localtime
#undef localtime
#endif

struct tm *localtime(const time_t *timep)
{
	static struct tm lt;
	memset(&lt,0,sizeof(lt));
	lt.tm_sec=*timep %60;
	lt.tm_min=(*timep %3600) / 60;
	lt.tm_hour=(*timep % 86400) / 3600;
	return &lt;
}
*/

/*------------------------------------------------------------
 * 一ループに一回呼ばれて時間を進める。 (每次循环被调用一次，推进时间)
 * 引数 (参数)
 *  なし (无)
 * 返り値 (返回值)
 *  真      TRUE(1) (真)
 *  偽      FALSE(0) (偽)
 ------------------------------------------------------------*/
BOOL setNewTime(void)
{
	if (gettimeofday(&NowTime, (struct timezone *)NULL) != 0) {
		NowTime.tv_sec = time(0);
		// Nuke 0701: Localtime down
		print("\n time err !! \n");
		return FALSE;
	}
	NowTime.tv_sec += DEBUG_ADJUSTTIME;
	return TRUE;
}

/*******************************************************************
今を元にした。 (以当前时刻为基准) byHiO 1998/12/4 18:37
*******************************************************************/
static long era = (long)912766409 + 5400;
/* SAのために残した。 (为SA保留) */
/* LS元を定義する適当な数値。 (定义LS元期的适当数值)
クライアントと同じじゃないとだめだ。 (必须与客户端相同)*/

/*******************************************************************
	マシン時間からLS時間にする (从机器时间转换为LS时间)
	long t : timeで取る (用time取得)
	LSTIME *lstime : LSTIME構造体へのポインタ (指向LSTIME结构体的指针)
*******************************************************************/
void RealTimeToLSTime(long t, LSTIME *lstime)
{
	long lsseconds = t - era; /* LS元からの秒数 (从LS元期起的秒数) */
	long lsdays;			  /* LS元からの秒数 (从LS元期起的秒数) */

	/* 元からの秒数を1年の秒数で割ると年になる。 (用自元期以来的秒数除以1年的秒数得到年份) */
	lstime->year = (int)(lsseconds / (LSTIME_SECONDS_PER_DAY * LSTIME_DAYS_PER_YEAR));

	lsdays = lsseconds / LSTIME_SECONDS_PER_DAY; /* まず元からの秒数を計算して (先计算自元期以来的秒数) */
	lstime->day = lsdays % LSTIME_DAYS_PER_YEAR; /*   1日分の秒数で割ったあまりが日。 (除以1天的秒数所得余数为日)  */

	/*(450*12)  で1日 (450*12为1天)  */
	lstime->hour = (int)(lsseconds % LSTIME_SECONDS_PER_DAY)
				   /* ここまでで一日分が求まってから何時間たったか。 (到此求出经过一天后过了多少时间) */
				   * LSTIME_HOURS_PER_DAY / LSTIME_SECONDS_PER_DAY;
	/* 一秒あたりの日数で割ってから一秒あたりの時間数をかけると現在何時間
	 なのかがわかる。 (先除以每秒日数，再乘以每秒时间数，即可得知当前时间)*/

	return;
}

/*******************************************************************
	LS時間からマシン時間にする (从LS时间转换为机器时间)
	LSTIME *lstime : LSTIME構造体へのポインタ (指向LSTIME结构体的指针)
	long *t :   時間へのポインタ (指向时间的指针)
*******************************************************************/
void LSTimeToRealTime(LSTIME *lstime, long *t)
{
	*t = (long)((lstime->hour * LSTIME_DAYS_PER_YEAR + lstime->day) /* 時間 (时间) */
					* LSTIME_HOURS_PER_DAY

				+ lstime->year)
		 /*この関数はバグっているように見える。nakamura (这个函数看起来有bug。nakamura) */

		 * 450;
	return;
}

/*******************************************************************
	LS時間で今の時間区分を返す。 (获取当前LS时间的时段)
	  戻り値 int :   0:朝 1:昼 2:夕 3:夜 (返回值 int : 0:早晨 1:白天 2:傍晚 3:夜晚)
	LSTIME *lstime : LSTIME構造体へのポインタ (指向LSTIME结构体的指针)
*******************************************************************/
LSTIME_SECTION getLSTime(LSTIME *lstime)
{
	if (NIGHT_TO_MORNING < lstime->hour
		&& lstime->hour <= MORNING_TO_NOON)
		return LS_MORNING;
	else if (NOON_TO_EVENING < lstime->hour
			 && lstime->hour <= EVENING_TO_NIGHT)
		return LS_EVENING;
	else if (EVENING_TO_NIGHT < lstime->hour
			 && lstime->hour <= NIGHT_TO_MORNING)
		return LS_NIGHT;
	else
		return LS_NOON;
}

#ifdef _ASSESS_SYSEFFICACY
static clock_t TotalClock = 0;
static clock_t StartClock = 0;
// static int EndClock = 0;
// static float SysTime=0.0;
static clock_t EndClock = 0;
static double SysTime = 0.0;
static int Cnum = 0;

#ifdef _ASSESS_SYSEFFICACY_SUB
static clock_t Net_TotalClock = 0;
static clock_t NPCGEN_TotalClock = 0;
static clock_t Battle_TotalClock = 0;
static clock_t Char_TotalClock = 0;
static clock_t Petmail_TotalClock = 0;
static clock_t Family_TotalClock = 0;
static clock_t SaveCheck_TotalClock = 0;
static clock_t GMBroadCast_TotalClock = 0;
static double Net_SysTime = 0.0;
static double NPCGEN_SysTime = 0.0;
static double Battle_SysTime = 0.0;
static double Char_SysTime = 0.0;
static double Petmail_SysTime = 0.0;
static double Family_SysTime = 0.0;
static double SaveCheck_SysTime = 0.0;
static double GMBroadCast_SysTime = 0.0;
static clock_t SubStartClock = 0;
#endif

void Assess_InitSysEfficacy()
{
	TotalClock = 0;
	StartClock = 0;
	EndClock = 0;
#ifdef _ASSESS_SYSEFFICACY_SUB
	Net_TotalClock = 0;
	NPCGEN_TotalClock = 0;
	Battle_TotalClock = 0;
	Char_TotalClock = 0;
	Petmail_TotalClock = 0;
	Family_TotalClock = 0;
	SaveCheck_TotalClock = 0;
	GMBroadCast_TotalClock = 0;

	SubStartClock = 0;
#endif
}

void Assess_SysEfficacy(int flg)
{
	if (flg == 0) {
		StartClock = clock();
	} else if (flg == 1) {
		EndClock = clock();
		if (EndClock < StartClock)
			return;
		TotalClock += (int)(EndClock - StartClock);
		Cnum++;
		if (Cnum % 500 == 0) {
			SysTime = (float)(TotalClock / Cnum) / CLOCKS_PER_SEC;
			TotalClock = 0;
#ifdef _ASSESS_SYSEFFICACY_SUB
			Net_SysTime = (float)(Net_TotalClock / Cnum) / CLOCKS_PER_SEC;
			Net_TotalClock = 0;
			NPCGEN_SysTime = (float)(NPCGEN_TotalClock / Cnum) / CLOCKS_PER_SEC;
			NPCGEN_TotalClock = 0;
			Battle_SysTime = (float)(Battle_TotalClock / Cnum) / CLOCKS_PER_SEC;
			Battle_TotalClock = 0;
			Char_SysTime = (float)(Char_TotalClock / Cnum) / CLOCKS_PER_SEC;
			Char_TotalClock = 0;
			Petmail_SysTime = (float)(Petmail_TotalClock / Cnum) / CLOCKS_PER_SEC;
			Petmail_TotalClock = 0;
			Family_SysTime = (float)(Family_TotalClock / Cnum) / CLOCKS_PER_SEC;
			Family_TotalClock = 0;
			SaveCheck_SysTime = (float)(SaveCheck_TotalClock / Cnum) / CLOCKS_PER_SEC;
			SaveCheck_TotalClock = 0;
			GMBroadCast_SysTime = (float)(GMBroadCast_TotalClock / Cnum) / CLOCKS_PER_SEC;
			GMBroadCast_TotalClock = 0;
#endif
			Cnum = 0;
		}
	}
	/*
	EndClock = clock();
	if( StartClock != 0 ){
		if( EndClock < StartClock ) return;
		TotalClock += (int)(EndClock-StartClock);
		Cnum++;
		if( Cnum%500 == 0 ){
			SysTime = (float)TotalClock/Cnum;
			TotalClock = 0;
		}
	}
	StartClock = EndClock;
	*/
}

void ASSESS_getSysEfficacy(float *TVsec)
{
	*TVsec = SysTime;
}

#ifdef _ASSESS_SYSEFFICACY_SUB
void Assess_SysEfficacy_sub(int flg, int loop)
{

	if (flg == 0) {
		SubStartClock = clock();
	} else if (flg == 1) {
		EndClock = clock();
		if (EndClock < SubStartClock)
			return;

		switch (loop) {
		case 1: // Net_TotalClock
			Net_TotalClock += (int)(EndClock - SubStartClock);
			break;
		case 2: // NPCGEN_TotalClock
			NPCGEN_TotalClock += (int)(EndClock - SubStartClock);
			break;
		case 3: // Battle_TotalClock
			Battle_TotalClock += (int)(EndClock - SubStartClock);
			break;
		case 4: // Char_TotalClock
			Char_TotalClock += (int)(EndClock - SubStartClock);
			break;
		case 5: // Petmail_TotalClock
			Petmail_TotalClock += (int)(EndClock - SubStartClock);
			break;
		case 6: // Family_TotalClock
			Family_TotalClock += (int)(EndClock - SubStartClock);
			break;
		case 7: // SaveCheck_TotalClock
			SaveCheck_TotalClock += (int)(EndClock - SubStartClock);
			break;
		case 8: // GMBroadCast_TotalClock
			GMBroadCast_TotalClock += (int)(EndClock - SubStartClock);
			break;
		}
	}
}

void ASSESS_getSysEfficacy_sub(float *TVsec, int loop_index)
{
	switch (loop_index) {
	case 1:
		*TVsec = Net_SysTime;
		break;
	case 2:
		*TVsec = NPCGEN_SysTime;
		break;
	case 3:
		*TVsec = Battle_SysTime;
		break;
	case 4:
		*TVsec = Char_SysTime;
		break;
	case 5:
		*TVsec = Petmail_SysTime;
		break;
	case 6:
		*TVsec = Family_SysTime;
		break;
	case 7:
		*TVsec = SaveCheck_SysTime;
		break;
	case 8:
		*TVsec = GMBroadCast_SysTime;
		break;
	}
}

#endif

#endif

#ifdef _CHECK_BATTLETIME

#include "battle.h"
static clock_t battleComClock = 0;
double battleComTotalTime[BATTLE_COM_END];
long battleComTotalUse[BATTLE_COM_END];

void check_battle_com_init(void)
{
	print("\n check_battle_com_init... ");
	print("\n BATTLE_COM_END = %d ", BATTLE_COM_END);
	memset(battleComTotalTime, 0, sizeof(double) * BATTLE_COM_END);
	memset(battleComTotalUse, 0, sizeof(long) * BATTLE_COM_END);
}

void check_battle_com_begin(void)
{
	print(" bi ");
	battleComClock = clock();
}

void check_battle_com_end(int b_com)
{
	clock_t endClock;
	double usedClock;

	endClock = clock();
	usedClock = (double)(endClock - battleComClock) / CLOCKS_PER_SEC;

	print(" BC[%d,%0.10f] ", b_com, usedClock);
	battleComTotalTime[b_com] += usedClock;
	battleComTotalUse[b_com]++;

	print(" bo ");
}

void check_battle_com_show(void)
{
	FILE *outfile;
	int i;
	char outstr[1024];

	outfile = fopen("battle_com_time.txt", "w");
	if (!outfile) {
		print("\n OPEN battle_com_time.txt ERROR!!! \n");
		return;
	}

	for (i = 0; i < BATTLE_COM_END; i++) {
		sprintf(outstr, "%d\t=\t%0.10f\t*\t%d\n",
				i,
				(double)(battleComTotalTime[i] / battleComTotalUse[i]),
				battleComTotalUse[i]);
		fputs(outstr, outfile);
	}
	fclose(outfile);

	print("\n RECORD battle_com_time.txt COMPLETE \n");
}

#endif
