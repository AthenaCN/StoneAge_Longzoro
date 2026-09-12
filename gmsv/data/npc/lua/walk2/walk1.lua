function walk_Init( _MeIndex )
local walkx = math.floor(math.random(42, 61));
local walky = math.floor(math.random(71, 83));
 	--设置NPC形象
 	Char.SetData(_MeIndex, %对像_形象%, 100821);
 	Char.SetData(_MeIndex, %对像_原形%, 100821);

 	--设置NPC所在地图
 	Char.SetData(_MeIndex, %对像_地图%, 2000);

 	--设置X坐标
 	Char.SetData(_MeIndex, %对像_X%, walkx);

 	--设置Y坐标
 	Char.SetData(_MeIndex, %对像_Y%, walky);

 	--扢离NPC方向
 	Char.SetData(_MeIndex, %对像_方向%, %右下%);

 	--设置NPC名称
 	Char.SetData(_MeIndex, %对像_原名%, "猪小弟");

	 --初始位置全域变数
	Timer = 1;
 	--设置回圈事件
 	if (Char.SetLoopEvent(nil, "walking_walk1", _MeIndex,1000) < 0) then
 		print("walking_walk注册事件失败。");
 		return false;
 	end


  --設置踩踏回應事件

 		if (Char.SetPostOverEvent(nil, "walk_Covered", _MeIndex) < 0) then
			print("walk_Covered 注册事件失败。");
			return false;
   end


 	--更新
 	NLG.UpChar(_MeIndex);

 	return true;
 end

   function walk_Covered( _MeIndex,_PlayerIndex)
   if Char.GetData(_PlayerIndex,%对像_形象%) == 100250 then
	  	--NLG.Warp(_PlayerIndex,2000,32,28);
	 		NL.DelNpc(_MeIndex);
	 		counter=counter-1;
	 		pcounter=Char.GetData(_PlayerIndex,%对像_声望%);
	 		Char.SetData(_PlayerIndex,%对像_声望%,pcounter+1);
	 		vip=pcounter+1;
 	--	NLG.TalkToCli(_PlayerIndex, "[抓猪活动]捉到一只，奖励会员点1点，目前共奖励点数"..fame.."点！", %黄色%, 0, -1);
	 	 NLG.TalkToCli( _PlayerIndex, "[抓猪活动]捉到一只，奖励会员点1点，目前共奖励点数：" .. tostring(Char.GetData( _PlayerIndex, %对像_声望%)) .. " ", %青色%, 0);
	 	else
 		NLG.TalkToCli(_PlayerIndex, "?...!...@@....我们不认识你！请变成小猪造型！！", %红色%, 0, -1);
	 	end
	end
 function walking_walk1( _MeIndex)
 	--设置动作为走路
 	local walk = 1;
 	NLG.SetAction(_MeIndex,walk);
 	--随机
 	local TM_DIR = math.floor(math.random(0, 7));

 	--走路
 	if(NLG.WalkMove(_MeIndex,TM_DIR) ~= 0) then
 		NLG.WalkMove(_MeIndex,math.floor(math.random(0, 16)));
 	end
 	--扢离向
 	NLG.CharLook(_MeIndex,TM_DIR);
 	return ;
 end
