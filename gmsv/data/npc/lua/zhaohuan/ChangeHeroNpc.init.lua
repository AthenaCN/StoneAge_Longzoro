function GMSV_NPC_Init()

	--英雄完成次数
	 if (ChangeHeroNpc_index == nil) then

		ChangeHeroCf = {};
		ChangeHeroCf["ImageNo"] = 100000;  --形象编码
		ChangeHeroCf["X"] = 52;				--NpcX坐标
		ChangeHeroCf["Y"] = 51;				--NpcY坐标
		ChangeHeroCf["Name"] = "快速精灵召唤";	--Npc名称
		ChangeHeroCf["Floor"] = 2000;		--Npc所在地图编号 如2000为玛丽渔村
		ChangeHeroNpc_index = NL.CreateNpc("./data/npc/lua/zhaohuan/ChangeHeroNpc.lua", "ChangeHeroNpc_Init");
		NLG.UpChar(ChangeHeroNpc_index);
	end
end



--随机种子
math.randomseed(os.time());

GMSV_NPC_Init();

