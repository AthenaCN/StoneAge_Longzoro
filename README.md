# 石器时代 8.0 私服搭建记录（GMSV + SAAC）

> **龙zoro版 GMSV v2.2.2.28 + SAAC v1.1** 从零搭建全流程：WSL 安装 Fedora → 编译 → 配置 → 双服务器稳定运行。
> 本文档按实际操作顺序记录，含全部踩坑与修复方案，照着做即可复现。

---

## 1. 架构

石器时代服务端由两个进程组成，缺一不可：

```
┌─────────────┐   TCP 9200   ┌──────────────┐   9200   ┌─────────┐
│  MySQL (sa) │◄────────────►│  saac        │◄────────►│ gmsv    │
│  logindata  │  账号/存档     │  账号服务器    │ 登录握手   │ 游戏服务器│
└─────────────┘              └──────┬───────┘           └────┬────┘
                                    │ 9400 (worker)          │ 9066 (WSL 游戏端口)
                              ┌─────▼─────┐           ┌──────▼─────┐
                              │ acwk      │           │ 代理 9065→9066 │
                              └───────────┘           └─────────────┘
```

| 进程 | 作用 | 端口 | 关键配置 |
|---|---|---|---|
| `saac` | 账号服务器：注册/登录校验、存档、PK/竞速场数据 | 9200 | `acserv.cf` |
| `acwk` | saac worker：`_FIX_WORKS` 开启后的外部工作进程 | 9400 | 必须 `-O0` 编译 |
| `gmsv` | 游戏服务器：地图、战斗、NPC、物品、宠物 | 9066 | `setup.cf` |
| `proxy9065.py` | 抓包代理：9065 → 9066（客户端连 9065） | 9065 | Windows 侧运行 |

> **当前端口状态**：WSL gmsv 监听 9066（抓包调试模式），Windows 侧 Python 代理转发 9065→9066，客户端连接 `127.0.0.1:9065`。正式部署时改回 `port=9065` 直连、关掉代理即可（D 盘 setup.cf 存档版本就是 9065）。

**启动顺序：必须先 `saac` 后 `gmsv`**。gmsv 启动时会向 saac 发送登录请求（服务器名+密码），握手成功后进入主循环。

---

## 2. 环境搭建（从零开始）

### 2.1 Windows 侧安装 WSL + Fedora 44

以管理员身份打开 **PowerShell**，执行：

```powershell
# 安装 WSL 本体 + Fedora 44 发行版
wsl --install FedoraLinux-44
```

> 如果提示找不到发行版，先执行 `wsl --list --online` 查看可用列表，确认 Fedora 的准确名称。

安装完成后**重启电脑**，然后启动并完成首次初始化（设置 Linux 用户名/密码）：

```powershell
wsl -d FedoraLinux-44
```

### 2.2 进入 Fedora，更新系统

```bash
# 更新系统（首次会比较久）
sudo dnf update -y
```

### 2.3 安装开发工具组

```bash
# 编译工具链：gcc / make / gdb 等
sudo dnf groupinstall -y "@development-tools"
```

### 2.4 安装编译依赖

```bash
# MySQL 客户端库（链接用 -lmysqlclient）+ zlib + 调试器
sudo dnf install -y mysql-devel zlib-devel gdb
```

### 2.5 准备 MySQL 数据库

```bash
# 安装并启动 MySQL（Fedora 仓库也可用 MariaDB，兼容）
sudo dnf install -y mysql-server
sudo systemctl enable --now mysqld
```

> WSL 2 较新版本默认启用 systemd，`systemctl` 可直接使用。若提示 systemd 未运行，改用 `sudo service mysqld start`。

**数据库的密码初始化 + 建库建表一条命令搞定，见第 5 节**（第 5 节命令用 `sudo mysql` 免密执行，自动完成密码设置、建库、建表，下次重装环境直接复制即可）。

### 2.6 拷贝源码到 Linux

Windows 侧源码在 `D:\StoneAge`，WSL 里通过 `/mnt/d` 访问：

```bash
mkdir -p ~/StoneAge
cp -r /mnt/d/StoneAge/saac ~/StoneAge/
cp -r /mnt/d/StoneAge/gmsv ~/StoneAge/
```

---

## 3. 目录结构

