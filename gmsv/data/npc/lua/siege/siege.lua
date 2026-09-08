function Siege_Init( _MeIndex )
	--戰鬥索引全域變數
	iBattle2[_MeIndex] = 0;	
	--初始位置全域變數
	iX2[_MeIndex] = 0;
	iY2[_MeIndex] = 0;

	--設置踩踏回應事件
	if (Char.SetPostOverEvent(nil, "Siege_Covered", _MeIndex) < 0) then
		print("Siege_Covered 注册事件失败。");
		return false;
	end

	if (Char.SetPreOverEvent(nil, "Siege_Covered", _MeIndex) < 0) then
		print("Siege_Covered 注册事件失败。");
		return false;
	end
	--設置對話回應事件
	if (Char.SetTalkedEvent(nil, "Siege_Talked", _MeIndex) < 0) then
		print("Siege_Talked 注册事件失败。");
		return false;
	end
	--設置迴圈事件
	if (Char.SetLoopEvent(nil, "Siege_Walk", _MeIndex,2000) < 0) then
		print("Siege_Walk 注册事件失败。");
		return false;
	end
	--更新
	NLG.UpChar(_MeIndex);
	return true;
end

function Siege_Walk( _MeIndex)

	--設置動作為走路
	local walk = 1;
	NLG.SetAction(_MeIndex,walk);
	--隨機方向
	local TM_DIR = math.floor(math.random(0, 7));
	--限定範圍
	if ( Char.GetData( _MeIndex, %对像_X%) >= iX2[_MeIndex]+math.floor(math.random(3, 7))) then
		TM_DIR = 6;
	elseif ( Char.GetData( _MeIndex, %对像_X%) <= iX2[_MeIndex]-math.floor(math.random(3, 7))) then
		TM_DIR = 2;
	elseif ( Char.GetData( _MeIndex, %对像_Y%) <= iY2[_MeIndex]-math.floor(math.random(3, 7))) then
		TM_DIR = 4;	
	elseif ( Char.GetData( _MeIndex, %对像_Y%) >= iY2[_MeIndex]+math.floor(math.random(3, 7))) then
		TM_DIR = 0;
	end
	--走路
	if(NLG.WalkMove(_MeIndex,TM_DIR) ~= 0) then
		NLG.WalkMove(_MeIndex,math.floor(math.random(0, 7)));
	end
	--設置方向
	NLG.CharLook(_MeIndex,TM_DIR);
	return ;
end

function Siege_Covered( _MeIndex, _PlayerIndex )
	--判斷踩中了什麼
	if( Char.GetData(_PlayerIndex,%对像_脸%) == 0)then
		return;
	else
		local cnt = 1;
		local PlayerNumber = 1 ;
		PlayerALV = Char.GetData(_PlayerIndex,%对像_等级%);
		local TMP_Index;
		while (cnt < 5) do
			TMP_Index = Char.GetTeamIndex(_PlayerIndex,cnt);
			if (TMP_Index > 0) then
				PlayerNumber = PlayerNumber + 1;
				PlayerALV = PlayerALV + Char.GetData(TMP_Index,%对像_等级%);
			end
			cnt = cnt + 1;
		end
		PlayerALV = PlayerALV/PlayerNumber;
		--設置站立動作
		NLG.SetAction(_MeIndex,0);
		NLG.TalkToCli(_PlayerIndex, "呼噜呼噜呼噜噜…………", %红色%, 0, _MeIndex)
		
		--敵人陣列信息
		local TM_EnemyIdAr = {{2535,2536,2537,2538,2539},{2535,2536,2537,2538,2539},{2535,2536,2537,2538,2539},{2535,2536,2537,2538,2539},{2535,2536,2537,2538,2539}};
		local TM_BaseLevel = {  { LV200 , LV200 } ,
					{ LV210 , LV200 , LV200 } ,
					{ LV250 , LV230 , LV230 , LV230 , LV230 } ,
					{ LV300 , LV230 , LV230 , LV230 , LV230 , LV300 , LV230 , LV230 , LV230 , LV230 } ,
					{ LV350 , LV250 , LV250 , LV250 , LV250 , LV400 , LV240 , LV240 , LV240 , LV240 }
					};
		local TM_SkillType = {{0,0},{0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0}};
		--保存戰鬥索引
		iBattle2[_MeIndex] = NLG.CreateBattle( _PlayerIndex, _MeIndex, nil, TM_EnemyIdAr[PlayerNumber], TM_BaseLevel[PlayerNumber], TM_SkillType[PlayerNumber]); 
		--設置戰鬥勝利事件
		Battle.SetWinEvent("data/npc/lua/siege/siege.lua","Siege_BattleOver",iBattle2[_MeIndex]);
		--取消踩踏回應事件
		Char.SetPreOverEvent(nil, nil, _MeIndex);
		Char.SetPostOverEvent(nil, nil, _MeIndex);
		--取消對話回應事件并設置新對話回應事件
		Char.SetTalkedEvent(nil, nil, _MeIndex);
		Char.SetTalkedEvent(nil, "Siege_Talked2", _MeIndex);
		--取消迴圈事件并設置新迴圈事件(用作檢查戰鬥是否結束)
		Char.SetLoopEvent(nil, nil, _MeIndex,100);
		Char.SetLoopEvent(nil, "Siege_Check", _MeIndex,500);
		--更新
		NLG.UpChar(_MeIndex);
	end
	return ;

