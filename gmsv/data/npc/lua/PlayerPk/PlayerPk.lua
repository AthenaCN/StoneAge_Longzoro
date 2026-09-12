function PlayerPk_Init( _MeIndex )
	Char.SetData(_MeIndex, %对像_形象%, 100000);
	Char.SetData(_MeIndex, %对像_原形%, 10000);
	Char.SetData(_MeIndex, %对像_地图%, 2005);
	Char.SetData(_MeIndex, %对像_X%, 34);
	Char.SetData(_MeIndex, %对像_Y%, 11);
	Char.SetData(_MeIndex, %对像_方向%, %左下%);
	Char.SetData(_MeIndex, %对像_原名%, "【格斗鉴定师】");

	if (Char.SetTalkedEvent(nil, "PlayerPk_Talked", _MeIndex) < 0) then
		print("PlayerPk_Talked 注册事件失败。");
		return false;
	end

	return true;
end

--Talked事件
--目前最高SEQNO=13
function PlayerPk_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	--這裡檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then
		return ;
	end
	--面向玩家
	--local i;
	--i = Char.GetData(_PlayerIndex, %对像_方向%);
	--if i >= 4 then
	--	i = i - 4;
	--else
	--	i = i + 4;
	--end
	--Char.SetData(_MeIndex, %对像_方向%,i);
	--NLG.UpChar( _MeIndex);
	NLG.ShowWindowTalked(_PlayerIndex, 0, 2, 0,
 "　　　　　　      ☆格斗鉴定师☆\n"..
 "\n以下是你当前战斗战绩结果："..
 "\n                PK  次  数："..Char.GetData(_PlayerIndex,%对像_PK数%)..
 "\n                PK  赢  数："..Char.GetData(_PlayerIndex,%对像_P赢数%)..
 "\n                PK  败  数："..Char.GetData(_PlayerIndex,%对像_P输数%)..
 "\n                PK  连  胜："..Char.GetData(_PlayerIndex,%对像_P连数%)..
 "\n                PK最高连胜："..Char.GetData(_PlayerIndex,%对像_P最高%),
 _MeIndex);
return ;
end

