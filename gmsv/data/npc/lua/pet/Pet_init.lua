function Pet_Reload()
	if (Pet_index == nil) then					--这句检查NPC是否已经创建，防止重复创建
		Pet_index = NL.CreateNpc("./data/npc/lua/pet/pet.lua", "Pet_Init");
		NLG.UpChar(Pet_index);
	else
		NL.DelNpc(Pet_index);
		NLG.UpChar(Pet_index);
		Pet_index = nil;
	end
end

Pet_Reload();
