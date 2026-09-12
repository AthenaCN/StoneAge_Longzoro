function QQCSA_MW_Reload()
	if (QQCSA_MW_index == nil) then
		QQCSA_MW_index = NL.CreateNpc("./data/npc/lua/hls/QQMW.lua", "QQCSA_MW_Init");
		NLG.UpChar(QQCSA_MW_index);
	else
		NL.DelNpc(QQCSA_MW_index);
		NLG.UpChar(QQCSA_MW_index);
		QQCSA_MW_index = nil;
	end
end

QQCSA_MW_Reload();
