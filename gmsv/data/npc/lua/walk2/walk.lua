function walkctrl_Init( _MeIndex )

 	--设置NPC形象
Char.SetData(_MeIndex, %对像_形象%, 101766);

 	--设置NPC所在地图
Char.SetData(_MeIndex, %对像_地图%, 2000);

 	--设置X坐标
 	Char.SetData(_MeIndex, %对像_X%, 35);

 	--设置Y坐标
 	Char.SetData(_MeIndex, %对像_Y%, 27);

 	--扢离NPC方向
 	Char.SetData(_MeIndex, %对像_方向%, %左下%);

 	--设置NPC名称
 	Char.SetData(_MeIndex, %对像_原名%, "富翁猪老大");

	 --初始位置全域变数
	Timer = 1;
	counter=0;
	counterindex={0};
 	--设置回圈事件
 	if (Char.SetLoopEvent(nil, "walking_walk", _MeIndex,1000) < 0) then
 		print("walking_walk注册事件失败。");
 		return false;
 	end
	if (Char.SetTalkedEvent(nil, "walk_WindowTalked", _MeIndex) < 0) then
		print("walk_WindowTalked 註冊事件失敗。");
	return false;
		end
 	--更新
 	NLG.UpChar(_MeIndex);

 	return true;
 end

 function walk_WindowTalked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	 	NLG.TalkToCli(_PlayerIndex, "每天晚上20点-21点举行活动！先用变幻衣服变成乌力造型后！踩在小蓝猪身上就OK了！", %蓝色%, 0, -1);
 end

 function walking_walk( _MeIndex)
  local TM_Hour = tonumber(os.date("%H", os.time()));
  		if(TM_Hour >= 02 and TM_Hour > 03) then
  			begin=1
  		else
  			begin=0
  		end
 	walking_loop(_MeIndex);
 	Timer=Timer+1
 	return ;
 end



 function walking_loop(_MeIndex)
 	 if math.mod(Timer,5)==0 and begin==1 then
 	 walk_Generate( _MeIndex);
 	NLG.TalkToCli(-1, "[捉猪通告]目前共有"..counter.."个猪小弟！", %蓝色%, 0, -1);

 	 end
end


 function walk_Generate( _MeIndex)
   	walkctrl = NL.CreateNpc("./data/npc/lua/walk2/walk1.lua", "walk_Init");
   	if walkctrl>=0 then
   	counter=counter+1;
 		counterindex[counter]=walkctrl;
 		print(counter);
 		end
 end
