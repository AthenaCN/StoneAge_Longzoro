function hlctrl_Init( _MeIndex ) 
 	--设置NPC形象
 	Char.SetData(_MeIndex, %对像_形象%, 100000); 
 	Char.SetData(_MeIndex, %对像_原形%, 100000); 
 	--设置NPC所在地图
 	Char.SetData(_MeIndex, %对像_地图%, 2000); 
 	--设置X坐标
 	Char.SetData(_MeIndex, %对像_X%, 78); 
 	--设置Y坐标
 	Char.SetData(_MeIndex, %对像_Y%, 47); 
 	--设置NPCよ
 	Char.SetData(_MeIndex, %对像_方向%, %左下%); 
 	--设置NPC名称
 	Char.SetData(_MeIndex, %对像_原名%, "『轮回圣炉』"); 
 	--设置回圈事件
	if (Char.SetTalkedEvent(nil, "hl_Talked", _MeIndex) < 0) then
		print("hl_Talked 註冊事件失敗。");
		return false;
	end
	
		if (Char.SetWindowTalkedEvent(nil, "hl_WindowTalked", _MeIndex) < 0) then
		print("hl_WindowTalked 註冊事件失敗。");
		return false;
	end
 	return true; 
 end 
 
function hl_Talked( _MeIndex, _PlayerIndex, _Msg, _Color, _Channel)
	
    local npcname= Char.GetData(_MeIndex, %对像_原名%)
	NLG.ShowWindowTalked(_PlayerIndex, 2, 0, 1,
	 				"7\n　　　　           "..npcname..""..
					 "\n"..
					 "\n1、请将宠物放置宠物栏第一格"..
					 "\n2、宠物栏位置以外挂上的为准"..
					 "\n3、目前只开放四圣兽进行轮回"..
					 "\n4、提供声望和贝币货币来轮回"..
					 "\n"..			 
					 "\n             贝币轮回(需 100 贝币)"..
					 "\n             声望轮回(需10000声望)",
	 				_MeIndex);
	end

function hl_WindowTalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)
local jb=Char.GetData(_TalkIndex, %对像_声望%);
local sw=Char.GetData(_TalkIndex, %对像_声望%);
local status=0;
petid=Char.GetPetEnemyId(_TalkIndex,%宠栏_1%);

	if(petid == 777)then
	id=1610;
	status=1;
	end
	
	if(petid == 918)then
		id=1845;
	status=1;
	end

	if(petid == 1140)then
		id=2474;
	status=1;
	end
	
	if(petid == 933)then
		id=2258;
	status=1;
	end

	if(_Seqno == 1 and status == 1)then
				if(_Data == "1")then
					if(jb>99)then
					Char.SetData(_TalkIndex, %对像_声望%,Char.GetData(_TalkIndex, %对像_声望%)-100);
					NLG.DelPet(_TalkIndex, petid); 
					NLG.GivePet(_TalkIndex, id); 
					status=0;
					NLG.TalkToCli( _TalkIndex, "恭喜你！你的圣兽轮回成功！", %黄色%, 0, _MeIndex);
					NLG.TalkToCli( _TalkIndex, "已扣除您的贝币币数 100 个。", %红色%, 0);
	                NLG.TalkToCli( _TalkIndex, "你目前的个人贝币数为：" .. tostring(Char.GetData( _TalkIndex, %对像_声望%)) .. " ", %青色%, 0);
					else
					NLG.TalkToCli( _TalkIndex, "你的贝币数不足100！", %红色%, 0, _MeIndex);
					end
				end
				if(_Data == "2")then
					if(sw>9999*100)then
					Char.SetData(_TalkIndex, %对像_声望%,Char.GetData(_TalkIndex, %对像_声望%)-10000*100);
					petid=Char.GetPetEnemyId(_TalkIndex,%宠栏_1%);
					NLG.DelPet(_TalkIndex, petid); 
					NLG.GivePet(_TalkIndex, id); 
					status=0;
					NLG.TalkToCli( _TalkIndex, "回炉成功！", %黄色%, 0, _MeIndex);
					NLG.TalkToCli( _TalkIndex, "已扣除您的声望数 10000 点。", %红色%, 0);
	                NLG.TalkToCli( _TalkIndex, "你目前的个人声望点数为：" .. Char.GetData( _TalkIndex, %对像_声望%)/100 .. " ", %绿色%, 0); 
					else
							NLG.TalkToCli( _TalkIndex, "你的声望数不足10000！", %红色%, 0, _MeIndex);
					end
				end
				
	else
				NLG.TalkToCli( _TalkIndex, "目前只支持四圣兽的回炉", %红色%, 0, _MeIndex);
	end
end