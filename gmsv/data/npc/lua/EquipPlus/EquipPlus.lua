function EquipPlus_Init( _MeIndex )
	Char.SetData(_MeIndex, %对像_形象%, 100700);
	Char.SetData(_MeIndex, %对像_原形%, 100700);
	Char.SetData(_MeIndex, %对像_地图%, 2000);
	Char.SetData(_MeIndex, %对像_X%, 49);
	Char.SetData(_MeIndex, %对像_Y%, 78);
	Char.SetData(_MeIndex, %对像_方向%, %右下%);
	Char.SetData(_MeIndex, %对像_原名%, "装备强化大师");

	if (Char.SetTalkedEvent(nil, "EquipPlus_Talked", _MeIndex) < 0) then
		print("EquipPlus_Talked 注册事件失败。");
		return false;
	end
	if (Char.SetWindowTalkedEvent(nil, "EquipPlus_WindowTalked", _MeIndex) < 0) then
		print("pztestnpc_WindowTalked 注册事件失败。");
		return false;
	end

	return true;
end

--Talked事件
--目前最高SEQNO=13
function EquipPlus_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	--這裡檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then
		return ;
	end 
	--面向玩家
	local i;
	i = Char.GetData(_PlayerIndex, %对像_方向%);
	if i >= 4 then 
		i = i - 4;
	else
		i = i + 4;		
	end
	Char.SetData(_MeIndex, %对像_方向%,i);
	NLG.UpChar( _MeIndex);
	NLG.ShowWindowTalked(_PlayerIndex, 2, 0, 0,
             "7\n　　　　　　   ☆装备强化系统☆"..
             "\n装备强化说明:"..
             "\n"..
             "\n"..
             "\n"..
             "\n"..
             "\n"..
             "\n                 【强化装备】"..
             "\n                 【离    开】",
 _MeIndex);
return ;
end

-- WindowTalked 事件
function EquipPlus_WindowTalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)

	--這裡檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_TalkIndex, _MeIndex, 1) == false) then
		return ;
	end
	if _Seqno == 0 and _Data == "1" then
		NLG.ShowWindowTalked(_TalkIndex, 2, 0, 1,
			 "7\n　　　　　　   ☆装备强化系统☆"..
			 "\n请选择(请勿更改道具名称):" ..
             "\n"..
             "\n"..
             "\n"..
             "\n"..
             "\n"..
		 	 "\n                 【装备列表】" ..
		 	 "\n                 【道具列表】" ..
		 	 "\n                 【离    开】",
			 _MeIndex);
		return;
	end
	
	if _Seqno == 1 then
		if _Data == "1" then
			local pos = 0;
			--改说明时勿添行
			local TM_Buff = "1\n请选择:";
			local itemindex;
			local equip = {"头部","身体","右手","右饰","左饰","腰间","左手","脚部","手掌"};

			while(pos <= 8)do
				itemindex = Char.GetItemIndex( _TalkIndex, pos);
				if( itemindex > 0)then
					TM_Buff = TM_Buff.."\n".. equip[pos + 1] .. "：" .. Item.GetData( itemindex, %道具_名%);
				else
					TM_Buff = TM_Buff .. "\n" .. equip[pos + 1] .. "：无装备";
				end
				pos = pos + 1;
			end
			NLG.ShowWindowTalked( _TalkIndex, 2, 0, 2, TM_Buff, _MeIndex);
			return;
		end
		
		if _Data == "2" then
			--改说明时勿添行
			local pos = 9;
			local TM_Buff = "1\n请选择道具：";
			local itemindex;
			local itempos = {"道具栏1","道具栏2","道具栏3","道具栏4","道具栏5","道具栏6","道具栏7","道具栏8"};

			while(pos <= 17)do
			itemindex = Char.GetItemIndex( _TalkIndex, pos);
			if (pos == 17) then
				TM_Buff = TM_Buff .. "\n" .. "　　　　　　　         下一页";
			else
				if( itemindex > 0)then
					TM_Buff = TM_Buff.."\n".. itempos[pos - 8] .. "：" .. Item.GetData( itemindex, %道具_名%);
				else
					TM_Buff = TM_Buff .. "\n" .. itempos[pos - 8] .. "：无物品";
				end
			end
			pos = pos + 1;
			end
			NLG.ShowWindowTalked( _TalkIndex, 2, 0, 3, TM_Buff, _MeIndex);
			return;
		end
	end

	if(_Seqno == 3 or _Seqno == 2 or _Seqno == 4) then
		if (_Seqno == 3 and _Data == "9") then
			--改说明时勿添行
			local pos = 17;
			local TM_Buff = "0\n";
			local itemindex;
			local itempos = {"道具栏9","道具栏10","道具栏11","道具栏12","道具栏13","道具栏14","道具栏15"};

			while(pos <= 24)do
				itemindex = Char.GetItemIndex( _TalkIndex, pos);
				if (pos == 24) then
				TM_Buff = TM_Buff .. "\n" .. "　　　　　　　        离  开";
				else
					if( itemindex > 0)then
						TM_Buff = TM_Buff.."\n".. itempos[pos - 16] .. "：" .. Item.GetData( itemindex, %道具_名%);
					else
						TM_Buff = TM_Buff .. "\n" .. itempos[pos - 16] .. "：无物品";
					end
				end
				pos = pos + 1;
			end

			NLG.ShowWindowTalked( _TalkIndex, 2, 0, 4, TM_Buff, _MeIndex);
		else
			local sel = tonumber(_Data);
			if _Seqno == 4 then
				sel = sel - 1;
			end
			EquipPlus_Plus(_TalkIndex,_Seqno-1,sel);
   		end
	end
