function MoWangBoss_Init( _MeIndex )
	local TM_Floor = {40001,40002,40003,40004,40005,40006,40007,40008,40009,40010,40011};
	local TM_X = {5,5,5,5,5,5,5,5,5,5,5};
	local TM_Y = {4,4,4,4,4,4,4,4,4,4,4};
	local TM_Base = {100000,100000,100000,100000,100000,100000,100000,100000,100000,100000,100000};
	local TM_Name = {"一殿秦广王","二殿楚江王","三殿宋帝王","四殿五官王","五殿阎罗王","六殿卞城王","七殿泰山王","八殿都市王","九殿平等王","十殿转轮王","地藏菩萨"};
	Char.SetData(_MeIndex,%对像_地图%,TM_Floor[NPC_I]);
	Char.SetData(_MeIndex,%对像_X%,TM_X[NPC_I]);
	Char.SetData(_MeIndex,%对像_Y%,TM_Y[NPC_I]);
	Char.SetData(_MeIndex, %对像_方向%, %右下%);
	Char.SetData(_MeIndex, %对像_原形%, TM_Base[NPC_I]);
	Char.SetData(_MeIndex, %对像_形象%, TM_Base[NPC_I]);
	Char.SetData(_MeIndex, %对像_原名%, TM_Name[NPC_I]);
	--戰鬥索引全域變數
	iBattle2[_MeIndex] = -1;	

	--設置對話回應事件
	if (Char.SetTalkedEvent(nil, "MoWangBoss_Talked", _MeIndex) < 0) then
		print("MoWangBoss_Talked 注册事件失败。");
		return false;
	end
	if (Char.SetWindowTalkedEvent(nil, "MoWangBoss_WindowTalked", _MeIndex) < 0) then
		print("MoWangBoss_WindowTalked 注册事件失败。");
		return false;
	end
	--更新
	return true;
end

