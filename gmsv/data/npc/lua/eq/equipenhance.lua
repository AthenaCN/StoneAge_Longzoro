function equipenhance_Init( _MeIndex )
	--下方阵列设定可以强化的道具 可以设定n个
	equipment = {22932,22933,22934,22935,22936,22937,22938,22939,22940,22941,22942,22943,22944,22945,22946,22947,22948,22949,22950,22951,22952,22953,22954,22955,22956,22957,22958,22959,22960,22961,22962,22963,22964,22965,22966,22967, -1, -1 };

	successrate = 30;  -- 输入 1~100 数值越高越容易成功
	-- 此冲装系统预设 +1 ~ +5 成功率为 100%
	-- +6 ~ +8 成功率为 successrate %
	-- +9 的成功率为 successrate * 0.6 %

	--下方设定每升级一次能力增加值 +6~+8增加的值为下方的两倍 +9为下方的三倍
	--hpup  = 10; 体力
	--mpup  = 3;  气
	--strup = 5;  攻
	--dexup = 5;  敏
	--tghup = 5;  防

 	-- +1~+9 所需的材料设置 地/水/火/风
	needitem = {23053, 23053, 23053, 23053, 23053, 23053, 23053, 23053, 23053};  --++
	--以下设定NPC相关资讯
	--设置NPC形象 
	Char.SetData(_MeIndex, %对像_形象%, 101767);
	--设置NPC所在地图
	Char.SetData(_MeIndex, %对像_地图%, 2000);
	--设置X坐标
	Char.SetData(_MeIndex, %对像_X%, 57);
	--设置Y坐标
	Char.SetData(_MeIndex, %对像_Y%, 78);
	--设置NPC方向
	Char.SetData(_MeIndex, %对像_方向%, %右下%);
	Char.SetData(_MeIndex, %对像_原名%, "上身5装备升级师");--设置NPC名称


	--响应说话事件，此事件如果有玩家对此NPC说话就会触发
	if (Char.SetTalkedEvent(nil, "equipenhance_Talked", _MeIndex) < 0) then
		print("Talked 注册事件失败。");
		return false;	
	end
	--窗口事件
	if (Char.SetWindowTalkedEvent(nil, "equipenhance_WindowTalked", _MeIndex) < 0) then
		print("WindowTalked 注册事件失败。");
		return false;
	end
	
	NLG.UpChar(_MeIndex); 
	return true; 
end

function equipenhance_checkitem(_itemid) --检查道具是否可以强化
	for i = 1,table.getn(equipment) do
		if _itemid == equipment[i-1] then
			return 1; 
		end
	end
	return 0;
end

function equipenhance_getlevel( _index)  --取得道具等级
	local buf = Item.GetData( _index,%道具_原名%);
	local buf2 = Item.GetData( _index,%道具_名%);
	local level = 0;
	if( (buf.."○○●") == buf2)then
		level = 1;
	elseif( (buf.."○●●") == buf2)then
		level = 2;
	elseif( (buf.."●●●") == buf2)then
		level = 3;
	elseif( (buf.."●●●●") == buf2)then   --+
		level = 4;
		elseif( (buf.."●●●5") == buf2)then   --+
		level = 5;
		elseif( (buf.."●●●6") == buf2)then   --+
		level = 6;
		elseif( (buf.."●●●7") == buf2)then   --+
		level = 7;
		elseif( (buf.."●●●8") == buf2)then   --+
		level = 8;
		elseif( (buf.."●MAX") == buf2)then   --+
		level = 9;
	
	end
	return level;
end

function equipenhance_enhance( _PlayerIndex,_Select) --检查选择道具强化条件

	-- 
	needitemnum = {1, 1, 1, 1, 1, 1, 1, 1, 1};   --++
	if _Select >= 0 then

		local itemindex = Char.GetItemIndex(_PlayerIndex,_Select);
		local level = equipenhance_getlevel(itemindex);

		if itemindex >= 0 then
			local itemid = Item.GetData(itemindex,%道具_ID%);
			if(equipenhance_checkitem(itemid) == 0)then
				NLG.TalkToCli(_PlayerIndex,"此道具无法被强化。",%黄色%);
				return;
			else if (level == 9) then    --++
				NLG.TalkToCli(_PlayerIndex,"此道具已达最高强化等级。",%红色%);
				return;
			else if (Char.FindItemId(_PlayerIndex,needitem[level+1]) < needitemnum[level+1]) then
				NLG.TalkToCli(_PlayerIndex,"所需的道具不足。",%黄色%);
			else
				equipenhance_enhancemain(_PlayerIndex,itemindex,itemid);
			end
		end
	end