```
~/StoneAge/
├── saac/                  # 账号服务器
│   ├── src/               # 源码（main.c recv.c lock.c acfamily.c char.c ...）
│   ├── acserv.cf          # 账号服务器配置
│   ├── data/              # 家族/庄园/留言数据（family、fmpointdir、fmsmemodir）
│   ├── db/                # 运行数据库（defend、herolist、int、string、missiontable）
│   ├── char/              # 人物存档（运行时按 0x00~0xff 自动分片）
│   ├── char_sleep/ mail/ lock/ log/   # 睡眠存档/邮件/锁/日志（运行时自动生成）
│   └── saac               # 编译产物
└── gmsv/                  # 游戏服务器
    ├── src/               # 源码（main.c + char/ npc/ map/ item/ magic/ battle/ 子库）
    ├── setup.cf           # 游戏服务器配置
    ├── data/              # 游戏数据（map/ itemset6.txt/ enemy1.txt/ enemybase1.txt/ npc/ ...）
    ├── Dengon/ Schedule/ log/   # 留言板/排程/日志（运行时自动生成，可删）
    └── gmsv               # 编译产物
```

> 运行时自动生成的目录/文件删掉后会按需重建（Dengon、Schedule、log、char 分片、race 等），不必手动创建。

---

## 4. 编译

### 4.1 saac

```bash
cd ~/StoneAge/saac

# 修改 Makefile：GCC 10+ 必须加 -fcommon（可同时改为 -g 便于调试）
#   CFLAGS=-w -O3 -fcommon $(INCFLAGS)   → 生产
#   CFLAGS=-g -O0 -fcommon $(INCFLAGS)   → 调试
make -C src
```

链接依赖：`-lmysqlclient -lz`

### 4.2 gmsv

```bash
cd ~/StoneAge/gmsv
make -C src
```

链接依赖：`-lm`

### 4.3 Makefile 三个坑（必读）

1. **GCC 10+ 必须 `-fcommon`**：旧代码大量使用"头文件里定义全局变量"，GCC 10 默认 `-fno-common` 会报 `multiple definition`，必须在 `CFLAGS` 加 `-fcommon`。
2. **Makefile 不追踪 `.h` 依赖**：改了头文件 / `version.h` 后 `make` 不会自动重编，**必须 `make -C src clean` 全量重编**，否则用的还是旧对象文件。
3. **gmsv 改子库**：`npc/`、`map/`、`item/` 等是静态库，改了子库代码要先重编子库再顶层链接：

```bash
cd src/npc && make        # 重编 npc 子库
cd ../.. && make -C src   # 顶层链接
```

---

## 5. 数据库准备（一条命令搞定）

用 `sudo mysql` 免密执行，**设置 root 密码 + 建库 + 建表一步完成**（密码 `123456` 为本地开发示例，公网部署请改为强密码并同步改 `acserv.cf` 的 `sql_PS`）：

```bash
sudo mysql -e "
ALTER USER 'root'@'localhost' IDENTIFIED BY '123456';
FLUSH PRIVILEGES;
CREATE DATABASE IF NOT EXISTS sa DEFAULT CHARSET utf8mb4;
USE sa;
CREATE TABLE IF NOT EXISTS logindata (
  Name varchar(32) NOT NULL,
  Pass varchar(32) NOT NULL,
  RegTime datetime DEFAULT NULL,
  LoginTime datetime DEFAULT NULL,
  Path varchar(255) DEFAULT NULL,
  Online varchar(16) DEFAULT NULL,
  PRIMARY KEY (Name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
"
```

验证表已建好：

```bash
mysql -u root -p123456 -e "SHOW TABLES FROM sa; DESCRIBE sa.logindata;"
```

> 表结构说明：`Name`=游戏账号（主键）、`Pass`=密码、`RegTime`=注册时间、`LoginTime`=登录时间、`Path`=存档路径、`Online`=在线状态。saac 通过这些字段完成注册/登录校验（对应 `acserv.cf` 的 `sql_Name`、`sql_PassWord` 等配置）。

---

## 6. 配置

### 6.1 saac / acserv.cf（关键项）

