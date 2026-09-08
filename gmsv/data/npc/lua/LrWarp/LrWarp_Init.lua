function LrWarp_Reload()
	if (LrWarp_index == nil) then
		LrWarp_index = NL.CreateNpc("./data/npc/lua/LrWarp/LrWarp.lua", "LrWarp_Init");
		NLG.UpChar(LrWarp_index);
	else
		NL.DelNpc(LrWarp_index);
		NLG.UpChar(LrWarp_index);
		LrWarp_index = nil;
	end
end

LrWarp_Reload();