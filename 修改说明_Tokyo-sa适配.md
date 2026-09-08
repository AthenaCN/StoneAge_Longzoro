# 龙zoro版 GMSV v2.2.2.8 适配 Tokyo-sa 9.0 客户端 — 修改全记录

> 基准：龙zoro版 GMSV v2.2.2.8 + SAAC v1.1（WSL Fedora GCC 14 编译）
> 客户端：win9.0 一键端配套 sa_9061.exe + Tokyo-sa 加密补丁（Tokyohot.dll）
> 目标：完整逆向 Tokyo-sa 协议字段编解码，不再硬编码，实现登录→创建角色→进入游戏→战斗全流程

---

## 一、编译层面修复（GCC 14 兼容性，不动逻辑）

### 1.1 Makefile 添加 `-fcommon`
**文件**：`gmsv/src/Makefile`、各子目录 Makefile（char/npc/map/item/magic/battle）
**原因**：GCC 10+ 默认 `-fno-common`，多个文件定义的全局变量会链接报错。
```makefile
CFLAGS=-w -O3 -fcommon $(INCFLAGS)
```

### 1.2 npc_quiz.c VLA → 静态缓冲
**文件**：`gmsv/src/npc/npc_quiz.c`
**原因**：GCC 14 下变长数组（VLA）在特定路径导致栈溢出崩溃。
```c
// 原：char buf[some_var];
// 改：static char buf[FIXED_SIZE];
```

### 1.3 acfamily.c 越界保护
**文件**：`saac/src/acfamily.c`
**原因**：数组索引未校验，GCC 14 优化后越界写导致崩溃。
```c
if (index >= MAX_FMPOINT) return;  // 添加边界检查
```

### 1.4 version.h 开宏
**文件**：`gmsv/src/include/version.h`
**原因**：部分功能宏未开启导致编译错误或运行时缺失。
- `_REDHAT_V9`：RedHat 9.0 编译兼容
- `_FIX_WORKS`：启用外部 worker 机制（saac 需要 acwk 连接 9400 端口）
- `_FIX_MESSAGE`：worker 封包内容修改

### 1.5 函数前置声明
**文件**：多个 .c 文件
**原因**：GCC 14 对隐式函数声明报错（原代码依赖旧编译器行为）。
```c
// 在文件顶部添加被调用函数的声明
void some_function(int fd);
```

### 1.6 saac worker (acwk) 必须 -O0 编译
**文件**：`saac/wk/Makefile` 或手动编译
**原因**：-O3 优化掉 Log 输出变量，导致 worker 无法正常工作。
```bash
cd ~/StoneAge/saac/wk
gcc -o acwk -O0 -g -w -fcommon -D_FIX_WORKS -D_FIX_MESSAGE \
    -include stdio.h wk.c error.c TCPIP.c tools.c
```

---

## 二、数据替换

