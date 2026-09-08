function PlayerPk_Reload()
	if (PlayerPk_index == nil) then
		PlayerPk_index = NL.CreateNpc("./data/npc/lua/PlayerPk/PlayerPk.lua", "PlayerPk_Init");
		NLG.UpChar(PlayerPk_index);
	else
		NL.DelNpc(PlayerPk_index);
		NLG.UpChar(PlayerPk_index);
		PlayerPk_index = nil;
	end
end

PlayerPk_Reload();