function LuckTalk_Init( _MeIndex )
	--設置NPC形象
	Char.SetData(_MeIndex, %对像_形象%, 101283);
	Char.SetData(_MeIndex, %对像_原形%, 101283);
	--設置NPC所在地圖
	Char.SetData(_MeIndex, %对像_地图%, 2000);
	--設置X坐標
	Char.SetData(_MeIndex, %对像_X%, 48);
	--設置Y坐標
	Char.SetData(_MeIndex, %对像_Y%, 77);
	--設置NPC方向
	Char.SetData(_MeIndex, %对像_方向%, %下%);
	--設置NPC名稱
	Char.SetData(_MeIndex, %对像_原名%, "渔村幽灵");
	--[[
		響應說話事件，此事件如果有玩家對此NPC說話就會觸發
	--]]
	if (Char.SetLoopEvent(nil, "LuckTalk_LoopCallBack", _MeIndex, 3000*1000) < 0) then
		print("LuckTalk_LoopCallBack 注册事件失败。");
		return false;
	end
	return true;
end
 

function LuckTalk_LoopCallBack( _MeIndex, _TalkIndex, _Msg, _Color, _Channel)
    local rand = math.floor(math.random(1, 100));
	Char.SetTalkedEvent(nil, "LuckTalk_TalkedCallBack", _MeIndex);
	if(talkerindex==nil)then
	   talkername="Null";
	else
	   talkername=Char.GetData(talkerindex,%对像_原名%);
	end
	
        if(rand==10)then
           gift=20811;
    elseif(rand==20)then
           gift=20815;
    elseif(rand==30)then
           gift=20816;
    elseif(rand==40)then
           gift=20817;
    elseif(rand==50)then
           gift=20818;
    elseif(rand==60)then
           gift=20819;
    elseif(rand==70)then
           gift=20820;
    else
           gift=20828;
    end

    NLG.TalkToCli(-1, "神秘的幽灵在渔村里徘徊....", %绿色%, 20);
    NLG.TalkToCli(-1, "「渔村幽灵」恭喜玩家 "..talkername.." 得到神秘礼物", %黄色%, 0);
    NLG.GiveItem(talkerindex,gift);

 	--设置动作为走路
 	local walk = 1; 
 	NLG.SetAction(_MeIndex,walk); 
 	--随机よ
 	local TM_DIR = math.floor(math.random(0, 7)); 
 	--限定范围
 	if ( Char.GetData( _MeIndex, %对像_X%) >= 62+math.floor(math.random(1, 3))) then 
 		TM_DIR = 6; 
 	elseif ( Char.GetData( _MeIndex, %对像_X%) <= 62-math.floor(math.random(1, 3))) then 
 		TM_DIR = 2; 
 	elseif ( Char.GetData( _MeIndex, %对像_Y%) <= 70-math.floor(math.random(1, 3))) then 
 		TM_DIR = 4; 
 	elseif ( Char.GetData( _MeIndex, %对像_Y%) >= 70+math.floor(math.random(1, 3))) then 
 		TM_DIR = 0; 
 	end 
 	--走路
 	if(NLG.WalkMove(_MeIndex,TM_DIR) ~= 0) then 
 		NLG.WalkMove(_MeIndex,math.floor(math.random(0, 2))); 
 	end 
 	--设置よ
 	NLG.CharLook(_MeIndex,TM_DIR); 
 	return ; 

end

function LuckTalk_TalkedCallBack( _MeIndex, _TalkIndex, _Msg, _Color, _Channel)
    talkerindex=_TalkIndex;
end 
