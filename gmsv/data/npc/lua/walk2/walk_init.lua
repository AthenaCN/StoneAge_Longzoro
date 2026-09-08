function walkctrl_Reload()
	if (walkctrl_index == nil) then					--这句检查NPC是否已经创建，防止重复创建
		--创建NPC第一个参数是SiegeCtrl_index函数所在的文件第二个参数是NPC初始化时候会调用的函数
		walkctrl_index = NL.CreateNpc("./data/npc/lua/walk2/walk.lua", "walkctrl_Init");
		--向NPC周围的玩家发送NPC数据更新封包
		NLG.UpChar(walkctrl_index);
	else
		NL.DelNpc(walkctrl_index);
		NLG.UpChar(walkctrl_index);
		walkctrl_index = nil;
	end
end

walkctrl_Reload();
