#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
测试脚本：5000 Hz 采样率发送 500 Hz 正弦波
用法：
    python test_5khz.py          # 单通道
    python test_5khz.py --dual   # 双通道
"""

import socket
import struct
import numpy as np
import time
import argparse
from threading import Thread, Event

class DataSender:
    MAGIC = 0x44415441  # "DATA"

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = None
        self.stop_evt = Event()

    def connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            print(f"[+] 已连接 {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"[!] 连接失败: {e}")
            return False

    def disconnect(self):
        if self.sock:
            self.sock.close()
            print("[-] 连接已断开")

    def send_packet(self, ch, fs, data, t0):
        if not self.sock:
            return False
        try:
            n = len(data)
            hdr = struct.pack("<IIIdd", self.MAGIC, ch, n, fs, t0)
            body = struct.pack(f"<{n}d", *data)
            self.sock.sendall(hdr + body)
            return True
        except Exception as e:
            print(f"[!] 发送失败: {e}")
            return False

    def stream(self, ch, fs, f_sig, amp=1.0, wave="sine"):
        pkt_dur = 0.1          # 每包 0.1 s
        pkt_size = int(fs * pkt_dur)
        print(f"[+] 通道 {ch} 开始发送：{f_sig} Hz，采样率 {fs} Hz，波形 {wave}")
        try:
            for i in range(100000):          # 足够长
                if self.stop_evt.is_set():
                    break
                t = np.linspace(i * pkt_dur, (i + 1) * pkt_dur, pkt_size, endpoint=False)
                if wave == "sine":
                    sig = amp * np.sin(2 * np.pi * f_sig * t)
                elif wave == "square":
                    sig = amp * np.sign(np.sin(2 * np.pi * f_sig * t))
                elif wave == "triangle":
                    sig = amp * (2 / np.pi) * np.arcsin(np.sin(2 * np.pi * f_sig * t))
                elif wave == "noise":
                    sig = amp * np.random.randn(len(t))
                elif wave == "composite":
                    sig = amp * (np.sin(2 * np.pi * f_sig * t) +
                                 0.5 * np.sin(2 * np.pi * f_sig * 2 * t + np.pi / 4) +
                                 0.3 * np.sin(2 * np.pi * f_sig * 3 * t + np.pi / 3))
                self.send_packet(ch, fs, sig, i * pkt_dur)
                time.sleep(pkt_dur)          # 精确 10 包/秒
        except KeyboardInterrupt:
            pass
        print(f"[-] 通道 {ch} 停止发送")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="localhost")
    ap.add_argument("--port", type=int, default=8888)
    ap.add_argument("--fs", type=float, default=5000, help="采样率")
    ap.add_argument("--freq", type=float, default=500, help="信号频率")
    ap.add_argument("--amp", type=float, default=1.0, help="幅值")
    ap.add_argument("--dual", action="store_true", help="双通道")
    ap.add_argument("--waveform", choices=["sine", "square", "triangle", "noise", "composite"],
                    default="sine", help="波形类型")
    args = ap.parse_args()

    sender = DataSender(args.host, args.port)
    if not sender.connect():
        return

    try:
        if args.dual:
            t1 = Thread(target=sender.stream, args=(1, args.fs, args.freq, args.amp, args.waveform))
            t2 = Thread(target=sender.stream, args=(2, args.fs, args.freq * 1.5, args.amp * 0.8, args.waveform))
            t1.start(); t2.start()
            t1.join();  t2.join()
        else:
            sender.stream(1, args.fs, args.freq, args.amp, args.waveform)
    except KeyboardInterrupt:
        print("\n[*] Ctrl+C pressed, stopping...")
        sender.stop_evt.set()
    finally:
        sender.disconnect()

if __name__ == '__main__':
    main()