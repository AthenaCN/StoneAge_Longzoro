function PetCatch_Init( _MeIndex )
	Char.SetData(_MeIndex, %对像_形象%, 100000);
	Char.SetData(_MeIndex, %对像_原形%, 100000);
	Char.SetData(_MeIndex, %对像_地图%, 2000);
	Char.SetData(_MeIndex, %对像_X%, 61);
	Char.SetData(_MeIndex, %对像_Y%, 83);
	Char.SetData(_MeIndex, %对像_方向%, %左下%);
	Char.SetData(_MeIndex, %对像_原名%, "宠物贩子");

	if (Char.SetTalkedEvent(nil, "PetCatch_Talked", _MeIndex) < 0) then
		print("PetCatch_Talked 注册事件失败。");
		return false;
	end
	if (Char.SetWindowTalkedEvent(nil, "PetCatch_WindowTalked", _MeIndex) < 0) then
		print("PetCatch_WindowTalked 注册事件失败。");
		return false;
	end

	return true;
end

--Talked事件
--目前最高SEQNO=13
function PetCatch_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	--這裡檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then
		return ;
	end

	NLG.ShowWindowTalked(_PlayerIndex, 2, 2, 0,
		"2\n 　　　　　　　   ☆宠物贩子☆\n"..
		"\n           　   【使用声望抓宠】"..
		"\n           　   【使用声望抓宠】"..
		"\n           　   【宠物贩子介绍】",
		_MeIndex);
	Char.SetData(_PlayerIndex,%对象_LUA%,0);
	return ;
end

-- WindowTalked 事件
function PetCatch_WindowTalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)

	--這裡檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_TalkIndex, _MeIndex, 1) == false) then
		return ;
	end