### 2.1 gmsv/data 整体替换为 9.0 数据
**目录**：`gmsv/data/`
**原因**：8.0 数据与 9.0 客户端不兼容（地图、NPC、物品、宠物编号不同）。
**操作**：将 `D:\石器时代9.0\服务端\gmsv\data\` 整体复制到 `gmsv/data/`，并做数据治理（清理无效引用）。

**已知副作用**：9.0 地图数据中部分传送点引用了不存在的地图编号（如 3300），启动时产生 357 条 `MAP_addNewObj error`，不影响核心游戏。

---

## 三、协议/加密逆向修改（核心）

### 3.1 握手字符：'F' → 'L'
**文件**：`gmsv/src/net.c`（约第 2321 行）
**原因**：Tokyo-sa 客户端期望握手字符为 `'L'`（Longzoro/龙zoro），收到 `'F'` 会立即断开报版本不同。
```c
// 原：char mess[1024] = "A";  // 后续逻辑可能设为 'F'
// 改：
mess[0] = 'L';
print("[HANDSHAKE] sending '%c'\n", mess[0]);
send(sockfd, mess, strlen(mess) + 1, 0);
```

### 3.2 握手发送：去掉 `if (!from_acsv)` 条件
**文件**：`gmsv/src/net.c`（约第 2302 行）
**原因**：WSL 下客户端(127.0.0.1)与 saac(127.0.0.1)来源 IP 相同，导致 `from_acsv` 误判为 1，跳过握手发送。客户端收不到握手就不发登录请求，30 秒后被超时踢掉。
```c
// 原：if (!from_acsv) write(sockfd, mess, strlen(mess) + 1);
// 改：无条件发送（saac 不会主动 connect 到 gmsv 监听端口，gmsv 主动连 saac）
send(sockfd, mess, strlen(mess) + 1, 0);
```

### 3.3 外层加密密钥：PersonalKey
**文件**：`gmsv/src/include/version.h`
**原因**：Tokyo-sa 外层消息加密（util_EncodeMessage）使用 `_RUNNING_KEY`。
```c
#define _DEFAULT_PKEY "ttttttttt"   // 默认密钥（登录前）
#define _RUNNING_KEY  "20041215"    // 运行密钥（外层消息加密）
```
**注意**：PersonalKey 仅用于外层消息加密，**不用于**字段编解码。

### 3.4 字段编解码密钥：TokyoKey（全局变量）
**文件**：`gmsv/src/autil.c`（第 15 行）、`gmsv/src/include/autil.h`（第 14 行）
**原因**：Tokyo-sa 字段编解码（util_deint/util_mkint/util_destring/util_mkstring）使用独立密钥，与 PersonalKey 无关。逆向确认默认密钥为 `"www.longzoro.com"`。
```c
// autil.c
char TokyoKey[256] = "www.longzoro.com";   // 从 static 改为全局非 static

// autil.h
extern char TokyoKey[256];
```

### 3.5 字段编解码函数使用 TokyoKey
**文件**：`gmsv/src/autil.c`
**修改**：util_deint / util_mkint / util_destring / util_mkstring 中所有 `PersonalKey` 替换为 `TokyoKey`。
```c
// util_deint
util_shl_64to256(t3, MesgSlice[sliceno], DEFAULTTABLE, TokyoKey);  // 原 PersonalKey

// util_mkint
util_256to64_shr(t3, (char *)&t2, sizeof(int), DEFAULTTABLE, TokyoKey);

// util_destring
util_shr_64to256(value, MesgSlice[sliceno], DEFAULTTABLE, TokyoKey);

// util_mkstring
util_256to64_shl(t1, value, strlen(value), DEFAULTTABLE, TokyoKey);
```

### 3.6 util_mkint 固定 6 字符输出
**文件**：`gmsv/src/autil.c`（util_mkint 函数）
**原因**：Tokyo-sa 整数编码固定 6 字符，不足时用密钥相关字符填充。
```c
len = strlen(t3);
for (j = len; j < 6; j++) {
    t3[j] = DEFAULTTABLE[(TokyoKey[j % strlen(TokyoKey)]) % 64];
}
t3[j] = '\0';
```

### 3.7 动态密钥：登录请求发出后立即切换为 cdkey + "www.longzoro.com"
**文件**：`gmsv/src/lssproto_serv.c`（LSSPROTO_CLIENTLOGIN_RECV 分支，约第 934 行）
**原因（2026-09-08 win 版抓包定论）**：Tokyo-sa 客户端**在发出登录请求后立即**把字段编解码密钥从固定 `"www.longzoro.com"` 切换为 `账号名 + "www.longzoro.com"`，**用动态密钥解登录响应并期望 result="ok"**。服务端必须在登录响应**发送前**设置动态密钥，否则客户端解响应乱码直接断开（表现为"服务器连接时间已到"）。
```c
/* Tokyo-sa: 客户端发登录请求后立即切动态密钥，必须先于响应发送设置 */
snprintf(TokyoKey, sizeof(TokyoKey), "%swww.longzoro.com", cdkey);
print("[DYNKEY-LOGIN] cdkey=%s TokyoKey=%s\n", cdkey, TokyoKey);
lssproto_ClientLogin_recv(fd, cdkey, passwd);   // 内部发送登录响应（用动态密钥编码）
util_DiscardMessage();
```
**示例**：账号 "1" → 动态密钥 `"1www.longzoro.com"`；账号 "Athena" → `"Athenawww.longzoro.com"`。
**实证**：win 版 9.0 服务端对 Athena 登录响应为 `&;95;kvU;0p7aja;#;`，其中 `kvU` 正是 `Athenawww.longzoro.com` 编码的 `"ok"`（与我们的响应逐字节一致）。

