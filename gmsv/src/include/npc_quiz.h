#ifndef __NPC_QUIZ_H__
#define __NPC_QUIZ_H__

typedef struct NPC_Quiz {

	int no;				//  クイズの番号  (谜题的编号)
	int type;			//  クイズのタイプ：ゲームスポット  (谜题类型：游戏·竞技)
	int level;			//  クイズのレベル  (谜题的等级)
	int answertype;		// 問題の答え方(①/②/③/選択マッチ)  (问题的回答方式)
	int answerNo;		//  クイズの問題  (谜题的问题)
	char question[512]; //  锁
	char select1[128];	// 蓟  靼1
	char select2[128];	// 蓟  靼2
	char select3[128];	// 蓟  靼3

} NPC_QUIZ;

void NPC_QuizTalked(int meindex, int talkerindex, char *msg,
					int color);
BOOL NPC_QuizInit(int meindex);
void NPC_QuizWindowTalked(int meindex, int talkerindex, int seqno, int select, char *data);

BOOL QUIZ_initQuiz(char *filename);

#endif

/*__NPC_QUIZ_H__*/
