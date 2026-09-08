
--_MeIndex 是本NPC的索引
function QQCSA_MW_Init( _MeIndex )
	--设置NPC形象
	Char.SetData(_MeIndex, %对像_形象%, 104516);

	--设置NPC所在地图
	Char.SetData(_MeIndex, %对像_地图%, 2000);

	--设置X坐标
	Char.SetData(_MeIndex, %对像_X%, 50);

	--设置Y坐标
	Char.SetData(_MeIndex, %对像_Y%, 54);

	--设置NPC方向
	Char.SetData(_MeIndex, %对像_方向%, %右下%);

	--设置NPC名称
	Char.SetData(_MeIndex, %对像_原名%, "欢乐派派送");
	--設置手動啟動函數
	Char.SetTalkedEvent(nil, "QQCSAMW_Manual", _MeIndex);
	
		--窗口對話事件
	if (Char.SetWindowTalkedEvent(nil, "QQCSAMW_ManualWindow", _MeIndex) < 0) then
		print("QQCSAMW_ManualWindow 注册事件失败。");
		return false;
	end
	
	if (Char.SetTalkedEvent(nil, "QQCSA_MW_Talked", _MeIndex) < 0) then
		print("QQCSA_MW_Talked 注册事件失败。");
		return false;
	end
	return true;
end




function QQCSAMW_Manual( _MeIndex, _TalkIndex, _Msg, _Color, _Channel)

	--檢查玩家是否站在NPC面前
	if(NLG.CheckInFront(_TalkIndex, _MeIndex, 1) == false) then
		return ;
	end 

	--面向玩家
	local i;
	i = Char.GetData(_TalkIndex, %对像_方向%);
	if i >= 4 then 
		i = i - 4;
	else
		i = i + 4;		
	end
	Char.SetData(_MeIndex, %对像_方向%,i);
	NLG.UpChar( _MeIndex);

	--檢查口令
	NLG.ShowWindowTalked(_TalkIndex, 1, 3, 0, "              【欢乐派派送】\n\n  我是一个派派送使者,给我魔王的证明,我将赠与你道具或者宠物！",  _MeIndex);
end

function QQCSAMW_ManualWindow( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)

	--判定是否檢查口令窗口
	if(_Seqno == 0) then
		--判定是否按了OK
		if (_Select == 1) then
     QQCSA_MW_Talked( _MeIndex, _TalkIndex, "hi", 0, -1);
	end
end
end



--Talked事件不需要返回值所以 return ; 既可
--_MeIndex 是本NPC的索引
--_PlayerIndex 触发事件的玩家索引
--_Msg 玩家说话的内容
--_Color 说话的颜色
--_Channel 这个是频道
function QQCSA_MW_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)

	--这里检查玩家是否站在NPC面前
	if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then
		return ;
	end
	-- 玩家身上的魔王道具信物 用来交换奖励的
	PPS = {19396,19397,19398,19399,19400,19401,19402,19403};--魔王兑换道具ID NO1 NO2 NO3.....
	
-- NO1 基础设置
	NO1PET = 15; --NO1 奖励的宠物数量设置(根据个人情况改动)
	NO1ITEM = 18; --NO1 奖励的道具数量设置(根据个人情况改动)
  NO1NUM = NO1PET+NO1ITEM;--NO1的奖励数量

-- NO2 基础设置
  NO2PET = 22; --NO2 奖励的宠物数量设置(根据个人情况改动)
	NO2ITEM = 14; --NO2 奖励的道具数量设置(根据个人情况改动)
  NO2NUM = NO2PET+NO2ITEM;--NO2的奖励数量

-- NO3 基础设置
	NO3PET = 28; --NO3 奖励的宠物数量设置(根据个人情况改动)
	NO3ITEM = 17; --NO3 奖励的道具数量设置(根据个人情况改动)
  NO3NUM = NO3PET+NO3ITEM;--NO3的奖励数量

-- NO4 基础设置
	NO4PET = 25; --NO4 奖励的宠物数量设置(根据个人情况改动)
	NO4ITEM = 21; --NO4 奖励的道具数量设置(根据个人情况改动)
  NO4NUM = NO4PET+NO4ITEM;--NO4的奖励数量

-- NO5 基础设置
	NO5PET = 29; --NO5 奖励的宠物数量设置(根据个人情况改动)
	NO5ITEM = 14; --NO5 奖励的道具数量设置(根据个人情况改动)
  NO5NUM = NO5PET+NO5ITEM;--NO5的奖励数量
  