### 3.8 角色登录响应也使用动态密钥
**文件**：`gmsv/src/lssproto_serv.c`（lssproto_CharLogin_send 函数，约第 2637 行）
**原因**：角色登录响应在登录之后，此时密钥已是动态密钥。
```c
CONNECT_getCdkey(fd, cdkey, sizeof(cdkey));
snprintf(TokyoKey, sizeof(TokyoKey), "%swww.longzoro.com", cdkey);
print("[DYNKEY] cdkey=%s TokyoKey=%s\n", cdkey, TokyoKey);
```

### 3.9 响应 checksum 统一为 strlen 公式（2026-09-08 定论）
**文件**：`gmsv/src/lssproto_serv.c`（lssproto_ClientLogin_send / lssproto_CharList_send / lssproto_CreateNewChar_send / lssproto_CharLogin_send）
**原因**：win 版抓包实证 **Tokyo-sa 客户端所有响应（登录/角色列表/创建角色/角色登录）的 checksum 统一为 `strlen(result) + strlen(data)`**，不是早期误判的 `0x00FE0680 + func - (func & 1)`。
```c
/* lssproto_ClientLogin_send：登录响应 result="ok" data="" → checksum=2 */
checksum = util_mkstring(buffer, result);
util_mkint(buffer, checksum);

/* lssproto_CharList_send / CharLogin_send：result="successful" → checksum=10 */
checksum = util_mkstring(buffer, result);
checksum += util_mkstring(buffer, data);
util_mkint(buffer, checksum);
```
**误判溯源**：`0x00FE0680+func` 公式源于账号 "1" 的双重巧合——固定密钥编码的 `msMsjl` 用 `1www.longzoro.com`（账号1动态密钥）解恰好也等于 2（=strlen("ok")），且用固定密钥解恰好等于 0x00FE06DE，导致早期误判为 func 公式。Athena 账号（动态密钥无巧合）暴露了真实公式。
**实证**：win 版登录响应 checksum `0p7aja` 用 `Athenawww.longzoro.com` 解 = 2；角色列表 `0p7Yja` 解 = 10。

### 3.10 角色登录响应用 util_SendMesg（带 `&;101;` 前缀）
**文件**：`gmsv/src/lssproto_serv.c`（lssproto_CharLogin_send）
**原因**：9.0 原版反汇编确认 `__util_SendMesg` 格式为 `&;%d%s;#;`，func=78 时消息中写 `&;101;`（78+23）。
```c
util_SendMesg(fd, LSSPROTO_CHARLOGIN_SEND, buffer);  // 内部加 &;101; 前缀
```

### 3.11 登录成功 result = "ok"（不是 "uk"）
**文件**：`gmsv/src/callfromcli.c`（约第 110 行）
**原因（2026-09-08 win 版抓包定论）**：Tokyo-sa 客户端用动态密钥解登录响应、期望 result 为 **`"ok"`**。早期写成 `"uk"` 是因为账号 "1" 的巧合：固定密钥编码 `"uk"` 与 `1www.longzoro.com` 动态密钥编码 `"ok"` **恰好都是 `"{sF}"`**，导致 1/1 能登录、其他账号（如 Athena）全部失败。
```c
lssproto_ClientLogin_send(fd, "ok");  /* Tokyo-sa: 登录成功 result（动态密钥编码） */
```
**实证**：Athena 登录响应 `kvU` = `Athenawww.longzoro.com` 编码的 `"ok"`；账号 1 的 `{sF` = `1www.longzoro.com` 编码的 `"ok"`。

