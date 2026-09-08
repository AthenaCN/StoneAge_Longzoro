function SiegeCtrl_Init( _MeIndex )

	--設置NPC形象
	Char.SetData(_MeIndex, %对像_形象%, 100710);
	Char.SetData(_MeIndex, %对像_原形%, 100710);

	--設置NPC所在地圖
	Char.SetData(_MeIndex, %对像_地图%, 2000);

	--設置X坐標
	Char.SetData(_MeIndex, %对像_X%, 63);

	--設置Y坐標
	Char.SetData(_MeIndex, %对像_Y%, 89);

	--設置NPC方向
	Char.SetData(_MeIndex, %对像_方向%, %右下%);

	--設置NPC名稱
	Char.SetData(_MeIndex, %对像_原名%, "逆袭控制台");

	--設置手動啟動函數
	Char.SetTalkedEvent(nil, "SiegeCtrl_Manual", _MeIndex);

	--一個全局變量-用於控制活動模式是否為激活狀態
	SiegeCtrl_Flg = 1;
	
	--迴圈事件用於檢查時間
	if (Char.SetLoopEvent(nil, "SiegeCtrl_LoopCallBack", _MeIndex, 60000) < 0) then
		print("SiegeCtrl_LoopCallBack 注册事件失败。");
		return false;
	end

	--窗口對話事件
	if (Char.SetWindowTalkedEvent(nil, "SiegeCtrl_ManualWindow", _MeIndex) < 0) then
		print("SiegeCtrl_ManualWindow 注册事件失败。");
		return false;
	end

	--統計只數
	iCounter2 = 10;

	--生成的NPC索引
	GeneratedIndex2 = {0};
	GeneratedCounter2 = 0;

	--NPC初始位置陣列
	iX2 = {0};
	iY2 = {0};

	--玩家戰鬥索引陣列
	iBattle2 = {0};

	--最後要返回操作結果
	return true;
end
function SiegeCtrl_LoopAnnounce(_MeIndex)

	--如有剩餘怪物
	if(iCounter2 > 0)then

		--公告剩餘怪物數
		local TM_Buff = "[紧急事件]还剩下" .. iCounter2 .."只怪物在"..TM_MapName[TM_RdSlMap].."作乱。请勇士们抓紧行动！";
		NLG.TalkToCli(-1, TM_Buff, %红色%, 0, -1);
		return;

	--無剩餘怪物則公告已經消滅完成
	elseif(iCounter2 == 0)then
		local TM_Buff;
		if (M_Siege_Boss_Index == nil) then					--這句檢查NPC是否已經創建，防止重複創建
			--創建NPC 第一個參數是 SiegeCtrl_index函數 所在的文件 第二個參數是NPC初始化時候會調用的函數
			M_Siege_Boss_Index = NL.CreateNpc("./data/npc/lua/siege/siege_boss.lua", "Siege_Boss_Init");
			--向NPC周圍的玩家發送NPC數據更新封包
			NLG.UpChar(M_Siege_Boss_Index);

			TM_Buff = TM_MapName[TM_RdSlMap] .. "似乎出现了更可怕的怪物。争取时间赶快去把他消灭了。";
			NLG.TalkToCli(-1, TM_Buff, %红色%, 0, -1);
			Char.SetLoopEvent(nil, nil, _MeIndex, 60000);
		else
			--公告
			TM_Buff = "[紧急事件]在"..TM_MapName[TM_RdSlMap].."作乱的怪物已经被消灭殆尽。";
			NLG.TalkToCli(-1, TM_Buff, %红色%, 0, -1);
			SiegeCtrl_KillAll(_MeIndex);
			--取消迴圈事件
			Char.SetLoopEvent(nil, nil, _MeIndex, 60000);
			--設置迴圈事件用於檢查時間
			Char.SetLoopEvent(nil, "SiegeCtrl_LoopCallBack", _MeIndex, 60000);
		end
		return;
	else

		--用於出錯情況的修正
		iCounter2 = 0;

	end
end

