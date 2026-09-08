function MoWang_Init( _MeIndex )
	Char.SetData(_MeIndex, %对像_形象%, 104584);
	Char.SetData(_MeIndex, %对像_原形%, 104584);
	Char.SetData(_MeIndex, %对像_地图%, 2000);
	Char.SetData(_MeIndex, %对像_X%, 68);
	Char.SetData(_MeIndex, %对像_Y%, 97);
	Char.SetData(_MeIndex, %对像_方向%, %左下%);
	Char.SetData(_MeIndex, %对像_原名%, "地狱接待使");
	TM_SetNpcTime = 60; --设置冷却时间，单位：分钟

	if (Char.SetTalkedEvent(nil, "MoWang_Talked", _MeIndex) < 0) then
		print("MoWang_Talked 注册事件失败。");
		return false;
	end
	if (Char.SetWindowTalkedEvent(nil, "MoWang_WindowTalked", _MeIndex) < 0) then
		print("MoWang_WindowTalked 注册事件失败。");
		return false;
	end
	Char.SetLoopEvent(nil, "MoWang_LoopTime", _MeIndex,120000);

	return true;
end

--Talked事件
--目前最高SEQNO=13
function MoWang_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	--這裡檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then
		return ;
	end
	--local TM_Itemindex = NLG.GiveRandItem(_PlayerIndex,"29-50",60);
	--return;
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
	NLG.ShowWindowTalked(_PlayerIndex, 2, 2, 0,
		"2\n 　　　　　　　   ☆魔王挑战☆\n"..
		"\n           　   【参加魔王副本】"..
		"\n           　   【观看副本战斗】"..
		"\n          　    【减少冷却时间】"..
		"\n           　   【魔王副本介绍】",
		_MeIndex);
	Char.SetData(_PlayerIndex,%对象_LUA%,0);
	return ;
end