end

function Siege_BattleOver( _BattleIndex, _CreateIndex)
	--取消檢查戰鬥是否結束的迴圈事件
	Char.SetLoopEvent(nil, nil, _CreateIndex,1000);
	--減少統計只數
	iCounter2 = iCounter2 - 1;
	local cnt = 1;
	local PlayerIndex = {-1,-1,-1,-1,-1};
	local count = 1;
	local playnum = 0;
	while (cnt <= 5) do
		local TMP_Index = Battle.GetPlayIndex(_BattleIndex,0,cnt-1);
		if (TMP_Index > 0) then
			playnum = playnum + 1;
		end
		cnt = cnt + 1;
	end
	local PrizeLevel = 1;
	if(playnum == 1) then
		PrizeLevel = 1;
	elseif(playnum ==2) then
		PrizeLevel = 2;
	elseif(playnum ==3) then
		PrizeLevel = 3;
	elseif(playnum ==4) then
		PrizeLevel = 4;
	else
		PrizeLevel = 5;
	end
	local i = 0;
	--[[	這樣設置的話,
		1號獎品的概率為二分之一
		2號獎品的概率為四分之一
		還有四分之一的概率不得獎品	]]--
	local PrizeList = {	{33249},
				{33250},
				{33251},
				{33252},
				{33253}	};

	local Tmp_Int;
	--獲得戰鬥中的玩家索引
	while (count < 6) do
		PlayerIndex[count] = Battle.GetPlayIndex(_BattleIndex,0,count-1);
		if(PlayerIndex[count] >= 0) then
			--[[	此處對PlayerIndex[count]設置獎勵
				(math.random(1, 4)中4為最大獎品數量
				即PrizeList中的項數	]]--
			i = i + 4;
			Tmp_Int = math.floor(math.random(1, 2))
			if( PrizeList[PrizeLevel][Tmp_Int] >= 0) then
				if(NLG.GiveItem( PlayerIndex[count] , PrizeList[PrizeLevel][Tmp_Int]) < 0)then
					NLG.TalkToCli(PlayerIndex[count], "道具栏位不足!", %白色%, 0, -1);
				end
			end
			NLG.TalkToCli(PlayerIndex[count], "呼噜噜噜噜噜…………(似乎是打输了很不甘愿的样子…)", %红色%, 0, _CreateIndex);
		else
			print(NL.GetErrorStr());
		end
		count = count + 1 ;
	end

	--刪除NPC
	local Tmp_Buff;
	if( i == 1) then
		Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."打倒了一只怪物。";
		NLG.TalkToCli(-1,Tmp_Buff , %红色%, 0, -1);
	elseif( i == 2) then
		Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."、"..Char.GetData(PlayerIndex[2],%对像_原名%).."打倒了一只怪物。";
		NLG.TalkToCli(-1,Tmp_Buff , %红色%, 0, -1);
	elseif( i == 3) then
		Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."、"..Char.GetData(PlayerIndex[2],%对像_原名%).."、"..Char.GetData(PlayerIndex[3],%对像_原名%).."打倒了一只怪物。";

		NLG.TalkToCli(-1, Tmp_Buff, %红色%, 0, -1);
	elseif( i == 4) then
		Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."、"..Char.GetData(PlayerIndex[2],%对像_原名%).."、"..Char.GetData(PlayerIndex[3],%对像_原名%).."、"..Char.GetData(PlayerIndex[4],%对像_原名%).."打倒了一只怪物。";
		NLG.TalkToCli(-1, Tmp_Buff, %红色%, 0, -1);
	elseif( i == 5) then
		Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."、"..Char.GetData(PlayerIndex[2],%对像_原名%).."、"..Char.GetData(PlayerIndex[3],%对像_原名%).."、"..Char.GetData(PlayerIndex[4],%对像_原名%).."、"..Char.GetData(PlayerIndex[5],%对像_原名%).."打倒了一只怪物。";

		NLG.TalkToCli(-1, Tmp_Buff, %红色%, 0, -1);
	end
	NL.DelNpc(_CreateIndex);
	return;
end

function Siege_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	--檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 2) == false) then
		return ;
	end 
	--面向玩家
	local i;
	i = Char.GetData(_PlayerIndex, %对像_方向%);
	if i >= 4 then 
		i = i - 4;
	else
		i = i + 4;		
	end
	Char.SetData(_MeIndex, %对像_方向%,i);
	NLG.UpChar( _MeIndex);

	--計算挑戰者隊伍人數及隊伍平均等級以確定怪物強度
	local cnt = 1;
	local PlayerNumber = 1 ;
	PlayerALV = Char.GetData(_PlayerIndex,%对像_等级%);
	local TMP_Index;
	while (cnt < 5) do
		TMP_Index = Char.GetTeamIndex(_PlayerIndex,cnt);
		if (TMP_Index > 0) then
			PlayerNumber = PlayerNumber + 1;
			PlayerALV = PlayerALV + Char.GetData(TMP_Index,%对像_等级%);
		end
		cnt = cnt + 1;
	end
	PlayerALV = PlayerALV/PlayerNumber;
	--設置站立動作
	NLG.SetAction(_MeIndex,0);
	NLG.TalkToCli(_PlayerIndex, "呼噜呼噜噜…………", %红色%, 0, _MeIndex);

	--敵人陣列信息
	local TM_EnemyIdAr = {{2535,2536,2537,2538,2539},{2535,2536,2537,2538,2539},{2535,2536,2537,2538,2539},{2535,2536,2537,2538,2539},{2535,2536,2537,2538,2539}};
		local TM_BaseLevel = {  { LV200 , LV200 } ,
					{ LV210 , LV200 , LV200 } ,
					{ LV250 , LV230 , LV230 , LV230 , LV230 } ,
					{ LV300 , LV230 , LV230 , LV230 , LV230 , LV300 , LV230 , LV230 , LV230 , LV230 } ,
					{ LV350 , LV250 , LV250 , LV250 , LV250 , LV400 , LV240 , LV240 , LV240 , LV240 }
					};
		local TM_SkillType = {{0,0},{0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0}};
	--保存戰鬥索引
	iBattle2[_MeIndex] = NLG.CreateBattle( _PlayerIndex, _MeIndex, nil, TM_EnemyIdAr[PlayerNumber], TM_BaseLevel[PlayerNumber], TM_SkillType[PlayerNumber]); 
	--設置戰鬥勝利事件
	Battle.SetWinEvent("data/npc/lua/siege/siege.lua","Siege_BattleOver",iBattle2[_MeIndex]);
	--取消踩踏回應事件
	Char.SetPreOverEvent(nil, nil, _MeIndex);
	Char.SetPostOverEvent(nil, nil, _MeIndex);
	--取消對話回應事件并設置新對話回應事件
	Char.SetTalkedEvent(nil, nil, _MeIndex);
	Char.SetTalkedEvent(nil, "Siege_Talked2", _MeIndex);
	--取消迴圈事件并設置新迴圈事件(用作檢查戰鬥是否結束)
	Char.SetLoopEvent(nil, nil, _MeIndex,100);
	Char.SetLoopEvent(nil, "Siege_Check", _MeIndex,500);
	--更新
	NLG.UpChar(_MeIndex);
	return;