### 3.12 创建角色字段解码（用动态密钥后自动正确）
**文件**：`gmsv/src/lssproto_serv.c`（创建角色处理，约第 939-986 行）
**修改**：从硬编码默认值改为使用解码后的实际值。
```c
// 字段映射（slice 索引）：
// [2]=dataplacenum  [3]=charname   [4]=imgno    [5]=faceimgno
// [6]=vital         [7]=str        [8]=tgh      [9]=dex
// [10]=earth        [11]=water     [12]=fire    [13]=wind
// [14]=hometown     [15]=checksum

checksum += util_deint(2, &dataplacenum);
checksum += util_destring(3, charname);
checksum += util_deint(4, &imgno);
// ... 其余字段同理

// 原（硬编码）：
// lssproto_CreateNewChar_recv(fd, 0, "Athena", 100000, 30000, 5,5,5,5, 10,0,0,0, 1);
// 改（实际解码值）：
lssproto_CreateNewChar_recv(fd, dataplacenum, charname, imgno, faceimgno,
                            vital, str, tgh, dex, earth, water, fire, wind, hometown);
```

### 3.13 全局 checksum 校验（已全部恢复为真校验）
**文件**：`gmsv/src/lssproto_serv.c`
**过程**：早期动态密钥未实现时，Tokyo-sa 客户端消息的 checksum 与 8.0 原版期望不匹配，曾把约 63 处校验改为 `if (0)` 绕过；动态密钥 + strlen 公式定论后，**已全部恢复为真校验**（当前源码 `if (0)` 计数 = 0，含角色列表请求校验——客户端用动态密钥编码 checksum=0，服务器同密钥解出 0 即匹配）。

### 3.14 角色列表硬编码（临时）
**文件**：`gmsv/src/callfromac.c`（saacproto_ACCharList_recv，约第 106 行）
**原因**：龙zoro版 saac 返回文本格式角色列表，Tokyo-sa 客户端期望原版二进制编码格式。当前发送预编码的空角色列表。
```c
/* Tokyo-sa: charlist format differs from longzoro saac.
   Send pre-encoded empty charlist (same as 9.0 original).
   TODO: convert saac data to Tokyo-sa binary format */
strcpy(buffer, ";2U0Xrf7kCVdhzJ;;msMqjl");
util_SendMesg(clifd, LSSPROTO_CHARLIST_SEND, buffer);
```

### 3.15 创建角色响应硬编码（临时）
**文件**：`gmsv/src/callfromac.c`（WHILECREATE 分支，约第 214 行）
**原因**：创建角色成功后响应格式与角色列表相同，暂用相同的空列表数据。
```c
char cbuf[256];
strcpy(cbuf, ";2U0Xrf7kCVdhzJ;;msMqjl");
util_SendMesg(clifd, LSSPROTO_CREATENEWCHAR_SEND, cbuf);
```

### 3.16 角色名临时强制（已可移除）
**文件**：`gmsv/src/callfromcli.c`（lssproto_CharLogin_recv，约第 231 行）
**原因**：早期创建角色字段解码失败时，角色名保存为乱码，角色登录时找不到。现在动态密钥正确后角色名解码正常，此硬编码可移除。
```c
// TEMP: force charname to Athena (created with hardcoded name)
strcpy(charname, "Athena");  // 建议移除，使用客户端实际发送的角色名
```

### 3.17 角色登录后发送 `&;6;#;` 消息
**文件**：`gmsv/src/char/char.c`（约第 1858 行）
**原因**：9.0 原版在角色登录响应后发送 func=6 的消息（LSSPROTO_DU_RECV）。
```c
lssproto_CharLogin_send(clifd, SUCCESSFUL, "");
/* Tokyo-sa: 9.0原版在角色登录响应后发送 &;6;#; 消息 */
util_SendMesg(clifd, -17, "");  // -17 + 23 = 6
```

### 3.18 char.c 添加 autil.h 引用
**文件**：`gmsv/src/char/char.c`（第 31 行）
**原因**：使用 util_SendMesg 需要声明。
```c
#include "autil.h"
```

---

## 四、配置修改

