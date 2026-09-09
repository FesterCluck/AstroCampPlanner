#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

echo "==> Restoring .NET dependencies"
dotnet restore

echo "==> Checking for the ONC RPC toolchain (rpcgen, libtirpc)"
if ! command -v rpcgen >/dev/null || ! pkg-config --exists libtirpc 2>/dev/null; then
    echo "error: rpcgen and libtirpc-dev are required to build TestServers/ParksRpcServer." >&2
    echo "       On Debian/Kali: sudo apt-get install -y libtirpc-dev" >&2
    exit 1
fi

echo "==> Building the C backends and the dial-up client"
make -C TestServers/AstroTransitServer
make -C TestServers/WeatherSoapServer/server
make -C TestServers/ParksRpcServer
make -C TestServers/DialupServer
make -C Clients/DialupClient

echo "==> Building AstroCampPlanner"
dotnet build

echo
echo "Done. Run with: dotnet bin/Debug/net6.0/AstroCampPlanner.dll"
