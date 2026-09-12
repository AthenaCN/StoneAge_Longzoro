function PetUp_Reload()
	if (PetUp_index == nil) then
		PetUp_index = NL.CreateNpc("./data/npc/lua/PetUp/PetUp.lua", "PetUp_Init");
		NLG.UpChar(PetUp_index);
	else
		NL.DelNpc(PetUp_index);
		NLG.UpChar(PetUp_index);
		PetUp_index = nil;
	end
end

PetUp_Reload();