```
sql_IP 127.0.0.1
sql_Port 3306
sql_ID root
sql_PS 123456          # 与 2.5 设置的 MySQL 密码一致
sql_DataBase sa
sql_Table logindata
sql_Name Name
sql_PassWord Pass
port 9200              # 监听端口（gmsv 来连）
pass test              # 服务器密码（gmsv setup.cf 里要一致）
dbdir db
logdir log
chardir char
sleepchardir char_sleep
maildir mail
familydir data/family
fmpointdir data/fmpointdir
fmsmemodir data/fmsmemodir
```

### 6.2 gmsv / setup.cf（关键项）

```
账号服务器地址: 127.0.0.1
账号服务器端口: 9200
登陆服务器名称: 公益石器          # 与 saac 侧一致
登陆服务器密码: test
等待连接端口: 9065              # 客户端连接端口
游戏服务器ID: 1
服务端序列号: 1
最大在线人数: 700
日志记录文件: ./
NPC 配置目录: ./data/npc
```

> 两个服务器的**名称/密码/端口必须一一对应**，否则 gmsv 无法注册成功（表现为一握手就被 saac 踢掉）。

---

## 7. 启动与验证

### 7.1 启动

**方式 A：前台运行，实时看输出（推荐调试用）**——开两个终端窗口，各跑一个服务，日志直接打在屏幕上：

```bash
# 终端 1：前台启动 saac（账号服务器），实时输出注册/登录/握手信息
cd ~/StoneAge/saac && ./saac

# 终端 2：前台启动 gmsv（游戏服务器），实时输出地图加载/心跳循环
cd ~/StoneAge/gmsv && ./gmsv
```

**方式 B：tmux 单窗口双屏**——只开一个 WSL 终端也能同时看两个服务的实时输出：

```bash
sudo dnf install -y tmux
tmux new-session -d -s saac 'cd ~/StoneAge/saac && ./saac'
tmux new-session -d -s gmsv  'cd ~/StoneAge/gmsv && ./gmsv'
tmux attach -t saac    # 看 saac 实时输出（Ctrl+B 再按 D 退出，不杀服务）
tmux attach -t gmsv    # 看 gmsv 实时输出
```

**方式 C：一条命令启动（推荐，自动等 saac 就绪）**——saac 后台起，循环等 9200 端口就绪后再起 gmsv，比固定 `sleep 3` 稳：

```bash
# 后台静默版（部署/重启用，两个服务都后台）
touch ~/StoneAge/gmsv/lockip.txt
cd ~/StoneAge/saac && ./saac > /tmp/saac.log 2>&1 &
while ! ss -tln | grep -q ':9200'; do sleep 1; done
cd ~/StoneAge/gmsv && ./gmsv > /tmp/gmsv.log 2>&1 &
```

```bash
# 调试版（gmsv 前台实时看输出，saac 后台写日志）
touch ~/StoneAge/gmsv/lockip.txt
cd ~/StoneAge/saac && ./saac > /tmp/saac.log 2>&1 &
while ! ss -tln | grep -q ':9200'; do sleep 1; done
cd ~/StoneAge/gmsv && ./gmsv 2>&1 | tee /tmp/gmsv.log
```

> **为什么不用固定 `sleep 3`**：saac 启动要分配 1.9GB 内存 + 读 db/char/家族数据，初始化时间不固定；gmsv 过早连接会被 saac 拒绝，表现为 `连接已关闭[0]!!`、没有 `SAAC登陆成功`。用 `while` 等 9200 端口真正 LISTEN 后再起 gmsv 最稳。
>
> `touch lockip.txt` 是为了消除启动时 `读取锁定IP文件...无法打开文件` 的提示（无害，不创建也能跑）。
>
> 实时查看日志：`tail -f /tmp/saac.log` 或 `tail -f /tmp/gmsv.log`（Ctrl+C 退出查看，不影响服务）
> 验证状态：`ss -tln | grep -E '9200|9065'`、`pgrep -af 'saac|gmsv'`

### 7.2 验证

```bash
# 端口：两个都在 LISTEN 即成功
ss -tln | grep -E '9200|9065'

# 进程
pgrep -af 'saac|gmsv'
```

**成功日志特征**：

```
# saac（/tmp/saac.log）
数据库连接成功！
服务器密码正确 公益石器

# gmsv（/tmp/gmsv.log）
SAAC登陆成功
新服务器!  版本 : <龙zoro版GMSV服务端v2.2.2.28> 正常服务器!
玩家=0 宠物=0.0% 物品=0.0% 邮件:0 战斗:0   # 主循环心跳，稳定运行
```