end
	return ;
end
end

function equipenhance_enhancemain(_PlayerIndex, itemindex, itemid) --强化道具主程式

	local rate = math.floor(math.random(1, 100));
	local level = equipenhance_getlevel(itemindex);
	local hpup = math.floor(math.random(0, 0));
	local mpup = math.floor(math.random(0, 0));
	local strup = math.floor(math.random(5, 15));
	local dexup = math.floor(math.random(5, 10));
	local tghup = math.floor(math.random(5, 8));
	if level <9 then  -- 1~5 级成功率100%
		Item.SetData(itemindex,%道具_附体%, Item.GetData(itemindex,%道具_附体%) + hpup);
		Item.SetData(itemindex,%道具_附气%, Item.GetData(itemindex,%道具_附气%) + mpup);
		Item.SetData(itemindex,%道具_附攻%, Item.GetData(itemindex,%道具_附攻%) + strup);
		Item.SetData(itemindex,%道具_附敏%, Item.GetData(itemindex,%道具_附敏%) + dexup);
		Item.SetData(itemindex,%道具_附防%, Item.GetData(itemindex,%道具_附防%) + tghup);
		--Item.SetData(itemindex,%道具_附防%, Item.GetData(itemindex,%道具_附防%) + tghup);
		Item.SetData(itemindex,%道具_耐久%, Item.GetData(itemindex,%道具_MAX耐久%));

	if level == 8 then
		local buff = Item.GetData(itemindex,%道具_原名%).."●MAX";    --++
		Item.SetData(itemindex,%道具_名%,buff);
	end
	if level == 7 then
		local buff = Item.GetData(itemindex,%道具_原名%).."●●●8";    --++
		Item.SetData(itemindex,%道具_名%,buff);
	end
	if level == 6 then
		local buff = Item.GetData(itemindex,%道具_原名%).."●●●7";    --++
		Item.SetData(itemindex,%道具_名%,buff);
	end
	if level == 5 then
		local buff = Item.GetData(itemindex,%道具_原名%).."●●●6";    --++
		Item.SetData(itemindex,%道具_名%,buff);
	end
	if level == 4 then
		local buff = Item.GetData(itemindex,%道具_原名%).."●●●5";    --++
		--Item.SetData(itemindex,%道具_名%,buff);
		Item.SetData(itemindex,%道具_名%,buff);
	end
	if level == 3 then
		local buff = Item.GetData(itemindex,%道具_原名%).."●●●●";    --++
		Item.SetData(itemindex,%道具_名%,buff);
	end
	if level == 2 then
		local buff = Item.GetData(itemindex,%道具_原名%).."●●●";
		Item.SetData(itemindex,%道具_名%,buff);
	end
	if level == 1 then
		local buff = Item.GetData(itemindex,%道具_原名%).."○●●";
		Item.SetData(itemindex,%道具_名%,buff);
	end
	if level == 0 then
		local buff = Item.GetData(itemindex,%道具_原名%).."○○●";
		Item.SetData(itemindex,%道具_名%,buff);
	end

		--改说明
		buff = "攻(+)"..Item.GetData(itemindex,%道具_附攻%).." 敏(+)"..Item.GetData(itemindex,%道具_附敏%).." 防(+)"..Item.GetData(itemindex,%道具_附防%).." HP(+)"..Item.GetData(itemindex,%道具_附体%).." MP(+)"..Item.GetData(itemindex,%道具_附气%).." 【".. level+1 .."】次强化";
		Item.SetData(itemindex,%道具_说明%,buff);
		NLG.DelItem(_PlayerIndex,needitem[level+1],needitemnum[level+1]);
			if level+1 == 9 then    --++
				local buff2 = Item.GetData(itemindex,%道具_名%);
				local buff3 = Char.GetData(_PlayerIndex,%对像_原名%);
				--NLG.TalkToCli( -1, "恭喜玩家“" .. buff3 .. "”的武器“" .. buff2 .."”灵力已达到最高级，大家为他喝采！", %紫色%, 0, -1);
				NLG.TalkToCli( -1, "恭喜玩家“" .. buff3 .. "”的武器“" .. buff2 .."”灵力已达到最高级，大家为他喝采！", %紫色%, 0, -1);
			else
				NLG.TalkToCli(_PlayerIndex,"道具强化完成，目前为 ".. level+1 .." 级",%红色%);
			end
		return;
	end
	return ;