-- NO6 基础设置
	NO6PET = 31; --NO6 奖励的宠物数量设置(根据个人情况改动)
	NO6ITEM = 10; --NO6 奖励的道具数量设置(根据个人情况改动)
  NO6NUM = NO6PET+NO6ITEM;--NO6的奖励数量

-- NO7 基础设置
	NO7PET = 35; --NO7 奖励的宠物数量设置(根据个人情况改动)
	NO7ITEM = 25; --NO7 奖励的道具数量设置(根据个人情况改动)
  NO7NUM = NO7PET+NO7ITEM;--NO7的奖励数量
  
-- NO8 基础设置
	NO8PET = 10; --NO8 奖励的宠物数量设置(根据个人情况改动)
	NO8ITEM = 10; --NO8 奖励的道具数量设置(根据个人情况改动)
  NO8NUM = NO8PET+NO8ITEM;--NO8的奖励数量

---------NO1的设置 宠物 道具奖励  还有设置道具的几率--------------
  if(NLG.DelItem(_PlayerIndex,PPS[1],1) == true) then
  PET = {2561,2562,2563,2564,2680,2490,2492,2503,2504,2486,2487,2488,2489,2569,2570};--15个宠物奖励(根据个人情况改动)
  PETchance = {100,100,100,100,100,100,100,100,100,100,100,100,100,100,100};--每个宠物的几率(根据个人情况改动)
  ITEM = {10900,10901,10904,10915,10919,21179,20905,20914,20918,21014,21006,21019,21028,21029,21030,21031,21036,21038};--18个道具奖励(根据个人情况改动)
  ITEMchance = {50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50};--每个道具的几率(根据个人情况改动)
  PD = math.floor(math.random(1, NO1NUM));
  if ( PD <= NO1PET) then
	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO1PET));
  if (PETchance[JLP] >= JL) then
  if(NLG.GivePet( _PlayerIndex, PET[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "宠物栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end

  
  else 

	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO1ITEM));
  if (PETchance[JLP] >= JL) then
	if(NLG.GiveItem( _PlayerIndex , ITEM[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "道具栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end
  end
				NLG.ShowWindowTalked(_PlayerIndex, 0, 1, 1, "              【欢乐派派送】\n\n欢迎来到就爱上石器,请再接再厉，更好的奖品等待着你.",  _MeIndex);
	return ;
end






---------NO2的设置 宠物 道具奖励  还有设置道具的几率--------------
  if(NLG.DelItem(_PlayerIndex,PPS[2],1) == true) then
  PET = {2563,2564,2680,2490,2492,2503,2504,2486,2487,2488,2489,2569,2690,2681,2682,2671,2672,2673,2674,2675,2676,2677};--22个宠物奖励(根据个人情况改动)
  PETchance = {50,100,100,100,50,100,100,1,50,50,50,100,50,50,50,50,50,50,50,50,50,50};--每个宠物的几率(根据个人情况改动)
  ITEM = {10900,10901,21006,21019,21028,21029,21030,21031,21036,21038,21032,21033,21034,21035};--14个道具奖励(根据个人情况改动)
  ITEMchance = {100,50,50,50,100,100,100,100,100,100,50,50,50,50};--每个道具的几率(根据个人情况改动)
  PD = math.floor(math.random(1, NO2NUM));
  if ( PD <= NO2PET) then
	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO2PET));
  if (PETchance[JLP] >= JL) then
  if(NLG.GivePet( _PlayerIndex, PET[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "宠物栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end

  
  else 

	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO2ITEM));
  if (PETchance[JLP] >= JL) then
	if(NLG.GiveItem( _PlayerIndex , ITEM[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "道具栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end
  end
		NLG.ShowWindowTalked(_PlayerIndex, 0, 1, 1, "              【欢乐派派送】\n\n欢迎来到就爱上石器,请再接再厉，更好的奖品等待着你.",  _MeIndex);
	return ;
end







---------NO3的设置 宠物 道具奖励  还有设置道具的几率--------------
  if(NLG.DelItem(_PlayerIndex,PPS[3],1) == true) then
  PET = {2503,2504,2486,2487,2488,2489,2569,2690,2681,2682,2671,2672,2673,2674,2675,2676,20000,20001,20002,20003,20004,20005,20006,20007,20008,20009,20010,20011};--28个宠物奖励(根据个人情况改动)
  PETchance = {50,50,50,50,50,100,80,110,110,110,110,110,110,110,50,50,1,1,1,1,1,1,1,1,1,1,1,1};--每个宠物的几率(根据个人情况改动)
  ITEM = {19381,21230,21235,21240,21245,21250,21255,21260,21265,21270,21275,21280,21285,21290,21295,21300,21305};--17个道具奖励(根据个人情况改动)
  ITEMchance = {50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50};--每个道具的几率(根据个人情况改动)
  PD = math.floor(math.random(1, NO3NUM));
  if ( PD <= NO3PET) then
	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO3PET));
  if (PETchance[JLP] >= JL) then
  if(NLG.GivePet( _PlayerIndex, PET[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "宠物栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end

  
  else 

	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO3ITEM));
  if (PETchance[JLP] >= JL) then
	if(NLG.GiveItem( _PlayerIndex , ITEM[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "道具栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end
  end
		NLG.ShowWindowTalked(_PlayerIndex, 0, 1, 1, "              【欢乐派派送】\n\n欢迎来到就爱上石器,请再接再厉，更好的奖品等待着你.",  _MeIndex);
	return ;
end






---------NO4的设置 宠物 道具奖励  还有设置道具的几率--------------
  if(NLG.DelItem(_PlayerIndex,PPS[4],1) == true) then
  PET = {2503,2504,2486,2487,2488,2489,2569,2690,2681,2682,2671,2672,2673,2674,2675,2676,20000,20001,20002,20003,20004,20005,20006,24040,24041};--25个宠物奖励(根据个人情况改动)
  PETchance = {50,50,50,100,100,100,100,100,100,100,100,100,100,100,100,50,50,50,1,1,1,1,1,1,1,1,1,1,1,1};--每个宠物的几率(根据个人情况改动)
  ITEM = {30030,30031,30032,30033,30034,30035,30036,30037,30038,30039,30040,30041,30042,30043,30044,30045,30046,30047,30048,30049,27713};--21个道具奖励(根据个人情况改动)
  ITEMchance = {50,1,70,70,70,70,70,70,50,50,50,50,50,70,70,70,70,50,50,50,50,50};--每个道具的几率(根据个人情况改动)
  PD = math.floor(math.random(1, NO4NUM));
  if ( PD <= NO4PET) then
	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO4PET));
  if (PETchance[JLP] >= JL) then
  if(NLG.GivePet( _PlayerIndex, PET[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "宠物栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end

  
  else 

	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO4ITEM));
  if (PETchance[JLP] >= JL) then
	if(NLG.GiveItem( _PlayerIndex , ITEM[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "道具栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end
  end
		NLG.ShowWindowTalked(_PlayerIndex, 0, 1, 1, "              【欢乐派派送】\n\n欢迎来到就爱上石器,请再接再厉，更好的奖品等待着你.",  _MeIndex);
	return ;
end






---------NO5的设置 宠物 道具奖励  还有设置道具的几率--------------
  if(NLG.DelItem(_PlayerIndex,PPS[5],1) == true) then
  PET = {2489,2569,2690,2681,2682,2671,2672,2673,2674,2675,2676,20000,20001,20002,20003,20004,20005,20006,24042,24043,24045,24046,24047,24048,24049,24070,24071,24072,24073};--29个宠物奖励(根据个人情况改动)
  PETchance = {100,1,1,1,1,1,100,100,100,100,100,100,100,100,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};--每个宠物的几率(根据个人情况改动)
  ITEM = {22932,22933,22934,22935,22936,50054,50055,50056,50057,50058,27707,27708,27709,27710};--14个道具奖励(根据个人情况改动)
  ITEMchance = {50,50,50,50,50,50,1,1,1,1,1,1,1,1};--每个道具的几率(根据个人情况改动)
  PD = math.floor(math.random(1, NO5NUM));
  if ( PD <= NO5PET) then
	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO5PET));
  if (PETchance[JLP] >= JL) then
  if(NLG.GivePet( _PlayerIndex, PET[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "宠物栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end

  
  else 

	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO5ITEM));
  if (PETchance[JLP] >= JL) then
	if(NLG.GiveItem( _PlayerIndex , ITEM[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "道具栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end
  end
		NLG.ShowWindowTalked(_PlayerIndex, 0, 1, 1, "              【欢乐派派送】\n\n欢迎来到就爱上石器,请再接再厉，更好的奖品等待着你.",  _MeIndex);
	return ;
end






---------NO6的设置 宠物 道具奖励  还有设置道具的几率--------------
  if(NLG.DelItem(_PlayerIndex,PPS[6],1) == true) then
  PET = {2675,2676,20000,20001,20002,20003,20004,20005,20006,24042,24043,24074,24075,24076,24077,24078,27079,24080,24081,24082,24083,24084,24085,24086,24087,24088,24089,24090,24091,24092,24093};--31个宠物奖励(根据个人情况改动)
  PETchance = {1,100,100,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};--每个宠物的几率(根据个人情况改动)
  ITEM = {33304,33295,33300,33301,33302,33303,23362,23363,23364,23365};--10个道具奖励(根据个人情况改动)
  ITEMchance = {80,80,80,1,1,1,1,1,1,1};--每个道具的几率(根据个人情况改动)
  PD = math.floor(math.random(1, NO6NUM));
  if ( PD <= NO6PET) then
	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO6PET));
  if (PETchance[JLP] >= JL) then
  if(NLG.GivePet( _PlayerIndex, PET[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "宠物栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end

  
  else 

	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO6ITEM));
  if (PETchance[JLP] >= JL) then
	if(NLG.GiveItem( _PlayerIndex , ITEM[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "道具栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end
  end
		NLG.ShowWindowTalked(_PlayerIndex, 0, 1, 1, "              【欢乐派派送】\n\n欢迎来到就爱上石器,请再接再厉，更好的奖品等待着你.",  _MeIndex);
	return ;
end










---------NO7的设置 宠物 道具奖励  还有设置道具的几率--------------
  if(NLG.DelItem(_PlayerIndex,PPS[7],1) == true) then
  PET = {24074,24075,24076,24077,24078,27079,24080,24081,24082,24083,24084,24085,24086,24087,24088,24089,24090,24091,24092,24093,24042,24043,24045,24046,24047,24048,24049,24070,24071,24072,24073,24090,24091,24092,24093};--35个宠物奖励(根据个人情况改动)
  PETchance = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};--每个宠物的几率(根据个人情况改动)
  ITEM = {33304,33295,33300,33301,33302,33303,23362,23363,23364,23365,33239,33209,33210,33211,33212,33213,33214,33215,33216,33217,33218,33219,33220,33221,33222};--25个道具奖励(根据个人情况改动)
  ITEMchance = {1,100,80,1,1,1,1,100,100,100,100,100,100,1,100,100,100,100,1,1,100,100,100,1,80};--每个道具的几率(根据个人情况改动)
  PD = math.floor(math.random(1, NO7NUM));
  if ( PD <= NO7PET) then
	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO7PET));
  if (PETchance[JLP] >= JL) then
  if(NLG.GivePet( _PlayerIndex, PET[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "宠物栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end

  
  else 

	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO7ITEM));
  if (PETchance[JLP] >= JL) then
	if(NLG.GiveItem( _PlayerIndex , ITEM[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "道具栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end
  end
		NLG.ShowWindowTalked(_PlayerIndex, 0, 1, 1, "              【欢乐派派送】\n\n欢迎来到就爱上石器,请再接再厉，更好的奖品等待着你.",  _MeIndex);
	return ;
end






---------NO8的设置 宠物 道具奖励  还有设置道具的几率--------------
  if(NLG.DelItem(_PlayerIndex,PPS[8],1) == true) then
  PET = {24074,24075,24076,24077,24078,27079,24080,24081,24082,24083};--10个宠物奖励(根据个人情况改动)
  PETchance = {100,100,10,10,10,10,10,10,10,10};--每个宠物的几率(根据个人情况改动)
  ITEM = {33304,33295,33300,33301,33302,33303,23362,23363,23364,23365};--10个道具奖励(根据个人情况改动)
  ITEMchance = {100,100,10,10,10,10,10,10,10,10};--每个道具的几率(根据个人情况改动)
  PD = math.floor(math.random(1, NO8NUM));
  if ( PD <= NO8PET) then
	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO8PET));
  if (PETchance[JLP] >= JL) then
  if(NLG.GivePet( _PlayerIndex, PET[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "宠物栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end

  
  else 

	for iii=1,10000 do
  JL = math.floor(math.random(1, 100));
  JLP = math.floor(math.random(1, NO8ITEM));
  if (PETchance[JLP] >= JL) then
	if(NLG.GiveItem( _PlayerIndex , ITEM[JLP]) < 1)then
	NLG.TalkToCli(_PlayerIndex, "道具栏位不足!", %白色%, 0, -1);
	break;
	else break;
	end
  end
  end
  end
		NLG.ShowWindowTalked(_PlayerIndex, 0, 1, 1, "              【欢乐派派送】\n\n欢迎来到就爱上石器,请再接再厉，更好的奖品等待着你.",  _MeIndex);
	return ;
end

				NLG.ShowWindowTalked(_PlayerIndex, 0, 1, 1, "              【欢乐派派送】\n\n快来参加派派送哦!欢乐魔王大奖励。",  _MeIndex);
end