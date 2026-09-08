			--抓宠NPC
	 if (PetCatchNpc_index ~= nil) then
		NL.DelNpc(PetCatchNpc_index);
	 end

	 if (PetCatchNpc_index == nil) then
	 print("创建抓宠npc");
		PetCatchNpc_index = NL.CreateNpc("./data/npc/lua/PetCatch/PetCatch.lua", "PetCatch_Init");
		NLG.UpChar(PetCatchNpc_index);
	end
