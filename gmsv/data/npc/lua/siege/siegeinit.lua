function SiegeCtrl_Reload()
	if (SiegeCtrl_index == nil) then					--這句檢查NPC是否已經創建，防止重複創建
		--創建NPC 第一個參數是 SiegeCtrl_index函數 所在的文件 第二個參數是NPC初始化時候會調用的函數
		SiegeCtrl_index = NL.CreateNpc("./data/npc/lua/siege/siegectrl.lua", "SiegeCtrl_Init");
		--向NPC周圍的玩家發送NPC數據更新封包
		NLG.UpChar(SiegeCtrl_index);
	else
		NL.DelNpc(SiegeCtrl_index);
		NLG.UpChar(SiegeCtrl_index);
		SiegeCtrl_index = nil;
	end
end

SiegeCtrl_Reload();