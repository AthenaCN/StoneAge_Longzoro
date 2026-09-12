function Warp_Init( _MeIndex )

--以下设定NPC相关内容

--设置NPC形象

Char.SetData(_MeIndex, `对象_形象`, 16146);

--设置NPC所在地图

Char.SetData(_MeIndex, `对象_地图`, 2000);

--设置X坐标

Char.SetData(_MeIndex, `对象_X`, 62);

--设置Y坐标

Char.SetData(_MeIndex, `对象_Y`, 60);

--设置NPC方向

Char.SetData(_MeIndex, `对象_方向`, `左下`);

--设置NPC名称

Char.SetData(_MeIndex, `对象_原名`, "团队练级传送");

--响应说话事件，此事件如果有玩家对此NPC说话就会触发

if (Char.SetTalkedEvent(nil, "Warp_Talked", _MeIndex) < 0) then

  print("Talked 注册事件失败。");

  return false;

end

--窗口事件

if (Char.SetWindowTalkedEvent(nil, "Warp_WindowTalked", _MeIndex) < 0) then

  print("WindowTalked 注册事件失败。");

  return false;

end

--下方阵列设定传送位置 设定范例 {"位置名称", 地图编号, X, Y}

warppoint = {
{"20级  阿布洞窟1楼", 10001,10,10},

{"40级  海主人", 31401,110,10},

{"单挂60 猜谜之家", 810,29,20},

{"单挂80 猜谜之家", 808,29,20},

{"单挂120 蛇王巢穴", 9356,10,10},

{"70级  无名地下1楼", 30301,18,35},

{"80级  花的世界", 300,158,229},

{"100级 英雄岛海边", 500,230,328},

{"清风石器活动中心", 9002,14,4},

{"劳动节-宠物乐园", 9001,25,26}};
NLG.UpChar(_MeIndex);

return true;

end

function ShowDig( _MeIndex, _PlayerIndex, page, maxpage) --窗口显示内容设置 page为第几页 maxpage为总页数

token = "2\n　　　　　　　　　‘我是练级传送师！’\n请问您要去哪里练级？支持团体传送的哦!\n";

for i = 1,6 do

  if (i + ( page - 1 ) * 6) > table.getn(warppoint) then

   break;

  end

token = token .. "　　　　　　　" .. warppoint[i + (page -1 ) * 6][1] .. "\n" ;

end

if maxpage == 1 then

  button = `按钮_确定`;

elseif page == 1 and page <maxpage then

  button = `按钮_下一页`;

elseif page > 1 and page <maxpage then

  button = 48 ;

elseif page == maxpage then

  button = 16 ;

end



NLG.ShowWindowTalked( _PlayerIndex, 2, button, page,  token, _MeIndex); --对客户端发送窗口的封包

return;

end

function Warp_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)

  --这里检查玩家是否站在NPC面前

if(NLG.CheckInFront(_PlayerIndex, _MeIndex, 1) == false) then

  return ;

end

ShowDig( _MeIndex, _PlayerIndex, 1, 3) ;

return;

end

function Warp_WindowTalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)


if _Select == 0 then

  num = tonumber(_Data) + ( _Seqno - 1) * 6 ;

  local cnt = 1;
  local TMP_Index;
  while (cnt < 5) do
		TMP_Index = Char.GetTeamIndex(_TalkIndex,cnt);
		if (TMP_Index > 0) then
			NLG.Warp(TMP_Index,warppoint[num][2],warppoint[num][3],warppoint[num][4]);
			--NLG.UpChar(_MeIndex);
		end
		cnt = cnt + 1;
	end
	NLG.Warp(_TalkIndex,warppoint[num][2],warppoint[num][3],warppoint[num][4]);

elseif _Select == 16 then

  ShowDig( _MeIndex, _TalkIndex, _Seqno - 1, 3) ;

elseif _Select == 32 then

  ShowDig( _MeIndex, _TalkIndex, _Seqno + 1, 3) ;

end

return;

end
