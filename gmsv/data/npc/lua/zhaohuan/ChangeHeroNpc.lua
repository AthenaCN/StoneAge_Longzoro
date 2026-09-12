ChangeHeroNpc_Init = function(l_1_0)
  Char.SetData(l_1_0, 1, ChangeHeroCf.ImageNo)
  Char.SetData(l_1_0, 2, ChangeHeroCf.ImageNo)
  Char.SetData(l_1_0, 4, ChangeHeroCf.Floor)
  Char.SetData(l_1_0, 5, ChangeHeroCf.X)
  Char.SetData(l_1_0, 6, ChangeHeroCf.Y)
  Char.SetData(l_1_0, 7, 5)
  Char.SetData(l_1_0, 2000, ChangeHeroCf.Name)
  Char.SetTalkedEvent(nil, "ChangeHeroNpc_Talked", l_1_0)
  Char.SetWindowTalkedEvent(nil, "ChangeHeroNpc_TalkAction", l_1_0)
  Char.SetLoopEvent(nil, "ChangeHeroNpc_Loop", l_1_0, ChangeHeroCf.Floor)
  return true
end

ChangeHeroNpc_Talked = function(l_2_0, l_2_1, l_2_2, l_2_3, l_2_4)
  local l_2_5 = ""
  if NLG.CheckInFront(l_2_1, l_2_0, 1) == false then
    return
  end
  l_2_5 = "2" .. "\n我可以帮您完成勇者任务:" .. "\n" .. "\n[1] 完成任务" .. "\n[2] 网站说明"
  NLG.ShowWindowTalked(l_2_1, 2, 1, 0, l_2_5, l_2_0)
  return
end

ChangeHeroNpc_TalkAction = function(l_3_0, l_3_1, l_3_2, l_3_3, l_3_4)
  local l_3_5 = ""
  if l_3_2 == 0 then
    if l_3_3 == 1 then
      return
    end
    if l_3_4 == "1" then
      local l_3_6 = Char.GetData(l_3_1, 146)
      if l_3_6 > 0 then
        NLG.ShowWindowTalked(l_3_1, 0, 1, 9, "\n对不起,您已经完成勇者任务,\n不需要我的帮忙了!", l_3_0)
      end
      if l_3_6 <= 0 and Char.SetData(l_3_1, 146, 1) ~= nil then
        NLG.ShowWindowTalked(l_3_1, 0, 1, 9, "恭喜你完成勇者任务!", l_3_0)
      elseif l_3_4 == "2" then
        l_3_5 = "\n 我可以自动帮您完成勇者任务!" .. "\n 作者:龙哥" .. "\n QQ:957724732" .. "\n 网址:www.shiqiol.com" .. "\n欢迎玩家发布自己的个性任务" .. "\n 更多精彩尽在就爱上石器!"
        NLG.ShowWindowTalked(l_3_1, 0, 1, 9, l_3_5, l_3_0)
      end
    end
  end
end
