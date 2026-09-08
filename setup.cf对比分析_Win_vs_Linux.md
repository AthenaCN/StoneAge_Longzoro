# Win版 vs Linux版 setup.cf 配置项对比分析

> Win版：`D:\石器时代9.0\服务端\gmsv\setup.cf`（9.0 一键端，商业化魔改）
> Linux版：`D:\StoneAge\gmsv\setup.cf`（龙zoro版 v2.2.2.8，**已按官方 8.0 校准，2026-09-08**）
> 官方参考：`librae8226/gmsv`（官方精简版）、`19900623/stoneage`（官方扩展版，与 Linux 版同源）
> ⚠️ 本文档"三、两者都有但值不同"表格为**校准后**的值；校准前值见"五、校准记录"。

---

## 一、Win版独有（Linux版完全没有的配置项）

### 1.1 玩家等级/出生相关
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `PlayerLimitLevel` | 170 | 玩家限制等级（Linux版无此项，用 MAXLEVEL 代替） |
| `BornPoint` | 0 | 出生点（0为随机） |
| `NORIDEID` | 27 | 乘坐要验证ID |

### 1.2 外转/ WD 系统
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `WDTrans` | 1 | 外转转数 |
| `WDLevels` | 140 | 外转等级 |
| `WDpay` | 1 | 使用货币（0=关闭 1=石币 2=点数 3=积分） |
| `WDpaymount` | 1000 | 外转费用 |

### 1.3 IP/安全限制
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `iprestrict` | 25 | 注册IP限制数量 |
| `CompelStore` | 1 | 强制存档（1=开启强制 0=关闭强制） |
| `AutoSave` | 1 | 掉线是否自动存档 |

### 1.4 宠物转生/融合
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `Pettranslimit` | 1000 | 宠物一转成长上限 |
| `Pettranslimit2` | 2000 | 宠物二转成长上限 |
| `AngelOpen` | 0 | 6转时是否取消精灵召唤判断 |
| `Ridets` | 1 | 是否开放1转以上乘坐 |

### 1.5 登录删除/不可继承
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `logindelitem` | 16个-1 | 登录删除指定ID物品（16个） |
| `logindelpet` | 16个-1 | 登录删除指定ID宠物（16个） |
| `UNHeritable` | 32个-1 | 不可继承的物品（32个） |

### 1.6 地图禁止系统（Win版用独立配置名）
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `rideforbid` | 1400,... | 禁止骑乘地图（16个） |
| `helpforbid` | 100,... | 禁止HELP地图（32个） |
| `featherforbid` | 100,... | 禁止使用羽毛地图（32个） |
| `joinforbid` | 10个-1 | 禁止加入组队地图（10个） |
| `backforbid` | 10个-1 | 禁止原登地图（10个） |
| `FLOOR1-5` | 32021,8213,... | 禁止玩家互相传送地图（5个） |

> Linux版用 `FLOOR1-5`（官方 8.0 键，2026-09-08 已补齐；原 `UNLAWWARPFLOOR` 是无效键已删除）。

### 1.7 事件/任务系统
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `endevent` | 100,200,... | 自动完成的事件（10个） |
| `nowevent` | 100,200,... | 自动进行中的事件（10个） |

### 1.8 幸运星系统（Win版配置名不同）
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `LuckyStarTime` | 0 | 幸运星触发时间（0=关闭） |
| `LuckyAward` | 15\|16788\|... | 幸运星奖品（1个5为设置最高的奖品数量） |
| `LuckyNum` | 1 | 每次触发赠送数量 |

> Linux版用 `LUCKSTARTIME`/`LUCKSTARCHANCES`。

### 1.9 镶嵌/VIP
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `InslayNum` | 5 | 自动镶嵌宝石孔数（默认3，可设1-5） |
| `VIPSERVER` | 0 | 是否包时会员专服 |
| `VIPPOINT` | 1 | 会员专服每小时扣除会员点数 |
| `VIPMAP0-9` | 空 | 会员地图（VIPSERVER=0时无效） |

### 1.10 LUA 脚本
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `luadofile` | ./data/npc/lua/init.lua | NPC LUA 初始化脚本 |

> Linux版用 `LUAFILE`/`ITEMLUAFILE`。

### 1.11 其他
| 配置项 | Win版值 | 说明 |
|--------|---------|------|
| `Eggfeed` | 40 | 蛋饲料（？） |
| `Proexp` | 100 | 职业经验倍率 |
| `gameservid` | 龙安石器 | 服务器ID（字符串，Linux版是数字1） |