### 7.3 重启

**WSL 后台进程铁律：必须 `nohup ... & disown`，否则 WSL 会话退出时子进程被杀**（saac 曾因此反复退出；saac 还曾 CPU 99.9% 死循环，需定期 `ps -C saac -o pid,pcpu,comm` 检查）。

```bash
# 1. 重启 saac（先杀，9200 端口释放后重启）
pkill -x saac; sleep 2
cd ~/StoneAge/saac && nohup ./saac > /tmp/saac.log 2>&1 & disown

# 2. 重启 acwk（_FIX_WORKS 需要，必须 -O0 编译；先于 saac 启动会占用 9400 导致 saac bind 失败）
pkill -x acwk; sleep 1
cd ~/StoneAge/saac/wk && nohup ./acwk > /tmp/acwk.log 2>&1 & disown

# 3. 重启 gmsv（等 saac 就绪；gmsv 启动约 15-20 秒加载地图后才监听端口）
pkill -x gmsv; sleep 2
cd ~/StoneAge/gmsv && nohup ./gmsv > /tmp/gmsv.log 2>&1 & disown
sleep 20
ss -tln | grep -E '9066|9200|9400'
```

> Windows 侧代理（抓包模式）启动：`powershell -ExecutionPolicy Bypass -File C:\Users\<你的用户名>\Downloads\start_proxy.ps1`（代理会莫名自退，连不上 9065 时先检查代理）。

---

## 8. 数据说明（win9.0 一键端数据替换）

### 8.1 为什么替换

2.27 原版 `data/` 存在数据缺失（enemy1.txt 引用不存在的宠物模板 → 20 行语法错误；地图文件不完整等）。win9.0 一键端是网友基于 8.0 魔改的版本，**格式与 2.27 解析器兼容，且内容更丰富**，整体替换后问题消失。

### 8.2 兼容性验证（关键，替换前必做）

逐文件对比段数（按分隔符切分），确认解析器能读：

| 文件 | 2.27 段数 | 9.0 段数 | 结论 |
|---|---|---|---|
| itemset6.txt | 94 | 94 | ✅ 兼容（9.0 多 4369 物品） |
| enemy1.txt | 34 | 34 | ✅ 兼容（9.0 多 288 宠物） |
| enemybase1.txt | 56 | 56 | ✅ 兼容（9.0 多 344 模板） |
| magic.txt | 9 | 9 | ✅ 兼容 |
| petskill2.txt | 12 | 12 | ✅ 兼容 |
| profession.txt | 23 | 23 | ✅ 兼容 |

替换收益：+4369 物品 / +288 宠物 / +344 模板 / +99 技能 / +28 魔法 / +36 地图 / +265 NPC 文件。

### 8.3 替换方法

```bash
cd ~/StoneAge/gmsv
mv data data.bak2.27            # 原版备份（确认无误后可删）
cp -r /mnt/d/石器时代9.0/服务端/gmsv/data data   # 拷入 9.0 数据（5595 文件）
```

替换后 `enemy1.txt 语法错误 20 → 0`，服务器稳定主循环。

---

## 9. 已知告警清单（无害，不用修）

启动日志里会出现以下告警，**全部是 9.0 原版数据自带的瑕疵**，两侧（2.27 / 9.0）跑同样报错，**不影响任何游戏功能**（2026-09-08 修复 `MAX_MAP_FILES` 后，`地图ID号有毛病` 已从 312 条清零）：

| 告警 | 数量 | 原因 | 影响 |
|---|---|---|---|
| `文件大小不正确(目标:xxx 实际:xxx)` | ~10 | 魔改地图**尾部带冗余数据**。源码 `readmap.c` 用 `ftell(f) != st_size` 判断：`目标`=文件真实字节数，`实际`=地图头声明尺寸，声明 < 实际即报。**不是文件损坏** | 无 |
| `Duplicate Itemid 30100 / 50033` | 2 | 9.0 数据里物品 ID 重复 | 无 |
| `MAP_addNewObj error` | 0（已清零） | 地图传送点指向不可通行坐标（曾因 MAX_MAP_FILES 不足大量出现） | 无 |
| `地图ID号有毛病` | 0（已清零） | create 文件引用了不存在的/未加载的地图号（修复 MAX_MAP_FILES 后 1300 之后的地图全部加载） | 无 |
| `(物品名)尚未设定成分` | 少量 | 新物品未配置物品成份表 | 无 |