function SiegeCtrl_LoopCallBack( _MeIndex)

	--再獲取現在的時間(時)
	local TM_Hour = tonumber(os.date("%H", os.time()));

	--再獲取現在的時間(分)
	local TM_Minute = tonumber(os.date("%M", os.time()));

	--活動時間是19:00
	if(TM_Hour >= 20 and TM_Hour < 21 and TM_Minute <= 10) then
		--如果自動活動標識為激活且無已生成怪物
		if(SiegeCtrl_Flg == 1 and iCounter2 == 0) then
		
			--檢查是否出錯
			if(SiegeCtrl_Generate(_MeIndex)==0)then
				local TM_Buff = "[紧急事件]开启活动【怪物的逆袭】出错，请尽快联络游戏管理员，稍后将自动重试";
				NLG.TalkToCli(-1, TM_Buff, %红色%, 0, -1);
			end
			return;
		end
		return;
	else
		if(iCounter2 == 0 and SiegeCtrl_Flg == 0) then
			SiegeCtrl_Flg = 1;
			return;
		end
	end
end

function SiegeCtrl_Manual( _MeIndex, _TalkIndex, _Msg, _Color, _Channel)

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

	--檢查口令
	NLG.ShowWindowTalked(_TalkIndex, 1, 3, 0, "              【怪物的逆袭控制台】\n\n                    	请输入口令",  _MeIndex);
end

function SiegeCtrl_ManualWindow( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)

	--判定是否檢查口令窗口
	if(_Seqno == 0) then

		--判定是否按了OK
		if (_Select == 1) then

			--檢查口令
			if ( _Data == "kai") then
				NLG.TalkToCli(_TalkIndex, "手动开启口令正确。", %白色%, 0, _MeIndex);
				if(iCounter2 == 0)then
					local Tmp_cnt = SiegeCtrl_Generate( _MeIndex);
					if(Tmp_cnt > 0)then
						NLG.ShowWindowTalked(_TalkIndex, 0, 1, 1, "              【怪物的逆袭控制台】\n\n                   共生成"..Tmp_cnt.."只怪物成员",  _MeIndex);
					else
						NLG.ShowWindowTalked(_TalkIndex, 0, 1, 1, "              【怪物的逆袭控制台】\n\n                   生成怪物失败",  _MeIndex);
					end
				elseif(iCounter2 > 0)then
					NLG.ShowWindowTalked(_TalkIndex, 0, 1, 1, "              【怪物的逆袭控制台】\n\n           请先使用清除口令清除剩余怪物",  _MeIndex);
				else
					iCounter2 = 0;
					NLG.ShowWindowTalked(_TalkIndex, 0, 1, 1, "              【怪物的逆袭控制台】\n\n           未知错误，請重新尝试使用指令",  _MeIndex);
				end
			elseif(_Data == "guan") then
				NLG.TalkToCli(_TalkIndex, "清除口令正确。", %白色%, 0, _MeIndex);
				if(iCounter2 > 0)then
					local KillCounter = SiegeCtrl_KillAll( _Meindex);
					NLG.ShowWindowTalked(_TalkIndex, 0, 1, 1, "              【怪物的逆袭控制台】\n\n                   共清除"..KillCounter.."只怪物成员",  _MeIndex);
				elseif(iCounter2 == 0)then
					NLG.ShowWindowTalked(_TalkIndex, 0, 1, 1, "              【怪物的逆袭控制台】\n\n                 沒有活动中的怪物",  _MeIndex);
				else
					iCounter2 = 0;
					NLG.ShowWindowTalked(_TalkIndex, 0, 1, 1, "              【怪物的逆袭控制台】\n\n           未知错误，请重新尝试使用指令",  _MeIndex);
				end
				if(M_Siege_Boss_Index ~= nil) then
					NL.DelNpc(M_Siege_Boss_Index);
					M_Siege_Boss_Index = nil;
				end
			else
				NLG.ShowWindowTalked(_TalkIndex, 0, 1, 1, "              【怪物的逆袭控制台】\n\n                  请输入正确口令",  _MeIndex);
			end
		end
	end
end

function SiegeCtrl_KillAll( _Meindex)
	local i = GeneratedCounter2;
	while(i > 0) do
		NL.DelNpc(GeneratedIndex2[i]);
		i = i - 1;
	end
	i = GeneratedCounter2;

	GeneratedCounter2 = 0;
	GeneratedIndex2 = {0};
	iX2 = {0};
	iY2 = {0};
	iBattle2 = {0};
	iCounter2 = 0;

	if(M_Siege_Boss_Index ~= nil) then
		NL.DelNpc(M_Siege_Boss_Index);
		M_Siege_Boss_Index = nil;
	end

	return i;