---

## 二、Linux版独有（Win版没有的配置项）

### 2.1 声望/签到系统
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `NOFAMEMAP` | 60054 | 不给声望的地图 |
| `PETUPGETEXP` | 0 | 螺丝特有声望系统（0正常 1特色声望） |
| `REPORTITEM1-5` | -1 | 每日签到普通会员道具 |
| `VIPREPORTITEM1-5` | -1 | 每日签到VIP会员道具 |
| `reportlv` | 140 | 签到等级限制 |
| `reportta` | 5 | 签到转生数限制 |
| `SAVEFAME` | 1 | 每在线/SAVE保存一次所需声望 |

### 2.2 GM 保护
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `gm1-gm4` | 12345 | GM账号加强保护（需和gmset.txt对应） |

### 2.3 组队/经验
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `TEAMADDEXP` | 0 | 组队加成（0关闭，数字=几倍） |
| `EXPSHARE` | 100 | 团队经验共享（百分比） |

### 2.4 转生/属性
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `TRANS7POINT` | 64 | 7转比6转多的点数 |
| `NOTRANSPET` | 30个-1 | 禁止转生的宠物编号（最多30个） |
| `DEXPER` | 15 | 修正乱敏系数（官方30，15效果最好） |
| `PETTRANSABILITY` | 0 | 宠物转生后能力值增长 |
| `PETTRANSABILITY1-3` | 155/200/250 | 一转/二转/三转成长上限 |

### 2.5 交易/绑定
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `FirstLockItem` | 1,2,... | 道具第一次交易自动绑定（30个） |
| `dammagecalc` | 70 | 伤害程度设置（默认100） |

### 2.6 摆摊/频道
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `streetvendortrans` | 0 | 几转才可以摆摊 |
| `sameiponlinenum` | 10 | 同一IP登陆数 |
| `ALLSERVTRANS/LEVEL/SNED` | 5/140/1 | 使用星球频道所需声望/等级/声望 |
| `THEWORLDTRANS/LEVEL/SNED` | 5/140/1 | 使用世界频道所需声望/等级/声望 |

### 2.7 外挂防护
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `NOSTWENEMY` | 0 | 禁止外挂快速遇敌（0不改 1禁止 2自动原地遇敌） |
| `NOSTWENEMYGOLD` | 0 | 快速遇敌扣除石币 |
| `STWSENDTYPE` | 3 | 外挂瞬移损耗类型（-1关 0石币 1声望 2积分 3气势 4DP 5DP） |
| `STWSENDPOINT` | 100 | 瞬移所需损耗点 |
| `ITEMPETLOCKED` | 1 | 安全锁功能（0不使用） |

### 2.8 仓库/存储
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `POOLITEMBUG` | 1 | 道具仓库强存模式（0可强存 1共同不能 2共同可以 3禁止） |
| `POOLITEM` | 30个-1 | 仓库强存道具列表 |

### 2.9 PK/活动
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `AUTOPK` | 16 | 乱舞PK每天几点开始（-1不启用） |
| `AUTOPKTRANS` | 5 | 乱舞PK资格转数 |
| `AUTOPKLV` | 140 | 乱舞PK资格等级 |
| `AUTOPKPOINT` | 100 | 乱舞开始赠送积分 |
| `KILLPOINT` | 10 | 每杀一人获得积分 |
| `PKMAP` | 10,20,30,40 | 重回币PK地图扣点 |
| `PKMAPCOST` | 10 | 重回币PK每次胜利手续费 |
| `PLAYEROVERLAPPK` | 30个-1 | 玩家重叠时立即PK地图 |
| `MAPBATTLELOOP` | 20 | 循环挑战时间（几点开始，1小时） |

### 2.10 会员/上线提示
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `VIPBOUNDTIME` | 0 | 是否设为会员包时专区 |
| `UNBOUNDTIMETRANS` | 0 | 包时专区转数限制 |
| `UNBOUNDTIMELEVEL` | 120 | 包时专区等级限制 |
| `UNBOUNDTIMEMAP` | 2000,65,65 | 包时专区活动地图 |
| `LOGINDISPLAY` | 3 | 玩家上线提示（0不用 1登陆公告+会员 2仅公告 3仅会员） |

