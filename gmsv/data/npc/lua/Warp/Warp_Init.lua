function Warp_Reload()
	if (Warp_index == nil) then
		Warp_index = NL.CreateNpc("./data/npc/lua/Warp/Warp.lua", "Warp_Init");
		NLG.UpChar(Warp_index);
	else
		NL.DelNpc(Warp_index);
		NLG.UpChar(Warp_index);
		Warp_index = nil;
	end
end

Warp_Reload();