end

function EquipPlus_Plus( _PlayerIndex,_Page,_Sel)
	local itemtbl = { {0,1,2,3,4,5,6,7,8} , {9,10,11,12,13,14,15,16,17} , {18,19,20,21,22,23,-1,-1,-1} };
	
	local haveitemindex = itemtbl[_Page][_Sel];

	if haveitemindex >= 0 then
		local itemindex = Char.GetItemIndex(_PlayerIndex,haveitemindex);
		if itemindex >= 0 then
			local itemid = Item.GetData(itemindex,%道具_ID%);
			if(EquipPlus_Check(itemid) > 0)then
				EquipPlus_PPlus(_PlayerIndex,itemindex,itemid);
				return;
			else 
				NLG.TalkToCli(_PlayerIndex,"该物品无法强化!",%红色%);
				return;
			end
		end
	end
end

function EquipPlus_Check(_ItemID)
	local MAX = 10; --下面列表的项目数
	local PlusItemIDTbl = {14871,16671,17451,18091,18092,18093,2070,2071,2072,2056}; --可以加强的道具ID

	local i = 1;

	while(i<=MAX)do
		if _ItemID == PlusItemIDTbl[i] then
			return 1;
		end
		i = i + 1;
	end
	
	return 0;
end

function EquipPlus_PPlus(charaindex,itemindex,itemid)
	local buf = Item.GetData(itemindex,%道具_原名%);
	local buf2 = Item.GetData(itemindex,%道具_名%);
	--1~9级需要的加强材料,最后一项不要改
	local itemtbl = {20841,20841,20841,20842,20842,20842,20843,20843,20843,-1};
	--1~9级需要的材料数量,最后一项不要改
	local itemnumtbl = {1,1,1,1,1,1,1,1,1,1};
	local level = 0;
	if( (buf.."+1") == buf2)then
		level = 1;
	elseif( (buf.."+2") == buf2)then
		level = 2;
	elseif( (buf.."+3") == buf2)then
		level = 3;
	elseif( (buf.."+4") == buf2)then
		level = 4;
	elseif( (buf.."+5") == buf2)then
		level = 5;
	elseif( (buf.."+6") == buf2)then
		level = 6;
	elseif( (buf.."+7") == buf2)then
		level = 7;
	elseif( (buf.."+8") == buf2)then
		level = 8;
	elseif( (buf.."+9") == buf2)then
		level = 9;
	end
	if(level == 9)then
		NLG.TalkToCli(charaindex,"已经达到强化最大值!",%青色%);
		return;
	end
	if (Char.FindItemId(charaindex,itemtbl[level+1]) < itemnumtbl[level+1]) then
		NLG.TalkToCli(charaindex,"缺少强化所需要的材料!",%红色%);
		return;
	end
					--这里是概率,现在是1/10,改10为2则1/2的概率
	local randnum = math.floor(math.random(1, 1));
	if(randnum == 1)then
		if(level == 8 or level == 7)then
			--这是9级加强
			Item.SetData(itemindex,%道具_附攻%, Item.GetData(itemindex,%道具_附攻%) + 15);
			--加HP
			Item.SetData(itemindex,%道具_附体%, Item.GetData(itemindex,%道具_附体%) + 15);
			--加MP
			Item.SetData(itemindex,%道具_附气%, Item.GetData(itemindex,%道具_附气%) + 15);
			Item.SetData(itemindex,%道具_附防%, Item.GetData(itemindex,%道具_附防%) + 15);
			Item.SetData(itemindex,%道具_附敏%, Item.GetData(itemindex,%道具_附敏%) + 15);
		else
			Item.SetData(itemindex,%道具_附攻%, Item.GetData(itemindex,%道具_附攻%) + 5);
			--加HP
			
			--加MP
			Item.SetData(itemindex,%道具_附气%, Item.GetData(itemindex,%道具_附气%) + 5);
			Item.SetData(itemindex,%道具_附防%, Item.GetData(itemindex,%道具_附防%) + 5);
			Item.SetData(itemindex,%道具_附敏%, Item.GetData(itemindex,%道具_附敏%) + 5);
      Item.SetData(itemindex,%道具_附体%, Item.GetData(itemindex,%道具_附体%) + 5);		
		end
		local buff = Item.GetData(itemindex,%道具_原名%).."+"..level+1;
		Item.SetData(itemindex,%道具_名%,buff);
		local buff = Item.GetData(itemindex,%道具_说明%);
		if level == 0 then
			local i,j;
			i,j = string.find(buff,"次加强");
			if(i ~=2 or j ~= 7)then
				buff = "1次加强"..buff;
			else
				buff = string.gsub(buff,level,level+1);
			end
		else
			buff = string.gsub(buff,level,level+1);
		end
		Item.SetData(itemindex,%道具_说明%,buff);

		NLG.DelItem(charaindex,itemtbl[level+1],itemnumtbl[level+1]);
		NLG.TalkToCli(charaindex,"恭喜你，强化成功!(请重新登陆查看装备属性)",%黄色%);
		NLG.TalkToCli(-1,"恭喜"..Char.GetData(charaindex,%对像_原名%).."成功强化"..Item.GetData(itemindex,%道具_原名%)..",现在是+"..level+1,%紫色%);
		return;
	else
		NLG.DelItem(charaindex,itemtbl[level+1],itemnumtbl[level+1]);
					--降级概率,现在是1/2,改2为3则变为1/3
		randnum = math.floor(math.random(1,1));
		if randnum == 1 then
			if(level == 0)then
			NLG.TalkToCli(charaindex,"强化装备失败",%红色%);
			return;
			end
			if(level == 8 or level == 7 or level == 6)then
				Item.SetData(itemindex,%道具_附攻%, Item.GetData(itemindex,%道具_附攻%) - 15*(level-7) - 5 * 7);
				Item.SetData(itemindex,%道具_附防%, Item.GetData(itemindex,%道具_附防%) - 15*(level-7) - 5 * 7);
				Item.SetData(itemindex,%道具_附敏%, Item.GetData(itemindex,%道具_附敏%) - 15*(level-7) - 5 * 7);
				Item.SetData(itemindex,%道具_附体%, Item.GetData(itemindex,%道具_附体%) - 15*(level-7) - 5 * 7);
				Item.SetData(itemindex,%道具_附气%, Item.GetData(itemindex,%道具_附气%) - 15*(level-7) - 5 * 7);
				local buff = Item.GetData(itemindex,%道具_原名%);
				Item.SetData(itemindex,%道具_名%,buff);
				local buff = Item.GetData(itemindex,%道具_说明%);
				buff = string.gsub(buff,level,"0");
				Item.SetData(itemindex,%道具_说明%,buff);
				NLG.TalkToCli(charaindex,"强化失败!该装备初始化",%红色%);
			else
				Item.SetData(itemindex,%道具_附攻%, Item.GetData(itemindex,%道具_附攻%) - 5);
				Item.SetData(itemindex,%道具_附防%, Item.GetData(itemindex,%道具_附防%) - 5);
				Item.SetData(itemindex,%道具_附敏%, Item.GetData(itemindex,%道具_附敏%) - 5);
				Item.SetData(itemindex,%道具_附体%, Item.GetData(itemindex,%道具_附体%) - 5);
				Item.SetData(itemindex,%道具_附气%, Item.GetData(itemindex,%道具_附气%) - 5);
				local buff = Item.GetData(itemindex,%道具_原名%).."+"..level-1;
				Item.SetData(itemindex,%道具_名%,buff);
				local buff = Item.GetData(itemindex,%道具_说明%);
				buff = string.gsub(buff,level,level-1);
				Item.SetData(itemindex,%道具_说明%,buff);
				NLG.TalkToCli(charaindex,"强化失败!等级下降",%红色%);
				end
			return;
		else
			NLG.TalkToCli(charaindex,"对不起，强化失败!",%红色%);
		end




	end
end