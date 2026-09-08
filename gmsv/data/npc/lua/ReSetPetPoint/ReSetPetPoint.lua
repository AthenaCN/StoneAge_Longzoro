function ReSetPetPoint_Init( _MeIndex ) 
    Char.SetData(_MeIndex, %对像_形象%, 101157); 
 	Char.SetData(_MeIndex, %对像_原形%, 101157); 
 	Char.SetData(_MeIndex, %对像_地图%, 2000); 
 	Char.SetData(_MeIndex, %对像_X%, 51); 
 	Char.SetData(_MeIndex, %对像_Y%, 77); 
 	Char.SetData(_MeIndex, %对像_方向%, %右下%); 
 	Char.SetData(_MeIndex, %对像_原名%, "宠物工具"); 
	
	if (Char.SetTalkedEvent(nil, "ReSetPetPoint_Talked", _MeIndex) < 0) then
		print("ReSetPetPoint_Talked 註冊事件失敗。");
		return false;
	end
	
	if (Char.SetWindowTalkedEvent(nil, "ReSetPetPoint_WindowTalked", _MeIndex) < 0) then
		print("ReSetPetPoint_WindowTalked 註冊事件失敗。");
		return false;
	end
 	return true; 
 end 
 
function ReSetPetPoint_Talked( _MeIndex, _TalkIndex, _Msg, _Color, _Channel)
  if NLG.CheckInFront(_TalkIndex, _MeIndex, 1) == false then   
    return; 
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
     NLG.ShowWindowTalked(_TalkIndex, 1, 3, 1, 
                        "               「宠物工具」"..
                        "\n"..
                        "\n1、宠物工具可以自定义您所需要的宠物点数"..
                        "\n2、设置成功以后您的宠物自动变成1转140级"..
                        "\n3、宠物实际血攻防敏需按照后面的计算公式"..		
                        "\n4、宠物位置(1~5)"..
                        "\n"..
                        "\n请输入要设置的宠物位置",
			        _MeIndex); 

end

function ReSetPetPoint_WindowTalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)
     --  petset_pos = tonumber(_Data) - 1;
    if _Seqno ==1 then	
	    --设置转数
        Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_转数%, 3);
	    --设置等级
        Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_等级%, 170);
        NLG.ShowWindowTalked(_TalkIndex, 1, 3, 2, 
		                "\n每点体力："..
		                "\n增加  4  点耐久力"..
		                "\n增加 0.1 点攻击力"..					
		                "\n增加 0.1 点防御力"..
		                "\n增加  0  点敏捷力"..
                        "\n"..	
                        "\n请输入宠物体力：",   
					 _MeIndex); 
    elseif _Seqno == 2 then
	    Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_体力%, tonumber(_Data)*100);
        NLG.ShowWindowTalked(_TalkIndex, 1, 3, 3, 
		                "\n每点体力："..
		                "\n增加  1  点耐久力"..
		                "\n增加  1  点攻击力"..					
		                "\n增加 0.1 点防御力"..
		                "\n增加  0  点敏捷力"..	
                        "\n"..	   
                        "\n请输入宠物腕力：",
					_MeIndex); 

    elseif _Seqno == 3 then
	    Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_腕力%, tonumber(_Data)*100);
        NLG.ShowWindowTalked(_TalkIndex, 1, 3, 4, 
		                "\n每点体力："..
		                "\n增加  4  点耐久力"..
		                "\n增加 0.1 点攻击力"..					
		                "\n增加  1  点防御力"..
		                "\n增加  0  点敏捷力"..
                        "\n"..   
                        "\n请输入宠物耐力：",
					_MeIndex); 

    elseif _Seqno == 4 then
	    Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_耐力%, tonumber(_Data)*100);
        NLG.ShowWindowTalked(_TalkIndex, 1, 3, 5, 
		                "\n每点体力："..
		                "\n增加  1   点耐久力"..
		                "\n增加 0.05 点攻击力"..					
		                "\n增加 0.05 点防御力"..
		                "\n增加  1   点敏捷力"..
                        "\n".. 
                        "\n请输入宠物速度：",
					 _MeIndex); 

    elseif _Seqno == 5 then
	    Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_速度%, tonumber(_Data)*100);
    end
end 
