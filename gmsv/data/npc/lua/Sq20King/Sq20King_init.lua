function Sq20King_Reload()
	if (Sq20King_index == nil) then					--这句检查NPC是否已经创建，防止重复创建
		--创建NPC第一个参数是SiegeCtrl_index函数所在的文件第二个参数是NPC初始化时候会调用的函数
		Sq20King_index = NL.CreateNpc("./data/npc/lua/Sq20King/Sq20King.lua", "Sq20King_Init");
		--向NPC周围的玩家发送NPC数据更新封包
		NLG.UpChar(Sq20King_index);
		return "神奇大魔王创建成功。"
	else
		NL.DelNpc(Sq20King_index);
		NLG.UpChar(Sq20King_index);
		Sq20King_index = nil;
		return "神奇大魔王删除成功。"
	end
end

Sq20King_Reload();
