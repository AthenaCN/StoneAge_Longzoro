function LrWarp_Init( _MeIndex ) 

--以下设定NPC相关内容

--设置NPC形象 

Char.SetData(_MeIndex, %对像_形象%, 16130);

--设置NPC所在地图

Char.SetData(_MeIndex, %对像_地图%, 2000);

--设置X坐标

Char.SetData(_MeIndex, %对像_X%, 67);

--设置Y坐标

Char.SetData(_MeIndex, %对像_Y%, 50); 

--设置NPC方向

Char.SetData(_MeIndex, %对像_方向%, %右下%);

--设置NPC名称

Char.SetData(_MeIndex, %对像_原名%, "群体练级传送");

--响应说话事件，此事件如果有玩家对此NPC说话就会触发

if (Char.SetTalkedEvent(nil, "LrWarp_Talked", _MeIndex) < 0) then

  print("Talked 注册事件失败。");

  return false;

end

--窗口事件

if (Char.SetWindowTalkedEvent(nil, "LrWarp_WindowTalked", _MeIndex) < 0) then

  print("WindowTalked 注册事件失败。");

  return false;

end

--下方阵列设定传送位置 设定范例 {"位置名称", 地图编号, X, Y}
--下方第一个-1代表等级，第二个-1是旗标

lrwarppoint = { 

{"2 0级乌　龟", 10001,40,3,-1,-1},

{"4 0级海主人", 31401,92,27,-1,-1},

{"6 0级布伊区", 31705,26,29,-1,-1},

{"8 0级大象区", 300,81,270,-1,-1},

{"100级蓝雷区", 32018,12,34,-1,-1},

{"120级雷　龙", 7402,155,75,-1,-1},

{"140级金暴区", 60054,42,50,-1,-1},

{"140级地雷区", 60050,16,17,-1,-1},

{"140英雄岛", 500,230,321,-1,-1},

{"160级泪之海", 817,24,116,-1,-1},

{"伊甸园", 7000,35,128,-1,-1},

{"亚伊欧", 7400,99,90,-1,-1},

{"弗烈顿", 7300,87,101,-1,-1},

{"尼克斯", 7200,37,33,-1,-1},

{"塔耳塔", 7112,18,6,-1,-1},

{"漆黑过机暴", 32021,31,31,-1,-1}};

NLG.UpChar(_MeIndex); 

return true; 

end

function LrShowDig( _MeIndex, _PlayerIndex, page, maxpage) --窗口显示内容设置 page为第几页 maxpage为总页数

token = "2\n　　　　　　　　　‘我是懒人传送师！’\n懒鬼你要去那里呢.可以支持团队传送哦\n";

for i = 1,6 do

  if (i + ( page - 1 ) * 6) > table.getn(lrwarppoint) then

   break;

  end

token = token .. "　　　　　　　" .. lrwarppoint[i + (page -1 ) * 6][1] .. "\n" ;

end

if maxpage == 1 then

  button = %按钮_确定%;

elseif page == 1 and page <maxpage then

  button = %按钮_下一页%;

elseif page > 1 and page <maxpage then

  button = 48 ;

elseif page == maxpage then

  button = 16 ;

end



NLG.ShowWindowTalked( _PlayerIndex, 2, button, page,  token, _MeIndex); --对客户端发送窗口的封包

return;

end

function LrWarp_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)

  --这里检查玩家是否站在NPC面前

if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then

  return ;

end

LrShowDig( _MeIndex, _PlayerIndex, 1, 3) ;

return;

end

function LrWarp_WindowTalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)


if _Select == 0 then

  num = tonumber(_Data) + ( _Seqno - 1) * 6 ;
  
  local TM_level = lrwarppoint[num][5];
  local TM_ev = lrwarppoint[num][6];
  
  local cnt = 1;
  local TMP_Index;
  local warp_flg = 0;
  while (cnt < 5) do
		TMP_Index = Char.GetTeamIndex(_TalkIndex,cnt);
		if (TMP_Index > 0) then
			if (Char.GetData(TMP_Index,%对像_等级%)<TM_level) then
				warp_flg = 1;
				break;
			end
			if (TM_ev>-1) then
				if (Char.IsEventEnd(TMP_Index,TM_ev)==false) then
					return;
				end
			end
		end
		cnt = cnt + 1;
	end
	if (warp_flg==0) then
		cnt = 1;
  	while (cnt < 5) do
			TMP_Index = Char.GetTeamIndex(_TalkIndex,cnt);
			if (TMP_Index > 0) then
				NLG.Warp(TMP_Index,lrwarppoint[num][2],lrwarppoint[num][3],lrwarppoint[num][4]);
				--NLG.UpChar(_MeIndex); 
			end
			cnt = cnt + 1;
		end
	end
	
	if (warp_flg==1) then
		NLG.TalkToCli(_TalkIndex,"条件不足！");
		return;
	end
	if (Char.GetData(_TalkIndex,%对像_等级%)<TM_level) then
		NLG.TalkToCli(_TalkIndex,"条件不足！");
		return;
	end
	if (TM_ev>-1) then
		if (Char.IsEventEnd(_TalkIndex,TM_ev)==false) then
			NLG.TalkToCli(_TalkIndex,"条件不足！");
			return;
		end
	end
	NLG.Warp(_TalkIndex,lrwarppoint[num][2],lrwarppoint[num][3],lrwarppoint[num][4]);

elseif _Select == 16 then

  LrShowDig( _MeIndex, _TalkIndex, _Seqno - 1, 3) ;

elseif _Select == 32 then

  LrShowDig( _MeIndex, _TalkIndex, _Seqno + 1, 3) ;

end

return;

end