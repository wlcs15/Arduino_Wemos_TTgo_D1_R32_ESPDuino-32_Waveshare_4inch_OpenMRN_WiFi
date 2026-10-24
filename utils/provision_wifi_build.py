#!/usr/bin/env python3
"""Interactive Wi-Fi PSK provisioner. Run in YOUR terminal, never from Grok.

Windows cmd and Ubuntu TTY. Same refusals as provision_wifi_build.sh:
non-TTY, WIFI_PASSWORD env, --password / --psk.

  python -u utils/provision_wifi_build.py
  python -u utils/provision_wifi_build.py -p COM7 flash
  ./utils/provision_wifi_build.sh -p /dev/ttyUSB0 flash
"""

from __future__ import print_function

import getpass
import os
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
WRAP_OUT = os.path.join(ROOT, "main", "wifi_psk_wrap.inc")
IDS_DEFAULT = os.path.join(ROOT, "local", "hw_ids.env")
WRAP_PY = os.path.join(ROOT, "utils", "wifi_wrap.py")
BUILD_SH = os.path.join(ROOT, "utils", "build_idf5.sh")


def _die(msg):
    sys.stderr.write(msg + "\n")
    raise SystemExit(1)


def main():
    if not (sys.stdin.isatty() and sys.stdout.isatty() and sys.stderr.isatty()):
        _die(
            "Must be run in an interactive terminal (keyboard + screen).\n"
            "Do not run this from Grok, a pipe, or CI."
        )
    if os.environ.get("WIFI_PASSWORD"):
        _die(
            "WIFI_PASSWORD is already set in the environment.\n"
            "Unset it and type the password at the hidden prompt.\n"
            "    unset WIFI_PASSWORD\n"
            "    Remove-Item Env:WIFI_PASSWORD"
        )
    idf_args = []
    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        low = arg.lower()
        if low.startswith("--password") or low.startswith("--psk") or low.startswith(
            "--wifi-password"
        ):
            _die("Do not pass the Wi-Fi password as a command-line argument.")
        if arg in ("-p", "--port"):
            if i + 1 >= len(sys.argv):
                _die("Missing port after %s" % arg)
            idf_args.extend(["-p", sys.argv[i + 1]])
            i += 2
            continue
        idf_args.append(arg)
        i += 1

    ids = os.environ.get("WIFI_IDS_FILE", IDS_DEFAULT)
    wrap_out = os.environ.get("WIFI_WRAP_OUT", WRAP_OUT)
    if not os.path.isfile(ids):
        _die(
            "Missing %s\nFlash the DEBUG image, then:\n"
            "  python -u utils/collect_hw_ids.py --port COM7\n"
            "  python -u utils/collect_hw_ids.py --port /dev/ttyUSB0" % ids
        )

    default_ssid = os.environ.get("WIFI_SSID", "SRIF2333")
    try:
        ssid_in = input("Wi-Fi SSID [%s]: " % default_ssid).strip()
    except EOFError:
        _die("SSID prompt failed")
    ssid = ssid_in if ssid_in else default_ssid
    if not ssid:
        _die("SSID must not be empty.")

    psk = getpass.getpass("Wi-Fi password (hidden, not written as plaintext): ")
    psk2 = getpass.getpass("Again to confirm: ")
    if psk != psk2:
        _die("Passwords did not match.")
    if not psk:
        _die("Password must not be empty.")

    print("SSID in wrap blob: %s" % ssid)
    print("Encrypting on the host. Only ciphertext will be compiled in.")
    env = os.environ.copy()
    env.pop("WIFI_PASSWORD", None)
    proc = subprocess.Popen(
        [
            sys.executable,
            "-u",
            WRAP_PY,
            "encrypt",
            "--ids",
            ids,
            "--ssid",
            ssid,
            "--out",
            wrap_out,
        ],
        stdin=subprocess.PIPE,
        cwd=ROOT,
        env=env,
    )
    proc.communicate(psk.encode("utf-8"))
    psk = psk2 = "x"
    if proc.returncode != 0:
        raise SystemExit(proc.returncode)

    with open(wrap_out, "r", encoding="utf-8", errors="replace") as handle:
        text = handle.read()
    if "WIFI_PASSWORD" in text.upper() or "PSK=" in text.upper() or "password=" in text.lower():
        _die("Refusing to build: wrap file looks like it contains a password field.")

    print("Host wrap written. Building firmware (no PSK in the environment).")
    if not idf_args:
        idf_args = ["build"]
    if os.name == "nt":
        print("On Windows cmd, IDF build still needs Git Bash or an exported IDF_PATH:")
        print("  %s %s" % (BUILD_SH, " ".join(idf_args)))
        print("Wrap is done. Run that in a shell that can source ESP-IDF.")
        return 0
    cmd = ["bash", BUILD_SH] + idf_args
    raise SystemExit(subprocess.call(cmd, cwd=ROOT, env=env))


if __name__ == "__main__":
    main()