end

function SiegeCtrl_Generate( _MeIndex)

	--獲取線上玩家數
	local MAX = NLG.GetOnLinePlayer();

	--產生怪物數
	local TM_Number = 15;
	local TM_Buff;
	local TM_Int;
	local TM_DIR;
	local TM_NpcIndex;

	--怪物名稱陣列
	local TM_NameList = {"邪恶怪物","邪恶怪物","邪恶怪物","邪恶怪物",
				"邪恶怪物","邪恶怪物","邪恶怪物","邪恶怪物",
					"邪恶怪物","邪恶怪物"};

	--怪物圖像編號
	local TM_Img = 100907;

	--地圖陣列
	local TM_Map = {1000,3000,4000};
	TM_MapName = {"萨姆吉尔村","加加村","卡鲁它那村"};
	local TM_X ={{71,92,57,80,98,112,77,114,57,36},{90,105,103,102,102,88,80,71,65,77},{54,70,71,55,80,91,95,96,87,38}}
	local TM_Y ={{82,81,102,122,90,103,41,43,68,61},{65,73,83,94,110,95,101,109,116,95},{61,73,92,94,51,65,70,83,64,71}}

	--生成隨機數用於選擇地圖
	TM_RdSlMap = math.floor(math.random(1, 4));
	local TM_RdSlPos = 0;

	--產生怪物
	while (TM_Number > 0) do

		--生成隨機數用於選擇位置
		TM_RdSlPos = math.floor(math.random(1, 10));
		TM_Int = math.floor(math.random(1, 10));

		--NPC的方向隨機
		TM_DIR = math.floor(math.random(0, 7));
		TM_NpcIndex = NL.CreateNpc("./data/npc/lua/siege/siege.lua", "Siege_Init");
		if (TM_NpcIndex >= 0) then
			Char.SetData(TM_NpcIndex, %对像_形象%, TM_Img);
			Char.SetData(TM_NpcIndex, %对像_原形%, TM_Img);
			Char.SetData(TM_NpcIndex, %对像_原名%, TM_NameList[TM_Int]);

			--為限定範圍設置初始座標
					--	iX2[TM_NpcIndex] = TM_X[TM_RdSlMap][TM_RdSlPos];
			iX2[TM_NpcIndex] = TM_X[TM_RdSlMap][TM_RdSlPos];
			iY2[TM_NpcIndex] = TM_Y[TM_RdSlMap][TM_RdSlPos];

			--改變NPC方向
			NLG.CharLook(TM_NpcIndex, TM_DIR);
			NLG.UpChar(TM_NpcIndex);

			--傳送NPC到指定位置
			NLG.Warp(TM_NpcIndex, TM_Map[TM_RdSlMap], TM_X[TM_RdSlMap][TM_RdSlPos], TM_Y[TM_RdSlMap][TM_RdSlPos]);

			--計數
			TM_Number = TM_Number - 1;

			--生成怪物只數統計
			iCounter2 = iCounter2 + 1;
			GeneratedCounter2 = GeneratedCounter2 + 1;
			GeneratedIndex2[GeneratedCounter2] = TM_NpcIndex;
		else

			--錯誤信息
			print(NL.GetErrorStr());
		end
	end

	--檢查結果
	if(iCounter2 > 0)then

		--全服公告
		TM_Buff ="[紧急事件]怪物暴动，怪物攻城开始，尼斯的勇士们抓紧行动吧！【" .. TM_MapName[TM_RdSlMap] .. "】！！！";
		NLG.TalkToCli(-1, TM_Buff, %红色%, 0, -1);

		--設置自動活動為未激活
		SiegeCtrl_Flg = 0;

		--取消迴圈事件
		Char.SetLoopEvent(nil, nil, _MeIndex, 60000);

		--設置迴圈事件用於公告剩餘怪物數
		Char.SetLoopEvent(nil, "SiegeCtrl_LoopAnnounce", _MeIndex, 45000);
	end

	--返回怪物只數統計
	return iCounter2;
end
