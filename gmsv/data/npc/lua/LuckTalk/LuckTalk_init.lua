function LuckTalk_Reload()
	if (LuckTalk_index == nil) then					--这句检查NPC是否已经创建，防止重复创建
		--创建NPC第一个参数是LuckTalk_index函数所在的文件第二个参数是NPC初始化时候会调用的函数
		LuckTalk_index = NL.CreateNpc("./data/npc/lua/LuckTalk/LuckTalk.lua", "LuckTalk_Init");
		--向NPC周围的玩家发送NPC数据更新封包
		NLG.UpChar(LuckTalk_index);
	else
		NL.DelNpc(LuckTalk_index);
		NLG.UpChar(LuckTalk_index);
		LuckTalk_index = nil;
	end
end

LuckTalk_Reload();