### 2.11 掉宝/遇敌
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `battlegetitemcf` | 0 | 当值为0时挂机方可掉东西 |
| `BATTLEGETITEMRATE` | -1 | 战斗获得物品机率（越前面机率越低） |
| `BATTLEGETITEMRATEMAP` | 0 | 战斗获得物品地图设置 |
| `DELNEEDITEM` | 0 | 抓宠所需物品是否自动删除 |
| `charlooptime` | 15 | char运行时间 |
| `battletime` | 50 | 战斗回合延迟（毫秒） |

### 2.12 其他
| 配置项 | Linux版值 | 说明 |
|--------|-----------|------|
| `LCSDV` | 86400 | 登录验证标准差值（默认1024） |
| `allowerrornum2` | 1 | 每个连接错误上限（超过关闭） |
| `serverip` | 空 | 分线IP |
| `nohelpmap` | 40001,... | 不可以HELP的地图（30个） |
| `vipbattleexp` | 1 | 会员经验倍数 |
| `CANCELANGLETRANS` | 1 | 六转是否取消精召任务判断 |
| `PETSKILLSHOPPATH` | data/ablua/... | 宠技商品LUA脚本路径 |
| `FIMALYPKTIME` | 20 | 自定义族战开片时间（次日几点，-1=下战书时间） |
| `NOTESCAPE` | 12345,... | 禁止逃跑的地图（30个） |
| `TALKCHECKMIN/MAX` | 50/100 | 战斗NPC对话验证答案范围 |
| `TRANSPOINTUP` | -1,... | 每转点数上限定义（-1=默认） |
| `UNLAWTHISLOGOUT` | 142,... | 禁止原登地图（30个） |
| `NOJOINFLOOR` | 1042,... | 禁止组队地图（30个） |
| `SPECIALMAP` | -1,... | 特殊地图（四张） |
| `STREETVENDORPOINT` | 0,5,5,5,5,5 | 摆摊税收费用（%） |
| `LUAFILE/ITEMLUAFILE` | data/npc/lua/... | LUA脚本路径（Win版用luadofile） |
| ~~petskill2file/setpetskillfile~~ | ~~data/petskill3.txt~~ | **无效键，已删除**（8.0 真键是 petskillfile2，指向存在的 petskill2.txt） |

---

## 三、两者都有但值不同的关键配置（Linux 版为 2026-09-08 校准后值）