> 判断标准：**只要 saac 出现 `服务器密码正确`、gmsv 出现 `SAAC登陆成功` 并进入 `玩家=0 ...` 心跳循环，服务器就是健康的。**

### 9.1 资源加载修复：`MAX_MAP_FILES 1300 → 2000`（重要）

**现象**：WSL 启动日志地图数 1289、NPC 7085，与 win9.0 原版（地图 1443、NPC 7397）不一致，少 154 地图 / 312 NPC，日志大量 `地图ID号有毛病`。

**根因**：8.0 源码 `readmap.c` 的 `#define MAX_MAP_FILES 1300` 是 8.0 时代的文件上限，9.0 数据 `data/map/` 实际有 1459 个文件，**1300 之后的地图全部未扫描** → 引用这些地图的 NPC 全部创建失败。

**修复**：启用源码预留的 2000 上限：

```c
// gmsv/src/map/readmap.c:605
#define MAX_MAP_FILES 2000   // 地图目录最大档案数（原 1300）
```

修复后地图 1443 / NPC 7397 与 win 版完全一致，`地图ID号有毛病`、`MAP_addNewObj error` 全部清零。**改子目录文件后必须重编子库再顶层链接**（见 4.3）。

---

## 10. 踩坑与修复记录（按时间线）

### 10.1 GCC 10+ 编译报 `multiple definition`

**现象**：`multiple definition of ...` 链接错误。

**修复**：Makefile 的 `CFLAGS` 加 `-fcommon`（saac 与 gmsv 都要），见 4.3。

### 10.2 saac 启动即段错误 `DeleteMemLockServer (lock.c:141)`

**现象**：gmsv 握手成功后，saac 立即 `Segmentation fault` 退出，gmsv 报 `acsv is down! aborting...`。

**定位**：

```bash
ulimit -c unlimited
gdb -batch -ex run -ex "bt 15" ./saac
# DeleteMemLockServer (sname=...) at lock.c:141
#     ln = userlock[i];        ← userlock 内容异常
```

**根因**：`acfamily.c` 读庄园文件（`db_fmpoint`）时**越界**，写坏了 `userlock` 所在内存。

**修复**：给 `acfamily.c` 的 `readFMPoint` 加越界保护（数据不足时返回，不越界写）。

### 10.3 gmsv 崩溃 `### ventNo=-1 贪婪的函数:PetFullMsg 脱离 ###`

**现象**：启动流程走完后 gmsv 段错误，`最后函数 (QuizInit)`。

**根因**：`npc/npc_quiz.c` 的 `QuizInit` 使用了 **65536 长度的 VLA（变长数组）**，栈溢出。

**修复**：VLA 改为**静态数组**（`static` 全局缓冲）。

### 10.4 `main.c:2122 return;` 编译错误 / 隐式函数声明

**现象**：GCC 16 报错：

```
main.c:2122:9: error: 'return' with no value, in function returning non-void
main.c:872:9: error: implicit declaration of function 'initMissionTable'
recv.c:2883:17: error: implicit declaration of function 'delMissionTableOnedata'
```

**修复**：补充 `initMissionTable`、`checkMissionTimelimit`、`delMissionTableOnedata` 的前置声明，修正 `return;` → `return 0;`。

### 10.5 saac/gmsv 互踢：`GMSV 退出或工作站退出!!`

**现象**：saac 收到 gmsv 握手后立刻登出并关闭连接，gmsv 报 `acsv is down`。

**根因**：`version.h` 中的功能宏（`_RACEMAN`、`_ANGEL_SUMMON` 等）**在 saac 与 gmsv 两侧不一致**，导致协议行为不匹配（gmsv 发送 saac 不认识的消息，如 `ACRaceRecordandSort`）。

**修复**：统一两侧 `version.h`，开启 `_RACEMAN`、`_ANGEL_SUMMON` 等对应宏后**重新编译**（改头文件记得 clean 全量重编），双服务器稳定共存。

> **排查思路（重要）**：这类"一握手就互踢"的问题，优先对比 saac 与 gmsv 的 `version.h` 差异。

### 10.6 乱码注释（日文编码）

**现象**：源码注释是日文，Linux 下显示乱码。

