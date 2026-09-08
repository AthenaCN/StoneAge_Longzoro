function autopk_init( _MeIndex )
	autopk_list = {}
	autopk_list.max=0;
	autopk_list.bindex={};
	autopk_list.indexa={};
	autopk_list.indexb={};
	--設置NPC形象
	Char.SetData(_MeIndex, %对像_形象%, 100700);
	Char.SetData(_MeIndex, %对像_原形%, 100700);

	--設置NPC所在地圖
	Char.SetData(_MeIndex, %对像_地图%, 2005);

	--設置X坐標
	Char.SetData(_MeIndex, %对像_X%, 34);

	--設置Y坐標
	Char.SetData(_MeIndex, %对像_Y%, 21);

	--設置NPC方向
	Char.SetData(_MeIndex, %对像_方向%, %左下%);

	--設置NPC名稱
	Char.SetData(_MeIndex, %对像_原名%, "超级大乱斗");

	--[[
		響應說話事件，此事件如果有玩家對此NPC說話就會觸發
	--]]
	if (Char.SetTalkedEvent(nil, "autopk_talked", _MeIndex) < 0) then
		print("autopk_talked 註冊事件失敗。");
		return false;
	end

	if (Char.SetWindowTalkedEvent(nil, "autopk_windowtalked", _MeIndex) < 0) then
		print("autopk_windowtalked 註冊事件失敗。");
		return false;
	end

	return true;
end


function autopk_talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	--這裡檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then
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
	NLG.UpChar(_MeIndex);
	NLG.ShowWindowTalked(_PlayerIndex, 2, 0, 0,
 		"6\n"..
		 "\n　　　　　　　　【超级大乱斗】"..
		 "\n　　　　　　点击下方按钮加入游戏活动"..
		 "\n　　　　点击后将为您随机选取对手进行对战"..
		 "\n　　　　　注意：将自动解散您的团队。"..
 	  	 "\n"..
		 "\n　　　　　　　　【　参　加　】",
 		_MeIndex);

end


function autopk_windowtalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)
	if(_Seqno == 0)then
		if(_Data == "1")then
			NLG.TalkToCli(_TalkIndex,"请稍后，正在为您联络对手……",%黄色%);
			local battleindex = -1;
			local max = 801;
			local i,j,targetindex;
			local floor_t,floor_m;
			local targetlist = {};
			local cnt=0;
			while(battleindex < 0)do
				i = 0;
				j = 1;
--NLG.GetOnLinePlayer();
				targetlist = {};
				floor_m = Char.GetData(_MeIndex,%对像_地图%);
				while(i < max)do
					floor_t = Char.GetData( i, %对像_地图%);
					if (floor_t == floor_m )and(i ~= _TalkIndex)then
						targetlist[j] = i;
						j = j + 1;
					end
					i = i + 1;
				end
				if j == 1 then
					NLG.TalkToCli(_TalkIndex,"联络对手失败……",%黄色%);
					return;
				end
				targetindex = targetlist[math.floor(math.random(1, j-1))];
				while( targetindex == _TalkIndex)do
					targetindex = targetlist[math.floor(math.random(1, j-1))];
				end
				NLG.DischargeParty(targetindex);
				NLG.DischargeParty(_TalkIndex);
				NLG.TalkToCli(targetindex,"有玩家想要挑战您，系统为您自动解散团队。",%黄色%);
				battleindex = NLG.CreateBattlePvP(_TalkIndex, targetindex);
				if battleindex>=0 then
					break;
				end
				cnt = cnt + 1;
				if(cnt >= max)then
					NLG.TalkToCli(_TalkIndex,"联络对手失败……",%黄色%);
					break;
				end
			end
			NLG.TalkToCli(_TalkIndex,"已为您找到对手，您的对手是："..Char.GetData(targetindex,%对像_原名%).."。",%黄色%);
			NLG.TalkToCli(_TalkIndex,"现在进入决斗。",%黄色%);
			NLG.TalkToCli(targetindex,"您在超级大乱斗中被"..Char.GetData(_TalkIndex,%对像_原名%).."选中为决斗对手。",%黄色%);
			NLG.TalkToCli(targetindex,"现在进入决斗。",%黄色%);
			autopk_list.max = autopk_list.max + 1;
			autopk_list.bindex[autopk_list.max] = battleindex;
			autopk_list.indexa[autopk_list.max] = _TalkIndex;
			autopk_list.indexb[autopk_list.max] = targetindex;
			Char.SetLoopEvent(nil, "autopk_battleover", _TalkIndex,5000);
			return;
		end
	end
end

function autopk_battleover( _MeIndex)
	local i = 1;
	while(i <= autopk_list.max )do
		if(autopk_list.indexa[i] == _MeIndex)or (autopk_list.indexb[i] == _MeIndex)then
			local pindex = Battle.GetPlayIndex(autopk_list.bindex[i],0,0);
			local tindex = Battle.GetPlayIndex(autopk_list.bindex[i],1,0);
			if pindex == -1 and tindex == -1 then
				if(Char.GetData(autopk_list.indexa[i],%对像_血%)==1)and(Char.GetData(autopk_list.indexb[i],%对像_血%)>1)then
					NLG.TalkToFloor(Char.GetData(_MeIndex,%对像_地图%),Char.GetData(_MeIndex,%对像_原名%).."在超级大乱斗中惨败输给了"..Char.GetData(autopk_list.indexb[i],%对像_原名%).."。",%红色%);
					NLG.Warp(_MeIndex,2000,89,51);
				end
				if(Char.GetData(autopk_list.indexb[i],%对像_血%)==1)and(Char.GetData(autopk_list.indexa[i],%对像_血%)>1)then
					NLG.TalkToFloor(Char.GetData(_MeIndex,%对像_地图%),Char.GetData(autopk_list.indexb[i],%对像_原名%).."在超级大乱斗中惨败输给了"..Char.GetData(_MeIndex,%对像_原名%).."。",%红色%);
					NLG.Warp(autopk_list.indexb[i],2000,89,51);
				end
				autopk_list.indexa[i] = -1;
				autopk_list.indexb[i] = -1;
				autopk_list.bindex[i] = -1;
				autopk_list.max = autopk_list.max - 1;
				Char.SetLoopEvent(nil,nil,_MeIndex,1000);
			end
			return;
		end
		i = i + 1;
	end
end
