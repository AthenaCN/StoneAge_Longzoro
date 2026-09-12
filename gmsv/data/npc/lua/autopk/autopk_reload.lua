function autopk_reload()
	if (autopk_index == nil) then
		autopk_index = NL.CreateNpc("./data/npc/lua/autopk/autopk.lua", "autopk_init");
		NLG.UpChar(autopk_index);
 		return "创建成功";
	else
		NL.DelNpc(autopk_index);
		autopk_index = nil;
	return "删除成功";
	end
end

print(autopk_reload());
