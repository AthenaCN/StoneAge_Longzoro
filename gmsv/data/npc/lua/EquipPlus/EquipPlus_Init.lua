function EquipPlus_Reload()
	if (EquipPlus_index == nil) then
		EquipPlus_index = NL.CreateNpc("./data/npc/lua/EquipPlus/EquipPlus.lua", "EquipPlus_Init");
		NLG.UpChar(EquipPlus_index);
	else
		NL.DelNpc(EquipPlus_index);
		NLG.UpChar(EquipPlus_index);
		EquipPlus_index = nil;
	end
end

EquipPlus_Reload();