### 4.1 gmsv/setup.cf
```ini
acserv=127.0.0.1
acservport=9200
acpasswd=test
port=9066                  # 抓包模式（代理 9065→9066）
gameservname=公益石器
gameservid=1
storedir=../saac/char
chatmagiccdkeycheck=0      # 关闭 GM 权限检查
encodekey=1                # 客户端验证码（大陆无效但需保留）
```

### 4.2 saac/acserv.cf
```ini
sql_IP 127.0.0.1
sql_Port 3306
sql_ID root
sql_PS 123456
sql_DataBase sa
sql_Table logindata
AutoReg 1                  # 自动注册
port 9200
pass test
```

### 4.3 服务启动顺序（重要）
```bash
# 1. 先启 saac（监听 9200 + 9400）
cd ~/StoneAge/saac && nohup ./saac > /tmp/saac.log 2>&1 & disown

# 2. 再启 acwk（连接 9400）
cd ~/StoneAge/saac/wk && nohup ./acwk > /tmp/acwk.log 2>&1 & disown

# 3. 最后启 gmsv（连接 9200，监听 9066）
cd ~/StoneAge/gmsv && nohup ./gmsv > /tmp/gmsv.log 2>&1 & disown
```
**注意**：acwk 先于 saac 启动会占用 9400 端口，导致 saac bind 失败（错误码 -3）。

---

## 五、逆向工程核心结论

### 5.1 Tokyo-sa 字段编解码算法
| 函数 | 算法 |
|------|------|
| util_deint | util_shl_64to256（减key base64解码）→ 按位取反 → "2413"字节重排 → 整数 |
| util_mkint | "3142"字节重排 → 按位取反 → util_256to64_shr（加key base64编码），固定6字符 |
| util_destring | util_shr_64to256（加key base64解码）→ 字符串 |
| util_mkstring | util_256to64_shl（减key base64编码）→ 字符串 |

### 5.2 密钥体系
| 用途 | 密钥 | 时机 |
|------|------|------|
| 外层消息加密 | `"20041215"` (_RUNNING_KEY) | 始终 |
| 字段编解码（登录请求） | `"www.longzoro.com"`（固定） | 连接建立 → 发出登录请求 |
| 字段编解码（登录响应起） | `cdkey + "www.longzoro.com"`（动态） | **客户端发出登录请求后立即切换**，此后全程（登录响应/角色列表/创建角色/角色登录/游戏内） |

**客户端密钥状态机（2026-09-08 win 版抓包定论）**：
1. 登录请求用**固定密钥**编码（含 checksum）
2. **发出登录请求后立即切动态密钥**（不等响应）
3. 登录响应**用动态密钥解**（期望 result="ok"，checksum=strlen 公式）
4. 角色列表请求/响应、后续所有消息都用**动态密钥**

> 注：客户端**切换账号时会自动重设动态密钥**（win 版实证无需重启客户端）；早期"换账号必须重启客户端"的假设已被推翻。

### 5.3 Base64 表
```
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz{}"
```
（'{'=62, '}'=63，与标准 base64 不同）

### 5.4 字节重排表
- 解码（util_deint）：`"2413"`
- 编码（util_mkint）：`"3142"`

### 5.5 checksum 公式（2026-09-08 修正）
- **所有响应（登录/角色列表/创建角色/角色登录）**：`checksum = strlen(result) + strlen(data)`（strlen 公式）
- ~~`0x00FE0680 + func - (func & 1)`~~：**早期误判**（账号 "1" 双重巧合导致），已废弃
- 客户端请求侧：登录请求 checksum = `len(cdkey)+len(passwd)`（strlen 公式，与服务器一致）；角色列表请求 checksum = 0（动态密钥编码）

### 5.6 func 号 +23 机制
util_SendMesg 接收的 func 参数在消息中写成 +23 后的值。例如 func=78 → 消息中 `&;101;`。

---

## 六、硬编码治理（2026-09-08 全部清理完毕）

早期联调阶段的临时硬编码**已全部移除**，当前源码无任何硬编码残留：

