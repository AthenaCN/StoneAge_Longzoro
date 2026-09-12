function equipenhance_Reload()
	if (equipenhance_index == nil) then
		equipenhance_index = NL.CreateNpc("./data/npc/lua/eq/equipenhance.lua", "equipenhance_Init");
		NLG.UpChar(equipenhance_index);
	else
		NL.DelNpc(equipenhance_index);
		NLG.UpChar(equipenhance_index);
		equipenhance_index = nil;
	end
end

equipenhance_Reload();