**修复**：注释统一转为 **UTF-8 + 中日对照**（日文原文 + 括号中文翻译）。

### 10.7 `make: Nothing to be done` 但产物没更新

改过头文件没生效？Makefile 不追踪 `.h`，`make clean` 全量重编即可（见 4.3）。

---

## 11. 数据治理清单（可清理项）

| 对象 | 内容 | 是否可删 |
|---|---|---|
| `gmsv/log/*.log`、`vssver.scc` | 运行日志 + VSS 残留 | ✅ 可删，保留 `log.cf` |
| `gmsv/Dengon/` | 地图留言板数据（`地图坐标` 命名的 268KB 文件） | ✅ 可删，启动/使用时自动重建 |
| `gmsv/Schedule/` | 家族约战/庄园排程空模板 | ✅ 可删（2.8KB，留不留都行） |
| `data/` 内 `*.bak` `*.old` `*.scc` `复件*` | 9.0 魔改者留下的备份/版本控制残留，服务端不读 | ✅ 可删 |
| `data.bak2.27/` | 2.27 原版数据备份 | ⚠️ 确认 9.0 稳定后删（删后无法回滚） |
| 0 字节文件 / 小 `.arg` 文件 | `missionclean.txt` 等被源码引用、NPC 参数文件 | ❌ 保留（删了可能触发读取报错） |
| `char/ mail/ log/ char_sleep/` 下 `0x00~0xff` 分片 | 运行时自动生成 | ✅ 可删（会自动重建） |
| saac `data/family/*.bak` 等 | 运行时自动备份（写数据前先备份） | ✅ 可删（运行时会再生成） |

---

## 12. 常见问题（FAQ）

**Q1：gmsv 报 `Cannot connect. (Connection refused errno:111)`**
saac 没起来或端口不对。先启动 saac，确认 `ss -tln | grep 9200` 在 LISTEN。

**Q2：gmsv 报 `acsv is down! aborting...`**
saac 崩了或被互踢。查 saac 日志尾部，重点看有没有 `Segmentation fault` / `服务器密码正确`；版本宏不一致见 10.5。

**Q3：服务端崩溃，怎么抓栈？**

```bash
ulimit -c unlimited
rm -f core*
./saac &
# 崩溃后：
gdb ./saac core -batch -ex "bt 15"
```

**Q4：客户端连不上 9065？**
确认 gmsv 心跳在跑（`玩家=0 ...` 持续输出）；确认客户端版本与 `setup.cf` 的版本宏匹配；确认防火墙放行 9065。

**Q5：改完 `version.h` 重编没效果？**
`make -C src clean` 全量重编（见 4.3）。

**Q6：`wsl --install FedoraLinux-44` 找不到发行版？**
先 `wsl --list --online` 查看准确名称，再 `wsl --install -d <名称>`。

**Q7：WSL 里 `systemctl` 不可用？**
旧版 WSL 未启用 systemd。在 Windows 侧 `C:\Users\<用户>\.wslconfig` 写入 `[wsl2]\nsystemd=true` 后 `wsl --shutdown` 重启；或改用 `sudo service mysqld start`。

---

## 13. 免责声明

- 本项目仅用于**学习与研究**，源码与数据来自互联网公开分享（龙zoro版 / win9.0 一键端），版权归原作者所有。
- 请勿用于商业运营；部署到公网前务必修改数据库密码、服务器密码、GM 密码等默认配置。
- 数据库密码 `123456`、服务器密码 `test` 等均为本地开发默认值。

---

## 14. Tokyo-sa 客户端适配（9.0 客户端连接）

本项目核心是把龙zoro版 8.0 服务端适配到 win9.0 一键端的 Tokyo-sa 加密客户端（`sa_9061.exe` + Tokyohot.dll）。完整逆向结论与修改清单见同目录 **`修改说明_Tokyo-sa适配.md`**，这里只列关键结论：