| 原硬编码位置 | 原内容 | 现状 |
|------|------|------|
| callfromac.c 角色列表 | 预编码空列表 `";2U0Xrf7kCVdhzJ;;msMqjl"` | ✅ 已改为**直接转发 saac 文本角色列表**（客户端接受文本格式），另实现 `convert_saac_charlist()` 转 9.0 二进制格式备用 |
| callfromac.c 创建角色响应 | 同上空列表 | ✅ 已移除，走正常响应路径 |
| callfromcli.c 角色名 | `strcpy(charname, "Athena")` | ✅ 已移除，动态密钥正确后角色名解码正常，走正常敏感词校验 |
| lssproto_serv.c 全局 checksum | `if (0)` 绕过 63 处 | ✅ **已全部恢复**（当前源码 `if (0)` 计数 = 0），动态密钥正确后 checksum 校验通过 |

---

## 七、已验证死路（避免重蹈覆辙）

1. 握手字符用 'F'/'$'/'A' → 客户端立即断开
2. result="ok"/"success" 用于**创建角色**响应 → 卡"人物制作中"（"ok" 仅登录响应）
3. 用 PersonalKey "20041215" 做字段编解码 → checksum 永远不匹配
4. 固定密钥 "www.longzoro.com" 用于登录后所有消息 → 客户端解不开卡签入中
5. 动态密钥 "1zor.longolonrom" → IDA 反编译整数有误，正确为 cdkey+"www.longzoro.com"
6. 去掉角色登录响应的 `&;101;` 前缀 → 反汇编确认 9.0 原版也有此前缀
7. 角色登录 checksum 套用 0x00FE0680 公式 → 应为 strlen(result)+strlen(data)
8. TokyoKey 声明为 static → 其他文件无法修改动态密钥
9. acwk 用 -O3 编译 → Log 输出被优化掉
10. acwk 先于 saac 启动 → 占用 9400 端口导致 saac bind 失败
11. 栈上分配 128KB 缓冲区做加密 → gmsv CPU 99.9% 假死
12. imgno=1 → CHAR_checkPlayerImageNumber 失败（有效范围 100000+）
13. 登录响应 result="uk"（固定密钥编码）→ **仅账号 1 能登录**（`uk`与`ok`在账号1动态密钥下巧合同为 `{sF`），Athena 等账号必失败
14. 登录响应 checksum 用 0x00FE0680 公式（kvU 版）→ 账号 1 也失败（客户端校验 strlen 公式）
15. 假设"客户端换账号必须重启"→ **错误**，win 版实证客户端自动重设动态密钥
16. 角色列表请求 checksum 用 if(0) 永久跳过 → 现已恢复真校验（动态密钥下解出 0 匹配）

---

## 八、当前状态

- [x] 编译通过（GCC 14 + -fcommon）
- [x] saac + acwk + gmsv 三服务稳定运行
- [x] 客户端握手（'L'）
- [x] 登录（result="uk"）
- [x] 动态密钥切换（cdkey+"www.longzoro.com"）
- [x] 角色列表（saac 文本直接转发，无硬编码）
- [x] 创建角色（字段解码正确，checksum 匹配）
- [x] 角色登录（"successful"，动态密钥编码）
- [x] 进入游戏（地图、NPC、宠物正常）
- [x] 遇敌战斗
- [x] 登出
- [x] 乱码修复（NPC 对话/聊天/地名）
- [x] 资源加载修复（MAX_MAP_FILES 2000，地图 1443/NPC 7397 与 win 一致）
- [x] setup.cf 官方校准（删 4 无效键 / 改 30 值 / 补 6 功能键）
- [x] 全部临时硬编码移除（角色列表/角色名/checksum 绕过）
- [x] 双端源码同步（D 盘权威 ↔ WSL 运行副本，sync_src.sh）

---

## 八·五、win 版抓包逐字节对比（2026-09-08 登录链路定论）

**方法**：启动 win 版 9.0 服务端（管理器启动 gmsv），代理 9065→9066 抓包，`decode_msg3.py` 解码明文，与 WSL 服务端响应逐字节对比。

**关键结论**：WSL 服务端与 win 版响应**完全一致**（以下为 Athena/terminal 账号实测）：