function MoWangBoss_WindowTalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)

	--這裡檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_TalkIndex, _MeIndex, 1) == false) then
		return ;
	end
	
	if _Seqno == 0 then
	
		if Char.GetData(_MeIndex,%对像_地图%) == 40001 then
			if _Data == "2" then
				_Data = "3";
			end
		elseif Char.GetData(_MeIndex,%对像_地图%) == 40006 then
			if _Data == "1" then
				_Data = "2";
			elseif _Data == "2" then
				_Data = "3";
			end
		end
		
		if _Data == "1" then
			--計算挑戰者隊伍人數及隊伍平均等級以確定怪物強度
			local cnt = 1;
			local PlayerNumber = 1 ;
			PlayerALV = Char.GetData(_TalkIndex,%对像_等级%);
			local TMP_Index;
			while (cnt < 5) do
				TMP_Index = Char.GetTeamIndex(_TalkIndex,cnt);
				if (TMP_Index > 0) then
					PlayerNumber = PlayerNumber + 1;
					PlayerALV = PlayerALV + Char.GetData(TMP_Index,%对像_等级%);
				end
				cnt = cnt + 1;
			end
			PlayerALV = PlayerALV/PlayerNumber;
			NLG.TalkToCli(_TalkIndex, "呼噜呼噜噜…………", %红色%, 0, _MeIndex);

			--敵人陣列信息
			local TM_EnemyIdAr;
			local TM_EnemyLv;
			local TM_EnemyNum = {2,4,6,8,10};
			if(PlayerNumber==1)then
				TM_EnemyIdAr={{34000,34014},
											{34001,34013},
											{34002,34012},
											{34003,34011},
											{34004,34011},
											{1,1},
											{1,1},
											{1,1},
											{1,1},
											{1,1}};
				TM_EnemyLv =  {{121,121},
											{131,131},
											{141,141},
											{151,151},
											{158,158},
											{168,168},
											{169,169},
											{170,170},
											{180,180},
											{199,199}};
			elseif(PlayerNumber==2)then
				TM_EnemyIdAr={{34000,34014,34013,34012},
											{34001,34013,34012,34011},
											{34002,34014,34012,34011},
											{34003,34014,34013,34011},
											{34004,34014,34011,34011},
											{1,1,1,1},
											{1,1,1,1},
											{1,1,1,1},
											{1,1,1,1},
											{1,1,1,1}};
				TM_EnemyLv =  {{121,121,121,121},
											{131,131,131,131},
											{141,141,141,141},
											{151,151,151,151},
											{158,158,158,158},
											{168,168,168,168},
											{169,169,169,169},
											{170,170,170,170},
											{180,180,180,180},
											{199,199,199,199}};
			elseif(PlayerNumber==3)then
				TM_EnemyIdAr={{34000,34011,34012,34013,34014,34013},
											{34001,34011,34012,34013,34014,34011},
											{34002,34011,34012,34013,34014,34012},
											{34003,34011,34012,34013,34014,34013},
											{34004,34011,34012,34013,34014,34014},
											{1,1,1,1,1,1},
											{1,1,1,1,1,1},
											{1,1,1,1,1,1},
											{1,1,1,1,1,1},
											{1,1,1,1,1,1}};
				TM_EnemyLv =  {{121,121,121,121,121,121},
											{131,131,131,131,131,131},
											{141,141,141,141,141,141},
											{151,151,151,151,151,151},
											{158,158,158,158,158,158},
											{168,168,168,168,168,168},
											{169,169,169,169,169,169},
											{170,170,170,170,170,170},
											{180,180,180,180,180,180},
											{199,199,199,199,199,199}};
			elseif(PlayerNumber==4)then
				TM_EnemyIdAr={{34000,34011,34012,34013,34014,34014,34013,34012},
											{34001,34011,34012,34013,34014,34013,34012,34011},
											{34002,34011,34012,34013,34014,34014,34012,34011},
											{34003,34011,34012,34013,34014,34014,34013,34011},
											{34004,34011,34012,34013,34014,34014,34011,34011},
											{1,1,1,1,1,1,1,1},
											{1,1,1,1,1,1,1,1},
											{1,1,1,1,1,1,1,1},
											{1,1,1,1,1,1,1,1},
											{1,1,1,1,1,1,1,1}};
				TM_EnemyLv =  {{121,121,121,121,121,121,121,121},
											{131,131,131,131,131,131,131,131},
											{141,141,141,141,141,141,141,141},
											{151,151,151,151,151,151,151,151},
											{158,158,158,158,158,158,158,158},
											{168,168,168,168,168,168,168,168},
											{169,169,169,169,169,169,169,169},
											{170,170,170,170,170,170,170,170},
											{180,180,180,180,180,180,180,180},
											{199,199,199,199,199,199,199,199}};
			else
				TM_EnemyIdAr={{34000,34011,34012,34013,34014,34011,34012,34013,34014,34013},
											{34001,34011,34012,34013,34014,34011,34012,34013,34014,34013},
											{34002,34011,34012,34013,34014,34011,34012,34013,34014,34012},
											{34003,34011,34012,34013,34014,34011,34012,34013,34014,34014},
											{34004,34011,34012,34013,34014,34011,34012,34013,34014,34011},
											{1,1,1,1,1,1,1,1,1,1},
											{1,1,1,1,1,1,1,1,1,1},
											{1,1,1,1,1,1,1,1,1,1},
											{1,1,1,1,1,1,1,1,1,1},
											{1,1,1,1,1,1,1,1,1,1}};
				TM_EnemyLv =  {{121,121,121,121,121,121,121,121,121,121},
											{131,131,131,131,131,131,131,131,131,131},
											{141,141,141,141,141,141,141,141,141,141},
											{151,151,151,151,151,151,151,151,151,151},
											{158,158,158,158,158,158,158,158,158,158},
											{168,168,168,168,168,168,168,168,168,168},
											{169,169,169,169,169,169,169,169,169,169},
											{170,170,170,170,170,170,170,170,170,170},
											{180,180,180,180,180,180,180,180,180,180},
											{199,199,199,199,199,199,199,199,199,199}};
			end
			local npcfloor = {40001,40002,40003,40004,40005,40006,40007,40008,40009,40010};
			local Tmp_Int=1;
			while(Tmp_Int<=10)do
				if Char.GetData(_MeIndex,%对像_地图%) == npcfloor[Tmp_Int] then
					break;
				end
				Tmp_Int = Tmp_Int+1;
			end
			if Tmp_Int > 10 then
				return;
			end
			--保存戰鬥索引
			iBattle2[_MeIndex] = NLG.CreateBattle( _TalkIndex, _MeIndex, nil, TM_EnemyIdAr[Tmp_Int], TM_EnemyLv[Tmp_Int], nil); 
			--print("time="..iBattleTime[_MeIndex]);
			--設置戰鬥勝利事件
			Battle.SetWinEvent("data/npc/lua/MoWang/MoWangBoss.lua","MoWangBoss_BattleOver",iBattle2[_MeIndex]);
			--取消對話回應事件并設置新對話回應事件
			--Char.SetTalkedEvent(nil, nil, _MeIndex);
			--Char.SetTalkedEvent(nil, "MoWangBoss_Talked2", _MeIndex);
			--取消迴圈事件并設置新迴圈事件(用作檢查戰鬥是否結束)
			--Char.SetLoopEvent(nil, nil, _MeIndex,121);
			--Char.SetLoopEvent(nil, "MoWangBoss_Check", _MeIndex,500);
			--更新
			NLG.UpChar(_MeIndex);
			return;
		end
		
		if _Data == "2" then
			if Char.GetData(_TalkIndex,%对像_队模式%) ~= %组队_没队% then
				NLG.TalkToCli(_TalkIndex,"组队模式中不允许领取奖励！");
				return;
			end
			
			local TM_ItemId = {"14031-14090,14331-14390,14631-14690,14931-14990,15231-15290,15531-15590,15831-15890,16131-16190,16431-16490,16731-16790,17051-17150,17551-17650,14091,14391,14691,14991,15291,15591,15891,16191,16491,16791,17151,17651,18001-18030,21000-21006,20810,21029",
												 "14061-14120,14361-14420,14661-14720,14961-15020,15261-15320,15561-15620,15861-15920,16161-16220,16461-16520,16761-16820,17101-17200,17601-17700,14121,14421,14721,15021,15321,15621,15921,16221,16521,16821,17201,17701,18031-18060,21004-21008,20810,21023-21026,21032-21035,21029,21029",
												 "14091-14150,14391-14450,14691-14750,14991-15050,15291-15350,15591-15650,15891-15950,16191-16250,16491-16550,16791-16850,17151-17250,17651-17750,14151,14451,14751,15051,15351,15651,15951,16251,16551,16851,17251,17751,18061-18075,21004-21010,20810,21023-21026,21032-21035,21029,21029",
												 "14151-14210,14451-14510,14751-14810,15051-15110,15351-15410,15651-15710,15951-16010,16251-16310,16551-16610,16851-16910,17251-17350,17751-17850,14181,14481,14781,15081,15381,15681,15981,16281,16581,16881,17301,17801,18076-18090,21008-21011,20810,21023-21026,21032-21035,21032-21035,21029,21029",
												 "14181-14240,14481-14540,14781-14840,15081-15140,15381-15440,15681-15740,15981-16040,16281-16340,16581-16640,16881-16940,17301-17400,17801-17900,14211,14511,14811,15111,15411,15711,16011,16311,16611,16911,17351,17851,18091-18105,21010-21011,20713-20716,21023-21026,21023-21026,21032-21035,21032-21035,21029",
												 "1,1,1,1,1,1,1,1,1,1",
												 "1,1,1,1,1,1,1,1,1,1",
												 "1,1,1,1,1,1,1,1,1,1",
												 "1,1,1,1,1,1,1,1,1,1",
												 "1,1,1,1,1,1,1,1,1,1"};
			local TM_MyFloor = {40002,40003,40004,40005,40006,40007,40008,40009,40010,40011};
			local TMP_I=1;
			while(TMP_I<=10)do
				if(Char.GetData(_TalkIndex,%对像_地图%) == TM_MyFloor[TMP_I]) then
					break;
				else
					TMP_I = TMP_I+1;
				end
			end
			if (TMP_I>10) then
				return;
			end
			local TM_CengNum = {"一","二","三","四","五","六","七","八","九","十"};
			local TM_MyItemIndex = NLG.GiveRandItem(_TalkIndex,TM_ItemId[TMP_I]);
			if(TM_MyItemIndex>=0)then
				NLG.TalkToCli(-1,"[喜报来也]勇者<"..Char.GetData(_TalkIndex,%对像_原名%)..">从 阎罗"..TM_CengNum[TMP_I].."殿 带回物品<"..Item.GetData(TM_MyItemIndex,%道具_原名%)..">。" , %红色%, 0, -1);
			end
			NLG.Warp(_TalkIndex,2000,42,85);
			return;
		end
		
		if _Data == "3" then
			Char.HealAll(_TalkIndex);
			NLG.TalkToCli(_TalkIndex, "您已经完全恢复，再接再厉哦！", %黄色%, 0, _MeIndex);
			return;
		end
	end