| 配置项 | Win版(魔改) | Linux版(官方校准后) | 说明 |
|--------|-------|---------|----------|
| `debuglevel` | 1 | 3 | 官方也是 3，报告所有错误 |
| `TRANS` | 0 | 0 | 出生转数（原 1，已校准为官方 0） |
| `LV` | 1 | 1 | 出生等级（相同） |
| `RIDEMODE` | 3 | 0 | Win 自由骑任意骑宠；Linux 官方正常 8.0 |
| `CHARTRANS` | 7 | 5 | Win 开放 7 转；Linux 官方 5 转 |
| `PETTRANS` | 2 | 0 | 宠物转数（原 -1，已校准为官方 0） |
| `MAXLEVEL` | 170 | 140 | Win 最高 170；Linux 官方 140 |
| `LEVEL` | 140 | 140 | 一般等级上限（相同） |
| `battleexp` | 20 | **1** | 战斗经验倍数（原 100，已校准为官方 1） |
| `NPRIDE` | 1 | **已删** | **Linux 源码无此键，无效配置已删除**（送宠真键 PET1-4） |
| `GOLD` | 1000000 | **100000** | 出生金钱（原 3 万，已校准为官方 10 万） |
| `usememoryunitnum` | 5000000 | 15000000 | Win 640MB / Linux 1.9GB（容量差异保留） |
| `fdnum` | 180 | 700 | 最大连接数（Linux 按公服容量，保留） |
| `petnum` | 5000 | 20000 | 最大在线宠数（保留） |
| `othercharnum` | 10300 | 45000 | 其他最大数目（保留） |
| `walkinterval` | 100 | 5000 | 走路间隔 ms（非多线程模式无作用，保留） |
| `CAinterval`/`CDinterval` | 2500 | 5000 | 同上（保留） |
| `CharSaveinterval` | 86400 | **300** | 角色存档间隔秒（原 180，已校准为官方 300） |
| `Petdeletetime` | 1 | **60** | 宠物消失时间秒（原 30，已校准为官方 60） |
| `Itemdeletetime` | 1 | **60** | 物品消失时间秒（原 20，已校准为官方 60） |
| `Golddeletetime` | 1 | **30** | 石币消失时间秒（原 1800，已校准为官方 30） |
| `loghour` | 12 | 0 | 日志时间（保留） |
| `acwbsize` | 2000000 | 1048576 | AC 缓冲区（与官方 19900623 一致） |
| `FUSIONBEIT` | 2 | **1** | 融合宠转生（原 0，已校准为官方 1：可 1 转） |
| `RIDEPETLEVEL` | 200 | 140 | 新手可骑宠物等级（官方 140，保留） |
| `ANGELPLAYERTIME` | 1 | 5000 | 精灵召唤时间间隔（**官方 19900623 也是 5000**，Win 魔改） |
| `ANGELPLAYERMUN` | 200 | 1000 | 精灵召唤最小在线人数（官方 1000） |
| `BATTLEGOLD` | 10 | **0** | 战斗获得金钱倍数（原 1，已校准为官方 0） |
| `POINT` | 1 | 0 | 禁止点数上限复点（见备注） |
| `TRANS0-7` | 15 | **5/10/15/20/30/40/5/5** | 可超越祝福点数（原全 5，已校准为官方序列） |
| `TRANSPOINTUP` | — | **-1,-1,-1,-1,-1,610,-1** | 每转点数上限（原全 -1，6 转已校准为官方 610） |
| `ENEMYACTION` | 2 | **3** | 遇敌率（原 10，已校准为官方 3） |
| `AUTOPK` | — | **-1** | 乱舞 PK（原 16 点，已校准为官方关闭） |
| `LUCKSTARTIME` | 0 | **-1** | 幸运星（原 1000，已校准为官方关闭） |
| `PLAYERNUM` | 1000 | **0** | 虚假在线人数（原 10，已校准为官方 0） |
| `SHOWVIP` | — | **1** | VIP 显示（原 0，已校准为官方 1） |
| `POOLITEMBUG` | — | **3** | 仓库强存（原 1，已校准为官方 3 禁止强存） |
| `STWSENDTYPE` | — | **2** | 外挂瞬移损耗（原 3 积分，已校准为官方 2 声望） |
| `PETUP` | — | **1** | 丢宠可拾取（原 0，已校准为官方 1） |
| `DELNEEDITEM` | — | **1** | 抓宠物品自动删除（原 0，已校准为官方 1） |
| `PETTRANSABILITY1` | — | **180** | 一转成长限制（原 155，已校准为官方 180） |
| `RIDELEVEL` | — | **5** | 骑宠等级差（原 10，已校准为官方 5） |
| `FMPOINTPK` | — | **0** | 庄园互抢（原 1，已校准为官方 0） |
| `ANNOUNCETIME` | — | **30** | 循环公告分钟（原 0，已校准为官方 30） |

> 备注：`POINT` 官方 19900623 为 1（禁止点数上限复点），WSL 当前 0（允许复点）——保留（原版 8.0 默认行为）。

---

## 四、功能差异总结

### Win版优势功能（Linux版没有）
1. **外转系统**（WDTrans/WDLevels/WDpay）- 额外的转生系统
2. **登录删除物品/宠物**（logindelitem/logindelpet）- 登录时自动清理指定物品
3. **不可继承物品**（UNHeritable）- 防止特定物品通过交易/仓库转移
4. **细粒度地图禁止**（rideforbid/helpforbid/featherforbid/joinforbid/backforbid）- 按功能分类禁止
5. **自动完成/进行事件**（endevent/nowevent）- 任务自动化
6. **VIP专服**（VIPSERVER/VIPPOINT/VIPMAP）- 包时会员专属服务器
7. **自动镶嵌孔数**（InslayNum）- 控制装备镶嵌上限
8. **IP注册限制**（iprestrict）- 防止批量注册
9. **强制存档/自动存档**（CompelStore/AutoSave）- 数据安全

