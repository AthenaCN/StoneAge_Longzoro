function Pet_Init( _MeIndex ) 
    Char.SetData(_MeIndex, %对像_形象%, 101157); 
 	Char.SetData(_MeIndex, %对像_原形%, 101157); 
 	Char.SetData(_MeIndex, %对像_地图%, 2000); 
 	Char.SetData(_MeIndex, %对像_X%, 53); 
 	Char.SetData(_MeIndex, %对像_Y%, 86); 
 	Char.SetData(_MeIndex, %对像_方向%, %右下%); 
 	Char.SetData(_MeIndex, %对像_原名%, "宠物四围"); 
	
	if (Char.SetTalkedEvent(nil, "Pet_Talked", _MeIndex) < 0) then
		print("Pet_Talked 註冊事件失敗。");
		return false;
	end
	
	if (Char.SetWindowTalkedEvent(nil, "Pet_WindowTalked", _MeIndex) < 0) then
		print("Pet_WindowTalked 註冊事件失敗。");
		return false;
	end
 	return true; 
 end 

 
function Pet_Talked( _MeIndex, _TalkIndex, _Msg, _Color, _Channel)
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
                        "\n1、固定把宠物栏1的宠物变成JP宠"..
                        "\n2、设置成功以后您的宠物自动变成3转170级"..
                        "\n3、四围：3400 722 499 700"..		
                        "\n4、宠物位置(1~5)"..
                        "\n"..
                        "\n请输入要设置的宠物位置",
			        _MeIndex); 


end
function Pet_WindowTalked( _MeIndex, _TalkIndex, _Seqno, _Select, _Data)
     --  petset_pos = tonumber(_Data) - 1;
    if _Seqno ==1 then	
	    --设置转数
        Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_转数%, 3);
	    --设置等级
        Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_等级%, 170);
        
    
	    Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_腕力%, 612*100);
       Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_体力%, 400*100);
       Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_耐力%, 300*100);
       Char.SetData( Char.GetPetIndex(_TalkIndex,petset_pos), %对像_速度%, 700*100);
       

   end 
end 
