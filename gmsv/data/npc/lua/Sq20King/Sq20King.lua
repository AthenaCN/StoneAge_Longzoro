function Sq20King_Init( _MeIndex )
	Char.SetData(_MeIndex,%对像_原形%,104470);
	Char.SetData(_MeIndex,%对像_形象%,104470);
	Char.SetData(_MeIndex,%对像_地图%,2000);
	Char.SetData(_MeIndex,%对像_X%,53);
	Char.SetData(_MeIndex,%对像_Y%,78);
	Char.SetData(_MeIndex,%对像_方向%,%右下%);
	Char.SetData(_MeIndex,%对像_原名%,"神奇大魔王");

	if(Char.SetTalkedEvent(nil, "Sq20King_TalkedCallBack", _MeIndex) < 0) then
		print("Sq20King_TalkedCallBack 注册事件失败。");
		return false;
	end

--全局变量
    starttime=nil;
    endtime=nil;
    Sq20King_BattleIndex=-1;
	return true;
end

function Sq20King_TalkedCallBack( _MeIndex, _TalkIndex, _Msg, _Color, _Channel)
	--檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_TalkIndex, _MeIndex, 1) == false) then
		return ;
	end
--面向玩家
	local i;
	i = Char.GetData(_TalkIndex, %对像_方向%);
	if i >= 4 then
		i = i - 4;
	else
		i = i + 4;
	end
	Char.SetData(_MeIndex, %对像_方向%,i);
	NLG.UpChar( _MeIndex);
	if(Sq20King_BattleIndex >=0 ) then
		NLG.ShowWindowTalked(_TalkIndex,0,4,0,
		       "                  『神奇大魔王』"..
		       "\n"..
			   "\n没看到哥我正在忙着吗？"..
		       "\n别着急，一会儿杀了他们，就轮到你们了。",
			   _MeIndex);
		return;
	end

	-- 怪物 最多设置10只
	local TM_EnemyIdAr = {20301,20302,20303,20304,20305,20306,20307,20308,20309,20310};
	local TM_BaseLevel = {140, 140, 140, 140, 140,140, 140, 140, 140, 140};
	--local TM_SkillType = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	Sq20King_BattleIndex = NLG.CreateBattle(_TalkIndex, _MeIndex, nil, TM_EnemyIdAr, TM_BaseLevel, nil);

	if(Sq20King_BattleIndex < 0) then--判断是否成功创建战斗
		return ;
	end

	Battle.SetWinEvent(nil, "Sq20King_BattleOver", Sq20King_BattleIndex);
	Char.SetLoopEvent(nil,"Sq20King_Battle_Check",_MeIndex,2000);
	starttime=os.time();--战斗计时开始

	local TM_PlayIndex = _TalkIndex;
	    NLG.TalkToCli(TM_PlayIndex, "就凭你也想打败我吗？", %青色%, 0, _MeIndex);

	TM_PlayIndex = Char.GetTeamIndex(_TalkIndex, %队员_1%);
	if (TM_PlayIndex > 0) then
		NLG.TalkToCli(TM_PlayIndex, "就凭你也想打败我吗？", %青色%, 0, _MeIndex);
	end

	TM_PlayIndex = Char.GetTeamIndex(_TalkIndex, %队员_2%);
	if (TM_PlayIndex > 0) then
		NLG.TalkToCli(TM_PlayIndex, "就凭你也想打败我吗？", %青色%, 0, _MeIndex);
	end

	TM_PlayIndex = Char.GetTeamIndex(_TalkIndex, %队员_3%);
	if (TM_PlayIndex > 0) then
		NLG.TalkToCli(TM_PlayIndex, "就凭你也想打败我吗？", %青色%, 0, _MeIndex);
	end

	TM_PlayIndex = Char.GetTeamIndex(_TalkIndex, %队员_4%);
	if (TM_PlayIndex > 0) then
		NLG.TalkToCli(TM_PlayIndex, "就凭你也想打败我吗？", %青色%, 0, _MeIndex);
	end

	return ;
end

function Sq20King_BattleOver( _BattleIndex, _MeIndex)
	endtime=os.time();--战斗时间计算结束

	local TM_PrizeList = {20566};-- 奖品列表(道具 ID)可以设置多个
	local TM_PlayerIndex;
	local TM_cnt = 1;

	for i=1,5 do
		TM_PlayerIndex = Battle.GetPlayIndex(_BattleIndex, 0, i-1);
		if(TM_PlayerIndex >= 0) then
         local name = Char.GetData(TM_PlayerIndex,%对像_原名%);
	if(endtime-starttime > 1)then --这里来判断战斗中总所使用的时间可以防止快速战斗...(sq20:如果外挂太过于智能的，此方案无效)
		NLG.TalkToCli(TM_PlayerIndex,"怎么可能，我就这样失败了吗？我不信。", %红色%, 0, _MeIndex);
		NLG.TalkToCli(-1,"英勇的 "..name .." 打败了神奇大魔王。", %绿色%, 0, -1);
	else
		NLG.TalkToCli(TM_PlayerIndex,"艹，开快速就算你赢了也不会给你任何奖励。", %红色%, 0, _MeIndex);
		NLG.TalkToCli(-1,"英勇的 "..name .." 打败了打败的神奇大魔王。", %绿色%, 0, -1);
		return false;
	end
			for _,e in ipairs(TM_PrizeList) do
			  if NLG.GiveItem(TM_PlayerIndex, e) <= 0 then
			  NLG.TalkToCli(TM_PlayerIndex, "道具栏位不足!", %红色%, 0, -1);
			  end
			end
		end
	end

		--戰鬥胜利重設對話回應事件
		Char.SetTalkedEvent(nil, nil, _MeIndex);
		Char.SetTalkedEvent(nil, "Sq20King_TalkedCallBack", _MeIndex);
		--戰鬥胜利重設迴圈事件
		Char.SetLoopEvent(nil, nil, _MeIndex,1500);
		Sq20King_BattleIndex=-1;
		starttime=nil;
		endtime=nil;
	return ;
end

function Sq20King_Battle_Check(_MeIndex)
	--此為檢查戰鬥是否結束的函數
	local PlayerIndex = {-1,-1,-1,-1,-1};
	local count = 1;
	--獲取戰鬥中玩家索引
	while (count < 6) do
		PlayerIndex[count] = Battle.GetPlayIndex(Sq20King_BattleIndex,0,count-1);
		count = count + 1 ;
	end
	--如玩家索引不全為零則戰鬥尚未結束
	--否則判定戰鬥結束
	--因為如果戰鬥勝利
	--戰鬥勝利事件中取消了此迴圈事件
	--所以在此可判定戰鬥失敗
	if(PlayerIndex[1] ~=-1 or PlayerIndex[2] ~=-1 or PlayerIndex[3] ~=-1 or PlayerIndex[4] ~=-1 or PlayerIndex[5] ~=-1) then
		return;
	else
		--戰鬥失敗重設回應事件
		--戰鬥失敗重設對話回應事件
		Char.SetTalkedEvent(nil, nil, _MeIndex);
		Char.SetTalkedEvent(nil, "Sq20King_TalkedCallBack", _MeIndex);
		--戰鬥失敗重設迴圈事件
		Char.SetLoopEvent(nil, nil, _MeIndex,1500);
		Sq20King_BattleIndex=-1;
		starttime=nil;
		endtime=nil;
		NLG.TalkToCli(-1,"勇者被神奇大魔王击败了...世界又一次笼罩在神奇大魔王的阴影下。", %紫色%, 0, -1);
		return;
	end
end