end

function Siege_Talked2( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	--檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then
		return ;
	end 
	--正在戰鬥中的對話事件
	NLG.TalkToCli(_PlayerIndex, "呼噜噜噜～～～！", %红色%, 0, _MeIndex);
	NLG.TalkToCli(_PlayerIndex, Char.GetData( _PlayerIndex,%对像_原名%) .. "：(似乎已经有人在和这只怪物战斗了，我想我还是去看看有没有别只怪物在作乱吧。)", _Color, 0);
	return ;
end

function Siege_Check( _MeIndex)
	--此為檢查戰鬥是否結束的函數
--	print("ING");
	local PlayerIndex = {-1,-1,-1,-1,-1};
	local count = 1;
	--獲取戰鬥中玩家索引
	while (count < 6) do
		PlayerIndex[count] = Battle.GetPlayIndex(iBattle2[_MeIndex],0,count-1);
		count = count + 1 ;
	end
	--如玩家索引不全為零則戰鬥尚未結束
	--否則判定戰鬥結束
	--因為如果戰鬥勝利
	--戰鬥勝利事件中取消了此迴圈事件
	--所以在此可判定戰鬥失敗
	if(PlayerIndex[1] ~=-1 or PlayerIndex[2] ~=-1 or PlayerIndex[3] ~=-1 or PlayerIndex[4] ~=-1 or PlayerIndex[5] ~=-1) then
  	--戰鬥中
		return;
	else
		--戰鬥失敗重設踩踏回應事件
		Char.SetPreOverEvent(nil, nil, _MeIndex);
		Char.SetPreOverEvent(nil, "Siege_Covered", _MeIndex);
		Char.SetPostOverEvent(nil, nil, _MeIndex);
		Char.SetPostOverEvent(nil, "Siege_Covered", _MeIndex);
		--戰鬥失敗重設對話回應事件
		Char.SetTalkedEvent(nil, nil, _MeIndex);
		Char.SetTalkedEvent(nil, "Siege_Talked", _MeIndex);
		--戰鬥失敗重設迴圈事件
		Char.SetLoopEvent(nil, nil, _MeIndex,1500);
		Char.SetLoopEvent(nil, "Siege_Walk", _MeIndex,2000);
		--更新
		NLG.UpChar(_MeIndex);
		return;
	end
end