end

function equipenhance_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel) --对话事件
 	--这里检查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 2) == false) then
		return ;
	end

	local i;   --以下让NPC面对你
	i = Char.GetData(_PlayerIndex, %对像_方向%);
	if i >= 4 then 
		i = i - 4;
	else
		i = i + 4;		
	end
	Char.SetData(_MeIndex, %对像_方向%,i);
	NLG.UpChar( _MeIndex);
	local token = "4\n　　　　　　　　“ 浩瀚＊装备强化 ”\n我有神秘的力量，能让您的上下身LV5装备拥有更大的灵力，记得带好精炼石LV3哦。\n"..
			"\n               　‘强化装备’".."\n               　【打个招呼】";
	NLG.ShowWindowTalked( _PlayerIndex, 2, 0, 0,  token, _MeIndex); --对客户端发送窗口的封包

return;
end


function equipenhance_WindowTalked( _MeIndex, _PlayerIndex, _Seqno, _Select, _Data, _itemid) --窗口事件

	if _Seqno == 0 and _Data == "1" or _Seqno == 2 and _Data == "8" then
			local pos = 9;
			local TM_Buff = "1\n请选择道具：　　　　　　　　　【第一页】";
			local itemindex;
			local itempos = {"灵化【1】","灵化【2】","灵化【3】","灵化【4】","灵化【5】","灵化【6】","灵化【7】","灵化【8】"};
			while(pos <= 17)do
			itemindex = Char.GetItemIndex( _PlayerIndex, pos);
			if (pos == 17) then
				TM_Buff = TM_Buff .. "\n" .. "　　　　　　　下一页";
			else
				if( itemindex > 0)then	
						local buff2 = Item.GetData(itemindex,%道具_ID%);
						if (buff2 == 21279) then
						TM_Buff = TM_Buff.."\n".. itempos[pos - 8] .. "： " .. Item.GetData( itemindex, %道具_名%) .. "（可）";
						elseif (buff2 == 21280) then
						TM_Buff = TM_Buff.."\n".. itempos[pos - 8] .. "： " .. Item.GetData( itemindex, %道具_名%) .. "（可）";
						elseif (buff2 == 21283) then
						TM_Buff = TM_Buff.."\n".. itempos[pos - 8] .. "： " .. Item.GetData( itemindex, %道具_名%) .. "（可）";
						elseif buff2 == 21281 then
						TM_Buff = TM_Buff.."\n".. itempos[pos - 8] .. "： " .. Item.GetData( itemindex, %道具_名%) .. "（灵化石）";
						else
						TM_Buff = TM_Buff.."\n".. itempos[pos - 8] .. "： " .. Item.GetData( itemindex, %道具_名%) .. "（不可）";
						end
				else
					TM_Buff = TM_Buff .. "\n" .. itempos[pos - 8] .. "： 无灵化装备";
				end
			end
			pos = pos + 1;
			end
			NLG.ShowWindowTalked( _PlayerIndex, 2, 0, 1, TM_Buff, _MeIndex);
			return;
	end

	if _Seqno == 1 and _Data == "9" then --换页选道具
			local pos = 17;
			local TM_Buff = "1\n请选择道具：　　　　　　　　　【第二页】";
			local itemindex;
			local itempos = {"灵化【９】","灵化【10】","灵化【11】","灵化【12】","灵化【13】","灵化【14】","灵化【15】"};

			while(pos <= 24)do
				itemindex = Char.GetItemIndex( _PlayerIndex, pos);
				if (pos == 24) then
				TM_Buff = TM_Buff .. "\n" .. "　　　　　　　   上一页".."\n" .. "　　　　　　　   取　消";
				else
					if( itemindex > 0)then
						TM_Buff = TM_Buff.."\n".. itempos[pos - 16] .. "： " .. Item.GetData( itemindex, %道具_名%);
					else
						TM_Buff = TM_Buff .. "\n" .. itempos[pos - 16] .. "： 无灵化装备";
					end
				end
				pos = pos + 1;
				end
				NLG.ShowWindowTalked( _PlayerIndex, 2, 0, 2, TM_Buff, _MeIndex);
	end
	if _Seqno == 1 and _Data ~= "9" then --选第一页的道具
		equipenhance_enhance( _PlayerIndex,_Data + 8);
	end
	if _Seqno == 2 and _Data > "0" then --选第二页的道具
		equipenhance_enhance( _PlayerIndex,_Data + 16);
	end
	return;
end