| 项 | 结论 |
|---|---|
| 握手字符 | `'L'`（不是 'F'/'$'/'A'） |
| 外层消息加密 | PersonalKey `"20041215"`，`util_shrstring` 是**循环移位** |
| 字段编解码密钥 | 登录请求用固定 `"www.longzoro.com"`；**客户端发出登录请求后立即切换**为动态 `账号名+"www.longzoro.com"`，此后全程（含登录响应/角色列表/创建角色/游戏内） |
| base64 表 | `"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz{}"` |
| 字节重排表 | 解码 `"2413"` / 编码 `"3142"` |
| checksum | **所有响应统一 `strlen(result)+strlen(data)`**（登录=2、角色列表=10；~~早期误判的 `0x00FE0680+func` 公式已废弃~~） |
| 登录成功 result | `"ok"`（动态密钥编码：账号1→`{sF`、Athena→`kvU`；~~"uk"是账号1巧合~~） |
| 角色登录成功 result | `"successful"`（动态密钥编码） |

**登录全链路（登录→角色列表→创建角色→角色登录→进游戏→战斗→登出）已全部打通并经用户实测，且与 win 版 9.0 服务端响应逐字节一致**（见 `修改说明_Tokyo-sa适配.md` 第八·五节）。客户端切换账号自动重设动态密钥，无需重启客户端。

---

## 15. setup.cf 官方校准（2026-09-08）

WSL 的 setup.cf 原为龙zoro 作者测试用默认值，按官方 8.0（开源参考：`librae8226/gmsv`、`19900623/stoneage`）校准：

**删除的无效键**（源码根本不解析，写了白写）：

| 键 | 原因 |
|---|---|
| `petskill2file` | 指向不存在的 petskill3.txt；8.0 真键是 `petskillfile2` |
| `setpetskillfile` | 同上，文件也不存在 |
| `UNLAWWARPFLOOR` | 8.0 源码不认此键，真键是 `FLOOR1-5` |
| `NPRIDE=3` | 源码无此键，送宠真键是 `PET1-4`（一直空=出生没宠） |

**补上的未设置功能**（宏已开但键没写）：

| 键 | 值 | 功能 |
|---|---|---|
| `FLOOR1-5` | 32021/8213/850/30695/60034 | 禁止玩家互相传送地图（启动日志已确认生效） |
| `storechar` | ./data/storechar | 存角色档案文件（_STORECHAR） |

**改回官方值的 30 处**（已全部加注释，关键项）：

| 项 | 魔改前 | 官方后 | 项 | 魔改前 | 官方后 |
|---|---|---|---|---|---|
| 战斗经验倍数 | 100 | **1** | 出生金钱 | 3万 | **10万** |
| 出生转数 | 1 | **0** | 遇敌率(ENEMYACTION) | 10 | **3** |
| 战斗金钱 | 1 | **0** | 融合宠转生 | 0 | **1** |
| 循环公告 | 0 | **30分** | 乱舞PK | 16点 | **关闭** |
| 幸运星 | 开 | **关闭** | 宠物/物品消失 | 30/20s | **60/60s** |
| 石币消失 | 30分 | **30s** | 存档间隔 | 180s | **300s** |
| 每转点数 | 全5 | **5/10/15/20/30/40/5/5** | 6转上限 | -1 | **610** |
| 强存 | 允许 | **禁止** | 瞬移损耗 | 积分 | **声望** |
| 丢宠拾取 | 不可 | **可** | VIP显示 | 0 | **1** |
| 宠物转数 | -1 | **0** | 一转成长 | 155 | **180** |
| 骑宠等级差 | 10 | **5** | 虚假在线 | 10 | **0** |

**刻意保留的**（注释已说明）：
- `ITEM1=24114` 远古木：官方 8.0 出生物品 ID（23757 等）在 9.0 数据里不存在，换了会空物品
- `MAXLEVEL=140`：官方就是 140（win 的 170 是魔改）
- `LEVEL=140`：官方 120，但 exp.txt 是 9.0 的（按 140 一般等级），改了会错位
- `nohelpmap/NOTESCAPE/MAPHEAL` 等地图号：绑定 9.0 地图数据，不套官方 8.0 地图号

> win9.0 是**商业化魔改**（出生 100 万石币、7 件套装备、170 级、7 转、高经验倍率），WSL 按官方原版体验。完整对照见 `setup.cf对比分析_Win_vs_Linux.md`。

---

## 16. 乱码修复（NPC 对话 / 聊天 / 地名）

**现象**：NPC 对话框"加加的瘁郊樱"、聊天乱码。

