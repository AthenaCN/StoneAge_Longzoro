import socket, threading, sys, time, os
LOG_FILE = os.path.join(os.environ.get("TEMP", "/tmp"), "proxy_our.log")
sys.stdout = open(LOG_FILE, "a", buffering=1, encoding="utf-8", errors="replace")
LISTEN_PORT = 9065
TARGET_HOST = "127.0.0.1"
TARGET_PORT = 9066
def hexdump(data):
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_part = " ".join("%02x" % b for b in chunk)
        ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        lines.append("  %04x:  %-48s  %s" % (i, hex_part, ascii_part))
    return "\n".join(lines)
def forward(src, dst, direction):
    try:
        while True:
            data = src.recv(65536)
            if not data: break
            print("\n=== %s %s (%d bytes) ===" % (time.strftime("%H:%M:%S"), direction, len(data)))
            print(hexdump(data))
            sys.stdout.flush()
            dst.sendall(data)
    except Exception as e:
        print("[proxy] %s error: %s" % (direction, e))
    finally:
        try: src.close()
        except: pass
        try: dst.close()
        except: pass
def handle_client(client_sock, addr):
    print("[proxy] new connection from %s" % (addr,))
    try:
        server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_sock.connect((TARGET_HOST, TARGET_PORT))
        print("[proxy] connected to %s:%d" % (TARGET_HOST, TARGET_PORT))
    except Exception as e:
        print("[proxy] connect target failed: %s" % e)
        client_sock.close()
        return
    t1 = threading.Thread(target=forward, args=(client_sock, server_sock, "CLIENT->SERVER"))
    t2 = threading.Thread(target=forward, args=(server_sock, client_sock, "SERVER->CLIENT"))
    t1.daemon = t2.daemon = True
    t1.start(); t2.start()
    t1.join(); t2.join()
    print("[proxy] connection closed")
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(("0.0.0.0", LISTEN_PORT))
server.listen(5)
print("[proxy] listening on 0.0.0.0:%d -> %s:%d" % (LISTEN_PORT, TARGET_HOST, TARGET_PORT))
while True:
    client_sock, addr = server.accept()
    threading.Thread(target=handle_client, args=(client_sock, addr), daemon=True).start()