-- WindowTalked 事件
function MoWang_WindowTalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)

	--這裡檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_TalkIndex, _MeIndex, 1) == false) then
		return ;
	end

	local TM_ItemId = 20900;  --道具ID
	local TM_ItemName = "地狱通行证";  --道具名字
	local TM_Floor = {40001,40002,40003,40004,40005,40006,40007,40008,40009,40010};  --魔王地图名字
	local TM_CostVigor = 10; --活力点数

	local TM_ReadMe = {"功能：【介绍内容待定】","功能：【介绍内容待定】","功能：【介绍内容待定】","功能：【介绍内容待定】",""};

	if _Seqno == 0 then
		if _Data == "1" then
			NLG.ShowWindowTalked(_TalkIndex, 0, 4+8, 1,
				"　　　　　　　    ☆魔王挑战☆\n"..
				"\n进入阎王殿需要给我【"..TM_ItemName.."】"..
				"\n点击确定后继续！",
				_MeIndex);
			return;
		end

		if _Data == "2" then
			NLG.ShowWindowTalked(_TalkIndex, 1, 4+8, 2,
				" 　　　　　　　   ☆魔王挑战☆"..
				"\n请输入您要观看的楼层:" ,
				_MeIndex);
			return;
		end

		if _Data == "3" then
			local TM_MyNpcTime = Char.GetData(_TalkIndex,%对像_NPC时间%);
			if TM_MyNpcTime <= 0 then
				NLG.TalkToCli(_TalkIndex,"您的冷却时间为零，您可以直接参加魔王挑战副本！");
				return;
			end

			NLG.ShowWindowTalked(_TalkIndex, 1, 4+8, 3,
				"　　　　　　　    ☆魔王挑战☆\n"..
				"\n您现在的冷却时间为"..math.ceil(TM_MyNpcTime/60).."分钟"..
				"\n请输入您想要冷却的时间：(每分钟需要10点活力)",
				_MeIndex);
			return;
		end

		if _Data == "4" then
			Char.SetData(_TalkIndex,%对象_LUA%,1);
			local TM_button;
			if string.len(TM_ReadMe[2])<1 then
				TM_button = 1;
			else
				TM_button = 2+32;
			end
			NLG.ShowWindowTalked(_TalkIndex, 0, TM_button, 0,TM_ReadMe[1],_MeIndex);
		end

		if _Select == 32 then
			local TM_Page = Char.GetData(_TalkIndex,%对象_LUA%)+1;
			if string.len(TM_ReadMe[TM_Page])<1 then
				return;
			end
			local TM_button;
			if string.len(TM_ReadMe[TM_Page+1])<1 then
				TM_button = 2+16;
			else
				TM_button = 16+32;
			end
			NLG.ShowWindowTalked(_TalkIndex, 0, TM_button, 0,TM_ReadMe[TM_Page],_MeIndex);
			Char.SetData(_TalkIndex,%对象_LUA%,TM_Page);
		elseif _Select == 16 then
			local TM_Page = Char.GetData(_TalkIndex,%对象_LUA%)-1;
			if string.len(TM_ReadMe[TM_Page])<1 then
				return;
			end
			local TM_button;
			if string.len(TM_ReadMe[TM_Page+1])<1 then
				TM_button = 2+32;
			else
				TM_button = 16+32;
			end
			NLG.ShowWindowTalked(_TalkIndex, 0, TM_button, 0,TM_ReadMe[TM_Page],_MeIndex);
			Char.SetData(_TalkIndex,%对象_LUA%,TM_Page);
		end

	end

	if _Seqno == 1 and _Select == 4 then
		if Char.GetData(_TalkIndex,%对像_队模式%) ~= %组队_没队% then
			NLG.TalkToCli(_TalkIndex,"组队模式中不允许进入该副本！");
			return;
		end

		--if Char.FindItemId(_TalkIndex,TM_ItemId) < 1 then   开放把--弄掉
			--NLG.TalkToCli(_TalkIndex,"你没有道具【"..TM_ItemName.."】！请准备好后再来！");
			--return;
		--end

		--if Char.GetData(_TalkIndex,%对像_NPC时间%) > 0 then
			--NLG.TalkToCli(_TalkIndex,"您的冷却时间还没到，请"..math.ceil(Char.GetData(_TalkIndex,%对像_NPC时间%)/60).."分钟后再来！");
			--return;
		--end

		NLG.DelItem(_TalkIndex,TM_ItemId);
		Char.SetData(_TalkIndex,%对像_NPC时间%,60*TM_SetNpcTime);
		NLG.TalkToCli(-1,"[大陆风云]勇者<"..Char.GetData(_TalkIndex,%对像_原名%)..">闯入了阎罗殿开始寻宝。" , %红色%, 0, -1);
		NLG.Warp(_TalkIndex,40001,5,10);
	end

	if _Seqno == 2 and _Select == 4 then
		local TM_Data;
		TM_Data = tonumber(_Data);
		if TM_Data >=1 and TM_Data <=10 then
			if NLG.SearchWatchBattleRandIndex(_TalkIndex,TM_Floor[TM_Data]) < 0 then
				NLG.TalkToCli(_TalkIndex,"此地图暂时无战斗！");
				return;
			end
		end
	end

	if _Seqno == 3 and _Select == 4 then
		local TM_Data;
		TM_Data = tonumber(_Data);
		if TM_Data >=1 then
			local TM_MyCostTime = math.min(TM_Data,math.ceil(Char.GetData(_TalkIndex,%对像_NPC时间%)/60));
			if TM_MyCostTime <= 0 then
				NLG.TalkToCli(_TalkIndex,"您的冷却时间为零，直接可以参加魔王副本！");
				return;
			end
			if Char.GetData(_TalkIndex,%对像_声望%) < TM_MyCostTime*TM_CostVigor then
				NLG.TalkToCli(_TalkIndex,"您的活力不够，减少冷却时间每分钟需要"..TM_CostVigor.."活力！");
				return;
			end
			Char.SetData(_TalkIndex,%对像_声望%,Char.GetData(_TalkIndex,%对像_声望%)-TM_MyCostTime*TM_CostVigor);
			local TM_MyNowNpcTime = math.max(0,Char.GetData(_TalkIndex,%对像_NPC时间%) - TM_MyCostTime*60);
			Char.SetData(_TalkIndex,%对像_NPC时间%,TM_MyNowNpcTime);
			NLG.TalkToCli(_TalkIndex,"扣除"..TM_MyCostTime*TM_CostVigor.."活力！");
			NLG.TalkToCli(_TalkIndex,"您已成功减少冷却时间"..TM_MyCostTime.."分钟！");
			return;
		end
	end

end

function MoWang_LoopTime( _MeIndex )
	local TM_PlayNum = NLG.GetMaxPlayNum()-1;
	local TM_i;
	for i=0,TM_PlayNum do
		if NLG.CheckPlayIndex(i) == i then
			local TM_MyTime = Char.GetData(i,%对像_NPC时间%);
			if TM_MyTime > TM_SetNpcTime*60 then
				if TM_MyTime > TM_SetNpcTime*60 then
					Char.SetData(i,%对像_NPC时间%,0);
				end
			elseif TM_MyTime > 0 then
				Char.SetData(i,%对像_NPC时间%,math.max(0,(TM_MyTime - 60)));
			end
		end
	end
end