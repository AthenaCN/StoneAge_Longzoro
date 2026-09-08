function MoWang_Reload()
	if (MoWang_index == nil) then
		MoWang_index = NL.CreateNpc("./data/npc/lua/MoWang/MoWang.lua", "MoWang_Init");
		NLG.UpChar(MoWang_index);
	else
		NL.DelNpc(MoWang_index);
		NLG.UpChar(MoWang_index);
		MoWang_index = nil;
	end
end

function MoWangBoss_Reload()
	iBattle2 = {-1};
	NPC_I=1;
	while(NPC_I<=11)do
		MoWangBoss_index = NL.CreateNpc("./data/npc/lua/MoWang/MoWangBoss.lua", "MoWangBoss_Init");
		NPC_I = NPC_I+1;
		NLG.UpChar(MoWangBoss_index);
	end
end

MoWang_Reload();
MoWangBoss_Reload();