### Linux版优势功能（Win版没有）
1. **每日签到系统**（REPORTITEM/VIPREPORTITEM/reportlv/reportta）
2. **GM账号加强保护**（gm1-gm4）
3. **组队经验加成/共享**（TEAMADDEXP/EXPSHARE）
4. **乱舞PK系统**（AUTOPK/AUTOPKTRANS/KILLPOINT）
5. **重回币PK**（PKMAP/PKMAPCOST）
6. **外挂防护体系**（NOSTWENEMY/STWSENDTYPE/ITEMPETLOCKED）
7. **安全锁功能**（ITEMPETLOCKED）
8. **会员包时专区**（VIPBOUNDTIME/UNBOUNDTIMEMAP）
9. **玩家上线提示**（LOGINDISPLAY）
10. **星球/世界频道声望门槛**（ALLSERV*/THEWORLD*）
11. **宠物转生成长控制**（PETTRANSABILITY/PETTRANSABILITY1-3）
12. **7转点数**（TRANS7POINT）
13. **循环挑战**（MAPBATTLELOOP）
14. **摆摊税收**（STREETVENDORPOINT）
15. **登录验证标准差**（LCSDV）- 防异常登录

### 两者核心差异
- **Win版偏向商业化/爽玩**：内存占用小（640MB）、存档间隔长（24小时）、出生资源多（100万石币+7件套装备+送宠）、等级上限高（170级7转）、经验/金钱高倍率
- **Linux版偏向官方原版**：内存占用大（1.9GB）、存档间隔短（300秒）、出生资源官方（10万石币+无宠）、等级上限官方（140级5转）、经验1倍、外挂防护完善、GM管理功能丰富

---

## 五、Linux 版官方校准记录（2026-09-08）

按官方 8.0 参考（`librae8226/gmsv`、`19900623/stoneage`）校准 WSL setup.cf，共**删除 4 个无效键、修改 30 个值、补充 6 个功能键**：

### 5.1 删除的无效键（源码不解析）
| 键 | 原因 |
|---|---|
| `petskill2file=./data/petskill3.txt` | 无效键+文件不存在（8.0 真键是 petskillfile2） |
| `setpetskillfile=./data/setpetskill.txt` | 无效键+文件不存在 |
| `UNLAWWARPFLOOR=32021,...` | 8.0 源码不认此键，真键是 FLOOR1-5 |
| `NPRIDE=3` | 源码无此键，送宠真键是 PET1-4（一直空=出生没宠） |

### 5.2 补充的未设置功能
| 键 | 值 | 功能 |
|---|---|---|
| `FLOOR1-5` | 32021/8213/850/30695/60034 | 禁止玩家互相传送地图（_UNLAW_WARP_FLOOR 激活） |
| `storechar` | ./data/storechar | 存角色档案文件（_STORECHAR） |

### 5.3 修改的值（魔改前 → 官方后）
```
battleexp 100→1  TRANS 1→0  GOLD 30000→100000  ENEMYACTION 10→3
BATTLEGOLD 1→0  FUSIONBEIT 0→1  ANNOUNCETIME 0→30  PETTRANS -1→0
CharSaveinterval 180→300  Petdeletetime 30→60  Itemdeletetime 20→60  Golddeletetime 1800→30
TRANS0-7 全5→5/10/15/20/30/40/5/5  TRANSPOINTUP 6转-1→610
AUTOPK 16→-1  AUTOPKTRANS 5→1  AUTOPKLV 140→100  LUCKSTARTIME 1000→-1
PLAYERNUM 10→0  SHOWVIP 0→1  POOLITEMBUG 1→3  STWSENDTYPE 3→2
PETUP 0→1  DELNEEDITEM 0→1  PETTRANSABILITY1 155→180  RIDELEVEL 10→5
FMPOINTPK 1→0
```

### 5.4 刻意保留项（与官方不同但有意为之）
- `ITEM1=24114` 远古木：官方 8.0 出生物品（23757 等）在 9.0 数据不存在，换用会空物品
- `MAXLEVEL=140`：官方即 140（Win 170 是魔改）
- `LEVEL=140`：官方 120，但 9.0 exp.txt 按 140 一般等级设计
- `nohelpmap/NOTESCAPE/MAPHEAL` 等地图号：绑定 9.0 地图数据，不套 8.0 地图号
- `POINT=0`：官方 1（禁止复点），保留原版 8.0 默认行为

---

## 6. 更新记录

### 2026-09-08 晚间（登录链路修复，无配置变更）
- 本次修改集中在 `gmsv/src/` 协议层（动态密钥时机、checksum 公式、登录 result），**未改 setup.cf**
- 当前 WSL `setup.cf` 端口 = 9066（抓包模式），D 盘存档 = 9065（正式），二者刻意差异
- 配置键/值以上表（5.1-5.4）为最终态，后续登录问题优先查协议层而非配置