| 消息 | win 版 | WSL（修复后） |
|---|---|---|
| 登录响应 | `&;95;kvU;0p7aja;#;` | `&;95;kvU;0p7aja;#;` ✅ |
| 角色列表请求（客户端发） | `&;92;0pdaja;#;` | 同（动态密钥编码 checksum=0）|
| 角色列表响应 | `&;103;oXFprq}byhgk{Q;;0p7Yja;#;` | `&;103;oXFprq}byhgk{Q;;0p7Yja;#;` ✅ |

**定论**：
1. 登录响应 result=`"ok"`（动态密钥 `cdkey+www.longzoro.com` 编码：1→`{sF`、Athena→`kvU`）
2. 登录响应 checksum = `strlen("ok")`=2（动态编码 `0p7aja`）
3. 角色列表响应 result=`"successful"`（动态编码 `oXFprq}byhgk{Q`）、checksum=10
4. 客户端切换账号自动重设动态密钥（无需重启客户端）

---

## 九、乱码修复（NPC 对话 / 聊天 / 地名）

**现象**：NPC 对话框"加加的瘁郊樱"、聊天框乱码。

**根因**（双重）：
1. `autil.c` iconv 混转函数**逐字节误判**：GBK 流中连续字节恰为合法 UTF-8 即误转（实锤：`B4 E5 B3 A4`="村长" 中 `E5 B3 A4` 恰是 UTF-8 "哉"）。
2. `char.c` `CHAR_appendNameAndTitle` 冒号用 UTF-8 冒号字节（`EF BC 9A`），客户端按 GBK 解析变乱码。

**修复**：
1. `autil.c:592` 新增 `str_is_valid_utf8()` **整串校验**，只有整串合法 UTF-8 才转 GBK。
2. 冒号改 GBK 转义 `\xa3\xba`。

验证：`加加的村长: 欢迎来到加鲁卡东部森林的村庄加加！` 正常。

---

## 十、资源加载修复：MAX_MAP_FILES 1300 → 2000

**现象**：WSL 地图 1289 / NPC 7085，与 win 原版（1443 / 7397）不一致，日志大量 `地图ID号有毛病`（312 条）。

**根因**：`readmap.c` 的 `#define MAX_MAP_FILES 1300` 是 8.0 上限，9.0 数据 map/ 有 1459 个文件，1300 之后全部未扫描 → 引用这些地图的 NPC 创建失败。

**修复**：`gmsv/src/map/readmap.c:605` 启用 2000（源码自带注释行互换），重编 map 子库 + 顶层链接。

**验证**：地图 1443、NPC 7397 与 win 完全一致，`地图ID号有毛病` 和 `MAP_addNewObj error` 清零。

---

## 十一、setup.cf 官方校准（2026-09-08）

按官方 8.0 参考（`librae8226/gmsv`、`19900623/stoneage`）校准，**删 4 无效键 / 改 30 值 / 补 6 功能键**，详见同目录 `setup.cf对比分析_Win_vs_Linux.md` 第五节。要点：
- 删除：`petskill2file`、`setpetskillfile`、`UNLAWWARPFLOOR`、`NPRIDE`（均源码不解析）
- 补充：`FLOOR1-5`（禁止互传地图，已生效）、`storechar`
- 校准：`battleexp 100→1`、`TRANS 1→0`、`GOLD 30000→100000`、`ENEMYACTION 10→3`、`BATTLEGOLD 1→0`、`TRANSPOINTUP` 6转 610 等
- 保留：`ITEM1=24114`（9.0 数据无官方出生物品 ID）、`MAXLEVEL=140`（官方值）、`LEVEL=140`（绑定 9.0 exp 数据）

---

## 十二、双端源码同步

**约定**：Windows `D:\StoneAge\` 权威（UTF-8）↔ WSL `~/StoneAge/` 运行副本，任何修改双向同步。
**脚本**：`~/StoneAge/sync_src.sh`（rsync 双端 + 全量重编译）。
**坑**：`D:\StoneAge_gmsv_src_before_gbk.tar.gz` 名字误导，实为 GBK 实验态快照，不可当 UTF-8 基准。源码/setup.cf 保持 UTF-8（GBK 编译产生 17 万 U+FFFD）。
