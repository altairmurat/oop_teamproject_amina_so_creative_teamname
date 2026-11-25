#!/usr/bin/env python3
"""Amina's end-to-end runner: drives the REAL ./atm in a pseudo-terminal, like a person typing.
Run from the repo root:   python3 tests/amina_tests/e2e/run_e2e.py [scenario-id ...]
Each scenario restarts ./atm, so every test starts from the same initial state.
Transcripts are written to docs/transcripts/<id>.txt (what a screenshot of the console would show)."""
import os, pty, select, sys, time

ATM_CMD = ["./atm", "data/initial_conditions.txt"]
OUT_DIR = "docs/transcripts"

# ATM list: 1 = 000111 Kakao Single Unilingual, 2 = 000222 IM Multi Bilingual
# Chris = card 1111-1111-1111 / 1234 (Kakao), Andor = card 2222-2222-2222 / 4321 (IM)
C = ["1111-1111-1111", "1234"]
SCENARIOS = {
 # ---- sessions (REQ2.x) ----
 "s01_two_sessions_ids_and_summary": ("REQ2.1-2.4",
   ["1"] + C[:1] + [C[1], "1","1","0","0","0","4", "2","2000", "4",      # session 1: deposit 4x1000, withdraw 2000, cancel -> summary
    "1"] + C[:1] + [C[1], "1","2","100000", "4",                          # session 2: check deposit 100000 -> ID must continue
    "/", "0"]),
 "s02_no_summary_when_nothing_done": ("REQ2.3",
   ["1", C[0], C[1], "4", "0"]),
 # ---- card / password (REQ3.x) ----
 "s03_single_bank_rejects_other_bank_card": ("REQ3.1, 3.2",
   ["1", "2222-2222-2222", "0"]),
 "s04_multi_bank_accepts_other_bank_card": ("REQ3.1",
   ["2", C[0], "1", C[1], "4", "0"]),
 "s05_wrong_password_once_then_ok": ("REQ3.4",
   ["1", C[0], "9999", C[1], "4", "0"]),
 "s06_wrong_password_three_times": ("REQ3.5",
   ["1", C[0], "0000", "1111", "2222", "0"]),
 # ---- deposit (REQ4.x) ----
 "d01_cash_deposit_primary": ("REQ4.1, 4.3, 4.5",
   ["/", "1", C[0], C[1], "1","1","0","0","0","4", "4", "/", "0"]),
 "d02_cash_deposit_nonprimary_fee": ("REQ4.4, 4.5",
   ["2", C[0], "1", C[1], "1","1","0","0","0","5", "0","0","0","2", "4", "/", "0"]),
 "d03_fee_mismatch_ends_session": ("REQ4.4, 9.1",
   ["2", C[0], "1", C[1], "1","1","0","0","0","5", "0","0","0","1", "/", "0"]),
 "d04_more_than_50_bills_rejected": ("REQ4.2",
   ["1", C[0], C[1], "1","1","0","0","30","30", "/", "0"]),
 "d05_check_deposit_no_atm_cash": ("REQ4.6",
   ["1", C[0], C[1], "1","2","150000", "4", "/", "0"]),
 "d06_check_below_minimum": ("REQ4.1 (valid check)",
   ["1", C[0], C[1], "1","2","99999", "/", "0"]),
 # ---- withdrawal (REQ5.x) ----
 "w01_fewest_bills_17000": ("REQ5.1, 5.3, 5.4, 5.5",
   ["1", C[0], C[1], "1","2","100000", "2","17000", "4", "/", "0"]),
 "w02_insufficient_funds": ("REQ5.2",
   ["1", C[0], C[1], "2","3000", "/", "0"]),
 "w03_insufficient_atm_cash": ("REQ5.2",
   ["1", C[0], C[1], "1","2","400000", "2","200000", "/", "0"]),
 "w04_over_500000": ("REQ5.7",
   ["1", C[0], C[1], "1","2","1000000", "2","600000", "/", "0"]),
 "w05_three_ok_fourth_refused": ("REQ5.6",
   ["1", C[0], C[1], "1","2","1000000", "2","1000", "2","1000", "2","1000", "2", "/", "0"]),
 "w06_nonprimary_withdraw_fee_3000": ("REQ5.4",
   ["2", C[0], "1", C[1], "1","1","0","0","0","10", "0","0","0","2", "2","5000", "4", "/", "0"]),
 "w07_not_multiple_of_1000": ("REQ5.1",
   ["1", C[0], C[1], "1","2","100000", "2","2500", "/", "0"]),
 # ---- language of my messages (REQ8 / REQ4, 5) ----
 "k01_korean_deposit_error": ("REQ8.2 (Amina's messages)",
   ["2", C[0], "2", C[1], "1","2","99999", "/", "0"]),
}

def run(script, delay=0.25):
    pid, fd = pty.fork()
    if pid == 0:
        os.execvp(ATM_CMD[0], ATM_CMD)
    out = bytearray()
    def drain(timeout):
        got = False
        while True:
            r, _, _ = select.select([fd], [], [], timeout)
            if not r: return got
            try: data = os.read(fd, 4096)
            except OSError: return got
            if not data: return got
            out.extend(data); got = True
    drain(0.5)
    for line in script:
        os.write(fd, (line + "\n").encode()); drain(delay)
    drain(0.5)
    try: os.close(fd)
    except OSError: pass
    try: os.waitpid(pid, 0)
    except ChildProcessError: pass
    return out.decode("utf-8", "replace").replace("\r\n", "\n").replace("\r", "")

if __name__ == "__main__":
    os.makedirs(OUT_DIR, exist_ok=True)
    ids = sys.argv[1:] or list(SCENARIOS)
    for sid in ids:
        reqs, script = SCENARIOS[sid]
        text = run(script)
        header = f"# {sid}  [{reqs}]\n# typed (one per Enter): {' | '.join(script)}\n" + "-"*60 + "\n"
        with open(f"{OUT_DIR}/{sid}.txt", "w", encoding="utf-8") as f: f.write(header + text)
        print("wrote", sid)
