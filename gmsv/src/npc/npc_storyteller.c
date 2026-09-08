#include "version.h"
#include "char.h"

/*


  かたりべ  (说书人)

  リリスでは2種の言霊  (在莉莉丝有2种言灵)

 NPCARGUMENT:  0が指定なら言霊0、1が指定なら言霊1の設定  (NPC参数：指定0则言灵0，指定1则言灵1)

 状態は3つあり、順に進展する  (有3种状态，依次推进)

 状態0で アイテムイベント。この状態でtalkすると  (状态0为物品事件，在此状态下被对话时)
 ＞何かすると、もじもじ…中に戻ってしまうので、怪しいかたりべめ…＝で  (＞若做什么就会扭捏…回到途中，这个可疑的说书人…＝)
 状態1に移行  (转移到状态1)
 状態1でtalkされたら  (在状态1被搭话时)
 ＞その時の物語に沿って、魂を減らして、？の霊を増やそう、みたいな  (＞按当时的故事情节减少魂、增加？灵之类的)
		その霊を物語に? yes/no＝yesなら状態2へ。noなら  (将那个灵编入故事？yes/no＝yes则转到状态2，no则)
		＞ではさらばじゃ＝で状態0へ  (那么别过了＝回到状态0)

 状態2でtalkされたら  (在状态2被搭话时)
 ＞その時に怪しい霊＞もじもじ＝を怪しんで、の最後に消してしまう  (＞把那时的可疑之灵＞扭捏＝给消除掉)
		一話切にするべし さらばじゃ＝  (该结束这则故事了 别过啦＝)
		といって魂を減らし、霊フラグを追加する  (说着减少灵魂并追加灵标志)
		で状態0に戻る  (回到状态0)




 Talked:

 if(     話しかけられて1回目なら ){  (若是第1次被搭话){
   状態0に戻る  (回到状态0)
 }

 switch(状態){  (switch(状态){)
 case 0:  処理 ［状態1へ。break;  (处理，转到状态1)
 case 1:  処理 ［状態2へ。break;  (处理，转到状态2)
 case 2:  処理 ［状態0へ。break;  (处理，转到状态0)
 }

 }









 */
BOOL NPC_StoryTellerInit(int meindex) {
	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_TYPESTORYTELLER);
	CHAR_setFlg(meindex, CHAR_ISATTACKED, 0);
	CHAR_setFlg(meindex, CHAR_ISOVERED, 0);

	return TRUE;
}

void NPC_StoryTellerTalked(int meindex, int talker, char *msg, int col) {
}
