#!/bin/bash
# ============================================
# 石器时代服务一键启动脚本 (saac + acwk + gmsv)
# 用法:
#   bash start_all.sh        # 全部后台运行(推荐, 关终端不杀服务)
#   bash start_all.sh -f     # gmsv 前台运行(调试用, 实时看日志)
#   bash start_all.sh stop   # 全部停止
# ============================================

case "$1" in
  stop)
    echo "停止全部服务..."
    pkill -x gmsv 2>/dev/null || true
    pkill -x acwk 2>/dev/null || true
    pkill -x saac 2>/dev/null || true
    sleep 2
    ss -tln | grep -E '9066|9200|9400' || echo "已全部停止"
    exit 0
    ;;
esac

cd ~/StoneAge
touch gmsv/lockip.txt

echo "[1/4] 停止旧进程..."
pkill -x gmsv 2>/dev/null || true
pkill -x acwk 2>/dev/null || true
pkill -x saac 2>/dev/null || true
sleep 2

echo "[2/4] 启动 saac (9200)..."
cd ~/StoneAge/saac && nohup ./saac > /tmp/saac.log 2>&1 & disown
while ! ss -tln | grep -q ':9200'; do sleep 1; done
echo "      saac 就绪 (9200)"

echo "[3/4] 启动 acwk worker (必须, 否则角色列表报 server load too high)..."
cd ~/StoneAge/saac/wk && nohup ./acwk > /tmp/acwk.log 2>&1 & disown
sleep 2
echo "      acwk 已启动 (连 SAAC 9200 注册 worker)"

echo "[4/4] 启动 gmsv (9066)..."
cd ~/StoneAge/gmsv
if [ "$1" = "-f" ]; then
  # 前台调试模式: Ctrl+C 停止, 不影响 saac/acwk
  ./gmsv 2>&1 | tee /tmp/gmsv.log
else
  nohup ./gmsv > /tmp/gmsv.log 2>&1 & disown
  echo "      等待 gmsv 加载地图 (约20秒)..."
  sleep 20
  while ! ss -tln | grep -q ':9066'; do sleep 2; done
  echo "      gmsv 就绪 (9066)"
fi

echo ""
echo "=== 服务状态 ==="
ss -tln | grep -E '9066|9200|9400'
echo ""
echo "查看日志:  tail -f /tmp/saac.log  /tmp/acwk.log  /tmp/gmsv.log"
echo "停止服务:  bash start_all.sh stop"