end

function MoWangBoss_BattleOver( _BattleIndex, _CreateIndex)
	--取消檢查戰鬥是否結束的迴圈事件
	Char.SetLoopEvent(nil, nil, _CreateIndex,1000);
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
	local i = 0;
	local nowtime = tonumber(os.time());
	local herofloor = {1,20,40,60,80,121,131,115,141,132};
	local npcfloor = {40001,40002,40003,40004,40005,40006,40007,40008,40009,40010,40011};
	local Tmp_Int=1;
	while(Tmp_Int<=10)do
		if Char.GetData(_CreateIndex,%对像_地图%) == npcfloor[Tmp_Int] then
			break;
		end
		Tmp_Int = Tmp_Int+1;
	end
	if Tmp_Int > 10 then
		return;
	end
	
	--獲得戰鬥中的玩家索引
	local TM_CengNum = {"一","二","三","四","五","六","七","八","九","十"};
	while (count < 6) do
		PlayerIndex[count] = Battle.GetPlayIndex(_BattleIndex,0,count-1);
		if(PlayerIndex[count] >= 0) then
			--[[	此處對PlayerIndex[count]設置獎勵
				(math.random(1, 4)中4為最大獎品數量
				即PrizeList中的項數	]]--
			if(Char.GetData(PlayerIndex[count],%对像_战层%)<herofloor[Tmp_Int]) then
				Char.SetData(PlayerIndex[count],%对像_战层%,herofloor[Tmp_Int]);
			end
			--if( PrizeList[Tmp_Int] >= 0) then
			--	NLG.GiveItem(PlayerIndex[count],PrizeList[Tmp_Int]);
			--end
			NLG.TalkToCli(PlayerIndex[count], "呼噜噜噜噜噜…………(似乎是打输了很不甘愿的样子…)", %红色%, 0, _CreateIndex);
			NLG.DischargeParty(PlayerIndex[count]);
			NLG.Warp(PlayerIndex[count],npcfloor[Tmp_Int+1],5,10);
			local Tmp_Buff;
			Tmp_Buff = "[地狱来报]勇者<"..Char.GetData(PlayerIndex[count],%对像_原名%)..">挑战阎罗"..TM_CengNum[Tmp_Int].."殿成功。";
			NLG.TalkToCli(-1,Tmp_Buff , %红色%, 0, -1);
		else
			print(NL.GetErrorStr());
		end
		count = count + 1 ;
	end

	--刪除NPC
	--local Tmp_Buff;
	--if( i == 1) then
	--	Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."打倒了一只攻城怪物。";
	--	NLG.TalkToCli(-1,Tmp_Buff , %红色%, 0, -1);
	--elseif( i == 2) then
	--	Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."、"..Char.GetData(PlayerIndex[2],%对像_原名%).."打倒了一只攻城怪物。";
	--	NLG.TalkToCli(-1,Tmp_Buff , %红色%, 0, -1);
	--elseif( i == 3) then
	--	Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."、"..Char.GetData(PlayerIndex[2],%对像_原名%).."、"..Char.GetData(PlayerIndex[3],%对像_原名%).."打倒了一只攻城怪物。";
	--	NLG.TalkToCli(-1, Tmp_Buff, %红色%, 0, -1);
	--elseif( i == 4) then
	--	Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."、"..Char.GetData(PlayerIndex[2],%对像_原名%).."、"..Char.GetData(PlayerIndex[3],%对像_原名%).."、"..Char.GetData(PlayerIndex[4],%对像_原名%).."打倒了一只攻城怪物。";
	--	NLG.TalkToCli(-1, Tmp_Buff, %红色%, 0, -1);
	--elseif( i == 5) then
	--	Tmp_Buff = "[怪物攻城事件]勇士"..Char.GetData(PlayerIndex[1],%对像_原名%).."、"..Char.GetData(PlayerIndex[2],%对像_原名%).."、"..Char.GetData(PlayerIndex[3],%对像_原名%).."、"..Char.GetData(PlayerIndex[4],%对像_原名%).."、"..Char.GetData(PlayerIndex[5],%对像_原名%).."打倒了一只攻城怪物。";
	--	NLG.TalkToCli(-1, Tmp_Buff, %红色%, 0, -1);
	--end
	--取消對話回應事件
	--Char.SetTalkedEvent(nil, nil, _CreateIndex);
	--Char.SetTalkedEvent(nil, "MoWangBoss_Talked", _CreateIndex);
	iBattle2[_CreateIndex] = -1;
	return;
end

function MoWangBoss_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	--檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 2) == false) then
		return ;
	end 
	
	if(Char.GetData(_MeIndex,%对像_地图%)==40001) then
		NLG.ShowWindowTalked(_PlayerIndex, 2, 2, 0,
		"2\n 　　　　　　　   ☆挑战阎王☆\n"..
		"\n           　   【开始挑战阎王】"..
		"\n          　    【帮我恢复体力】",
		_MeIndex);
	elseif(Char.GetData(_MeIndex,%对像_地图%)==40006) then
		NLG.ShowWindowTalked(_PlayerIndex, 2, 2, 0,
		"2\n 　　　　　　☆目前只开放前五层☆\n"..
		"\n           　   【领取奖励走人】"..
		"\n          　    【帮我恢复体力】",
		_MeIndex);
	else
		NLG.ShowWindowTalked(_PlayerIndex, 2, 2, 0,
		"2\n 　　　　　　　   ☆挑战阎王☆\n"..
		"\n           　   【继续挑战阎王】"..
		"\n           　   【领取奖励走人】"..
		"\n          　    【帮我恢复体力】",
		_MeIndex);
	end
	Char.SetData(_PlayerIndex,%对象_LUA%,0);
	return ;
	
end

function MoWangBoss_Talked2( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	--檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then
		return ;
	end 
	--正在戰鬥中的對話事件
	NLG.TalkToCli(_PlayerIndex, "呼噜噜噜～～～！", %红色%, 0, _MeIndex);
	NLG.TalkToCli(_PlayerIndex, Char.GetData( _PlayerIndex,%对像_原名%) .. "：(似乎已经有人在和这个异界生物战斗了，我想我还是去看看有没有别只异界生物在作乱吧。)", _Color, 0);
	return ;
end

function MoWangBoss_Check( _MeIndex)
	--此為檢查戰鬥是否結束的函數
--	print("ING");
	local PlayerIndex = {-1,-1,-1,-1,-1};
	local count = 1;
	--獲取戰鬥中玩家索引
	if iBattle2[_MeIndex] == nil then
		Char.SetLoopEvent(nil, nil, _MeIndex,1000);
	end
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
		--戰鬥失敗重設對話回應事件
		Char.SetTalkedEvent(nil, nil, _MeIndex);
		Char.SetTalkedEvent(nil, "MoWangBoss_Talked", _MeIndex);
		--更新
		NLG.UpChar(_MeIndex);
		return;
	end
end
