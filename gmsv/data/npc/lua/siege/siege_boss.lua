--[[
	LNSVer = 1.1.26
--]]
--[[
	函数命名规则
	脚本系列名称_NPC名称_函数功能相关名称(例如某NPC的初试化函数 Sports_ArenaManager_Init)

	变量命名规则
	TM_ 开头的为局部变量
	TM_变量作用相关的名称(例如 用左记录某个索引的局部变量  TM_Index)
	M_ 开头的为全局变量
	M_脚本系列名称_变量作用相关的名称(例如 用左记录某个索引的全变量  M_Sports_Index)
	
	脚本编写规范
	1.同一脚本系列的所有全局变量应该集中在这个脚本系列的某个文件里面，并有相应的注解。
	2.变量和函数的声明必须遵循上面提及的命名规则并防止全局变量名和函数名重复(包括不同脚本之间的全局变量名和函数)
	3.能使用常量的情况下尽量使用常量(LNS拥有独特的常量系统使用常量并不会导致脚本性能下降)
--]]

function Siege_Boss_Init( _MeIndex )
	local TM_X ={{71,92,57,80,98,112,77,114,57,36},{90,105,103,102,102,88,80,71,65,77},{54,70,71,55,80,91,95,96,87,38}};
	local TM_Y ={{82,81,102,122,90,103,41,43,68,61},{65,73,83,94,110,95,101,109,116,95},{61,73,92,94,51,65,70,83,64,71}};
	local TM_Map = {1000,3000,4000};

	M_Siege_Boss_AwardFlg = true;

	--設置NPC形象
	Char.SetData(_MeIndex, %对像_形象%, 100907);
	Char.SetData(_MeIndex, %对像_原形%, 100907);

	local TM_RdSlPos = math.floor(math.random(1, 10));
	
	--設置NPC所在地圖
	Char.SetData(_MeIndex, %对像_地图%, TM_Map[TM_RdSlMap]);

	--設置X坐標
	Char.SetData(_MeIndex, %对像_X%, TM_X[TM_RdSlMap][TM_RdSlPos]);

	--設置Y坐標
	Char.SetData(_MeIndex, %对像_Y%, TM_Y[TM_RdSlMap][TM_RdSlPos]);

	--設置NPC方向
	Char.SetData(_MeIndex, %对像_方向%, %右下%);

	--設置NPC名稱
	Char.SetData(_MeIndex, %对像_原名%, "怪物首领");
	
	if(Char.SetTalkedEvent(nil, "Siege_Boss_TalkedCallBack", _MeIndex) < 0) then
		print("Siege_Boss_TalkedCallBack 注册事件失败。");
		return false;
	end
	NLG.TalkToCli(-1, "怪物首领在" .. TM_MapName[TM_RdSlMap] .. "做最后挣扎，勇士们将他送回老家吧！他身上有宝物哦！", %红色%, 0, -1);
	return true;
end

function Siege_Boss_TalkedCallBack( _MeIndex, _TalkIndex, _Msg, _Color, _Channel)
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
	
	-- 怪物 最多设置10只
	local TM_EnemyIdAr = {1690,1690,2535,2535,2540,2541,2542,2543,2544,2545};
	local TM_SkillType = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	local TM_BattleIndex = NLG.CreateBattle(_TalkIndex, _MeIndex, nil, TM_EnemyIdAr, nil, TM_SkillType);
	if(TM_BattleIndex < 0) then
		print("Siege_Boss_TalkedCallBack 创建战斗失败。");
		return ;
	end
	Battle.SetWinEvent(nil, "Siege_Boss_BattleOver", TM_BattleIndex);

	local TM_PlayIndex = _TalkIndex;

	NLG.TalkToCli(TM_PlayIndex, "呼噜呼噜呼噜噜…………", %红色%, 0, _MeIndex);

	TM_PlayIndex = Char.GetTeamIndex(_TalkIndex, %队员_1%);
	if (TM_PlayIndex > 0) then
		NLG.TalkToCli(TM_PlayIndex, "呼噜呼噜呼噜噜…………", %红色%, 0, _MeIndex);
	end

	TM_PlayIndex = Char.GetTeamIndex(_TalkIndex, %队员_2%);
	if (TM_PlayIndex > 0) then
		NLG.TalkToCli(TM_PlayIndex, "呼噜呼噜呼噜噜…………", %红色%, 0, _MeIndex);
	end

	TM_PlayIndex = Char.GetTeamIndex(_TalkIndex, %队员_3%);
	if (TM_PlayIndex > 0) then
		NLG.TalkToCli(TM_PlayIndex, "呼噜呼噜呼噜噜…………", %红色%, 0, _MeIndex);
	end
	
	TM_PlayIndex = Char.GetTeamIndex(_TalkIndex, %队员_4%);
	if (TM_PlayIndex > 0) then
		NLG.TalkToCli(TM_PlayIndex, "呼噜呼噜呼噜噜…………", %红色%, 0, _MeIndex);
	end
	
	return ;
end

function Siege_Boss_BattleOver( _BattleIndex, _CreateIndex)
	local TM_PrizeList = {22932,22933,22934,22935,22936,33239,33248,33249,33250,33251,33252,22937,22938,22939,22940,22941,22942,22943,22944,22945,22946,22947,22948,22949,22950,22951,22952,22953,22954,22955,22956,22957,22958,22959,22960,22961,22962,22963,22964,22965,22966,22967};			-- 奖品列表(道具 ID)可多个
	local TM_PlayerIndex;
	local TM_cnt = 1;
	if(M_Siege_Boss_AwardFlg == true) then
		for i=1,5 do
			TM_PlayerIndex = Battle.GetPlayIndex(_BattleIndex, 0, i-1);
			if(TM_PlayerIndex >= 0) then
				-- 发奖品
				for _,e in ipairs(TM_PrizeList) do
					NLG.GiveItem(TM_PlayerIndex, e);
				end
				NLG.TalkToCli(-1, Char.GetData(TM_PlayerIndex, %对像_原名%) .. "成功击倒" .. Char.GetData(_CreateIndex, %对像_原名%) .. "本次攻城活动结束！未获得奖品的请等下次~~谢谢支持！", %红色%, 0, -1)
				
			end
		end
		--清除控制台數據
		SiegeCtrl_KillAll(SiegeCtrl_index);
		--取消迴圈事件
		Char.SetLoopEvent(nil, nil, SiegeCtrl_index, 60000);
		--設置迴圈事件用於檢查時間
		Char.SetLoopEvent(nil, "SiegeCtrl_LoopCallBack", SiegeCtrl_index, 60000);
		M_Siege_Boss_AwardFlg = false;
	else
		for i=1,5 do
			TM_PlayerIndex = Battle.GetPlayIndex(_BattleIndex, 0, i-1);
			if(TM_PlayerIndex >= 0) then
				NLG.TalkToCli(TM_PlayerIndex, "可恶！居然接二连三的被打败，好不甘心...", %红色%, 0, _CreateIndex);
			end
		end
	end
	return ;
end
