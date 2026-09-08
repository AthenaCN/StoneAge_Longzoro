function ReSetPetPoint_Reload()
	if (ReSetPetPoint_index == nil) then					--这句检查NPC是否已经创建，防止重复创建
		ReSetPetPoint_index = NL.CreateNpc("./data/npc/lua/ReSetPetPoint/ReSetPetPoint.lua", "ReSetPetPoint_Init");
		NLG.UpChar(ReSetPetPoint_index);
	else
		NL.DelNpc(ReSetPetPoint_index);
		NLG.UpChar(ReSetPetPoint_index);
		ReSetPetPoint_index = nil;
	end
end

ReSetPetPoint_Reload();