--数据格式为{"宠物名,enemyid,抓取耗费声望,抓取耗费声望"} 最后一条数据不要做修改，仅作为标记
	local TM_PetData = {{"佩露夏",1610,5,10},{"",0,0,0}}

	if _Seqno == 0 then
		if _Data == "1" then
			NLG.ShowWindowTalked(_TalkIndex, 1, 4+8, 1,
				" 　　　　　　　   ☆宠物贩子☆"..
				"\n请输入您要抓的宠物名称:" ,
				_MeIndex);
			return;
		end

		if _Data == "2" then
			NLG.ShowWindowTalked(_TalkIndex, 1, 4+8, 2,
				" 　　　　　　　   ☆宠物贩子☆"..
				"\n请输入您要抓的宠物名称:" ,
				_MeIndex);
			return;
		end

		if _Data == "3" then
			Char.SetData(_TalkIndex,%对象_LUA%,1);
			local TM_button;
			if string.len(TM_PetData[2][1])<1 then
				TM_button = 1;
			else
				TM_button = 2+32;
			end
			NLG.ShowWindowTalked(_TalkIndex, 0, TM_button, 0,"宠物名称："..TM_PetData[1][1].."\n捕捉所需声望："..TM_PetData[1][3].."\n捕捉所需声望："..TM_PetData[1][4],_MeIndex);
		end

		if _Select == 32 then
			local TM_Page = Char.GetData(_TalkIndex,%对象_LUA%)+1;
			if string.len(TM_PetData[TM_Page][1])<1 then
				return;
			end
			local TM_button;
			if string.len(TM_PetData[TM_Page+1][1])<1 then
				TM_button = 2+16;
			else
				TM_button = 16+32;
			end
			NLG.ShowWindowTalked(_TalkIndex, 0, TM_button, 0,"宠物名称："..TM_PetData[TM_Page][1].."\n捕捉所需声望："..TM_PetData[TM_Page][3].."\n捕捉所需声望："..TM_PetData[TM_Page][4],_MeIndex);
			Char.SetData(_TalkIndex,%对象_LUA%,TM_Page);
		elseif _Select == 16 then
			local TM_Page = Char.GetData(_TalkIndex,%对象_LUA%)-1;
			if string.len(TM_PetData[TM_Page][1])<1 then
				return;
			end
			local TM_button;
			if string.len(TM_PetData[TM_Page+1][1])<1 then
				TM_button = 2+32;
			else
				TM_button = 16+32;
			end
			NLG.ShowWindowTalked(_TalkIndex, 0, TM_button, 0,"宠物名称："..TM_PetData[TM_Page][1].."/n捕捉所需声望："..TM_PetData[TM_Page][3].."/n捕捉所需声望："..TM_PetData[TM_Page][4],_MeIndex);
			Char.SetData(_TalkIndex,%对象_LUA%,TM_Page);
		end

	end

	if _Seqno == 1 and _Select == 4 then
		local TM_Data=_Data;
		local index = 1;
		local length = #TM_PetData-1;


		--检测宠物名称输入正确性
		for i=1,length do
			if _Data == TM_PetData[i][1] then
				index = i;
			end
			if i==length then
				NLG.TalkToCli(_TalkIndex,"宠物名称输入错误，请查证后仔细输入！");
				return;
			end
		end

		if Char.GetData(_TalkIndex,%对像_队模式%) ~= %组队_没队% then
			NLG.TalkToCli(_TalkIndex,"组队模式中不允许抓宠！");
			return;
		end


		if Char.GetData(_TalkIndex,%对像_声望%) < TM_PetData[index][2] then
			NLG.TalkToCli(_TalkIndex,"您的声望不够，捕捉".._Data.."需要"..TM_PetData[index][2].."声望！");
			return;
		end
		Char.SetData(_TalkIndex,%对像_声望%,Char.GetData(_TalkIndex,%对像_声望%)-TM_PetData[index][2]);
		NLG.TalkToCli(_TalkIndex,"扣除"..TM_PetData[index][2].."声望！");

		NLG.TalkToCli(-1,"[大陆风云]勇者<"..Char.GetData(_TalkIndex,%对像_原名%)..">收买了宠物贩子，得到了梦宠【"..TM_Data.."】的捕捉机会！" , %红色%, 0, -1);
		NLG.CreateBattle( _TalkIndex, _MeIndex, nil, TM_PetData[index][2], nil, nil, %战类_普%);
	end


	if _Seqno == 2 and _Select == 4 then
		local TM_Data=_Data;
		local index = 1;
		local length = #TM_PetData-1;



		--检测宠物名称输入正确性
		for i=1,length do
			if _Data == TM_PetData[i][1] then
				index = i;
				print("PetCatch输入匹配成功：".."name=".._Data.." index="..index);
				break;
			end
			if i==length then
				NLG.TalkToCli(_TalkIndex,"宠物名称输入错误，请查证后仔细输入！");
				return;
			end
		end

		if Char.GetData(_TalkIndex,%对像_队模式%) ~= %组队_没队% then
			NLG.TalkToCli(_TalkIndex,"组队模式中不允许抓宠！");
			return;
		end


		if Char.GetData(_TalkIndex,%对像_声望%) < TM_PetData[index][3]*100 then
			NLG.TalkToCli(_TalkIndex,"您的声望不够，捕捉".._Data.."需要"..TM_PetData[index][3].."声望！");
			return;
		end
		Char.SetData(_TalkIndex,%对像_声望%,Char.GetData(_TalkIndex,%对像_声望%)-TM_PetData[index][3]*100);
		NLG.TalkToCli(_TalkIndex,"扣除"..TM_PetData[index][3].."声望！");

		NLG.TalkToCli(-1,"[大陆风云]勇者<"..Char.GetData(_TalkIndex,%对像_原名%)..">收买了宠物贩子，得到了梦宠【"..TM_Data.."】的捕捉机会！" , %红色%, 0, -1);
		NLG.CreateBattle( _TalkIndex, _MeIndex, nil, TM_PetData[index][3], nil, nil, %战类_普%);
	end

end



