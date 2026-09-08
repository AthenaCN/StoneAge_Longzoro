function hlctrl_Reload()
	if (hlctrl_index == nil) then					--这句检查NPC是否已经创建，防止重复创建
		--创建NPC第一个参数是SiegeCtrl_index函数所在的文件第二个参数是NPC初始化时候会调用的函数
		hlctrl_index = NL.CreateNpc("./data/npc/lua/hl/hl.lua", "hlctrl_Init");
		--向NPC周围的玩家发送NPC数据更新封包
		NLG.UpChar(hlctrl_index);
		return "创建回炉NPC成功"; 
	else
		NL.DelNpc(hlctrl_index);
		NLG.UpChar(hlctrl_index);
		hlctrl_index = nil;
		return "删除回炉NPC成功";
	end
end

hlctrl_Reload();
