#!/usr/bin/env python3
#logs de autenticação sintéticos no formato auth.log."""
import random, sys
from datetime import datetime, timedelta

ATTACKER_IPS = [
    "185.220.101.42", "45.33.32.156", "103.99.0.118",
    "178.128.88.200", "91.240.118.50",
]

ATTACK_USERS = (["root"] * 40 + ["admin"] * 25 + ["test"] * 10 +
                ["ubuntu"] * 8 + ["user"] * 7 + ["postgres"] * 5 +
                ["oracle"] * 3 + ["guest"] * 2)

LEGIT_USERS = ["joao", "maria", "pedro", "ana", "carlos"]
LEGIT_IPS = ["10.0.0.33", "10.0.0.45", "192.168.1.10", "192.168.1.20"]

def gen(n, path):
    base = datetime(2026, 6, 1)
    months = ["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"]
    
    with open(path, 'w') as f:
        for i in range(n):
            is_attack = random.random() < 0.7

            if is_attack:
                #login atacante
                ip = random.choice(ATTACKER_IPS)
                user = random.choice(ATTACK_USERS)
                failed = random.random() < 0.95
                hour = random.choices(range(24), weights=[
                    3,3,8,8,8,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,3,3
                ])[0]
            else:
                # loogins normais em horário comercial
                ip = random.choice(LEGIT_IPS)
                user = random.choice(LEGIT_USERS)
                failed = random.random() < 0.05
                hour = random.choices(range(24), weights=[
                    0,0,0,0,0,0,1,3,8,10,10,8,6,8,10,10,8,5,3,1,0,0,0,0
                ])[0]

            ts = base + timedelta(
                days=random.randint(0, 29),
                hours=hour,
                minutes=random.randint(0, 59),
                seconds=random.randint(0, 59),
            )
            mon = months[ts.month - 1]
            day = ts.day
            time_str = ts.strftime("%H:%M:%S")
            pid = random.randint(1000, 65000)

            if failed:
                invalid = "invalid user " if random.random() < 0.3 else ""
                line = f"{mon} {day:2d} {time_str} server sshd[{pid}]: Failed password for {invalid}{user} from {ip} port {random.randint(1024,65535)} ssh2"
            else:
                line = f"{mon} {day:2d} {time_str} server sshd[{pid}]: Accepted password for {user} from {ip} port {random.randint(1024,65535)} ssh2"

            f.write(line + "\n")

            if (i + 1) % 1_000_000 == 0:
                print(f"  {i+1} linhas...", file=sys.stderr)

    print(f"OK: {n} linhas -> {path}", file=sys.stderr)

if __name__ == "__main__":
    gen(int(sys.argv[1]), sys.argv[2])