**根因**（双重）：
1. `autil.c` iconv 混转函数**逐字节误判**：GBK 流中连续字节恰为合法 UTF-8 即误转（实锤：`B4 E5 B3 A4`="村长" 中 `E5 B3 A4` 恰是 UTF-8 的"哉"）。
2. `char.c` `CHAR_appendNameAndTitle` 冒号用 UTF-8 冒号字节（`EF BC 9A`），客户端按 GBK 解析变乱码。

**修复**：
1. `autil.c` 新增 `str_is_valid_utf8()` **整串校验**，只有整串合法 UTF-8 才转 GBK。
2. 冒号改 GBK 转义 `\xa3\xba`。

修复后：`加加的村长: 欢迎来到加鲁卡东部森林的村庄加加！` 正常显示。

---

## 17. 双端源码同步（Windows D 盘 ↔ WSL）

**约定**：Windows `D:\StoneAge\` 是权威源码（UTF-8），WSL `~/StoneAge/` 是运行副本，**任何修改都要双向同步**。

一键同步脚本：`~/StoneAge/sync_src.sh`（rsync 双端 + 全量重编译）。手动同步：

```bash
# WSL 改完 → 同步到 D 盘
rsync -av --delete ~/StoneAge/gmsv/src/ /mnt/d/StoneAge/gmsv/src/
rsync -av --delete ~/StoneAge/saac/src/ /mnt/d/StoneAge/saac/src/
rsync -av ~/StoneAge/gmsv/setup.cf /mnt/d/StoneAge/gmsv/setup.cf  # 注意 D 盘端口 9065 是刻意差异

# D 盘改完 → 同步到 WSL（先备份 WSL 运行版）
cp ~/StoneAge/gmsv/setup.cf /tmp/setup.cf.bak
rsync -av --delete /mnt/d/StoneAge/gmsv/src/ ~/StoneAge/gmsv/src/
rsync -av --delete /mnt/d/StoneAge/saac/src/ ~/StoneAge/saac/src/
```

**编码铁律**：源码/setup.cf 保持 **UTF-8**（WSL 下 GBK 编译会导致 17 万 U+FFFD 乱码）。历史坑：`D:\StoneAge_gmsv_src_before_gbk.tar.gz` 名字误导，实为 GBK 实验态快照，**不可当 UTF-8 基准**。

---

## 18. 道具栏物品显示错位/空白（Tokyo-sa 客户端解析格式对齐）

**现象**：登录后道具栏部分物品不显示、图档错乱（衣服显示成卷轴、缎显示成饰品）、部分格空白；服务端抓包解码看物品数据"完全正常"。

**关键线索**：FullEdition 客户端源码（`石器时代8.5客户端最新源代码\system\netproc.cpp:3608 lssproto_I_recv`）：
客户端按**固定字段数步进**解析物品串（`no = j*13` 或 `j*14`，宏决定），每格字段数差 1 就整体错位。

**根因**：Tokyo-sa（9.0 win 版）客户端期望 **14 token/格**（格号 + 13 字段）。win 版 gmsv.exe 证实：
- `0x62F98C`：`%s|%s|%d|%s|%d|%d|%d|%d|%d|%s|%d|%s|%d|`（13 字段：name/副名/颜色/效果/图档/可用/目标/等级/标记/耐久/数量/杂/类型）
- `0x62F9C2`：`%d|||||||||||||`（空物品 14 token）
- `0x62F9B4`：`|||||||||||||`（无格号空物品 14 token）

而我们 CHAR_sendItemData（char_item.c:192）把格号传给 `ITEM_makeItemStatusString(格号,...)` 走了**单格 12 字段**分支 → 客户端按 14 步进解析 → 从第 2 格起错位。

**修复**（2 处，对齐 win 版/FullEdition）：
1. `gmsv/src/char/char_item.c` CHAR_sendItemData：
   `snprintf(token, sizeof(token), "%d|%s|", itemgroup[i], ITEM_makeItemStatusString(-1, itemindex));`
   （格号 + 全量 13 字段；与 FullEdition `char_item.c:193` 写法一致）
2. `gmsv/src/item/item.c` ITEM_makeItemFalseStringWithNum：`"%d|||||||||||||"`（格号+13 空字段 = 14 token）

**验证**：修复后解码服务端发出的物品串：
`17|普通的肉||0|耐久力40前後回复|24035|0|1|0|7|不会损坏|1|杂|20|`
= 14 字段 + 结尾，图档/名字/耐久全部正确。
