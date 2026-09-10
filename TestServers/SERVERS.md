# Backend Servers

AstroCampPlanner integrates with four backend services, each speaking a different communications technology from a different era:

| Service | Technology | Location | Binary / Description |
|---|---|---|---|
| Parks Service | ONC RPC | `TestServers/ParksRpcServer` | `rpc_server` (parks lookup) |
| Weather Bureau | SOAP / WCF | `TestServers/WeatherSoapServer` | `weather.wsdl`, `server/weather_server` |
| Astro Transit Service | REST / JSON | `TestServers/AstroTransitServer` | `astro_server` (transit time calculation) |
| Payment Gateway | Dial-up modem | `TestServers/DialupServer` | `server` (modem/gateway creating `campplanner_modem_port` pty symlink) |

## Prerequisites

Building `TestServers/ParksRpcServer` requires `rpcgen` and `libtirpc-dev`.

On Debian / Ubuntu / Kali:
```bash
sudo apt-get install -y libtirpc-dev
```

`setup.sh` checks for `rpcgen` and `libtirpc-dev` before building. Note that the checked-in `parks.h`, `parks_xdr.c`, and `parks_svc.c` under `TestServers/ParksRpcServer` are generated output with hand edits on top of the dispatcher's `main()`, so the Makefile does not regenerate them; regenerate from `parks.x` only if you are prepared to reapply that edit.

## Rebuilding the Servers

Each server subdirectory under `TestServers/` (as well as `Clients/DialupClient`) has its own `Makefile`:

```bash
make -C TestServers/AstroTransitServer
make -C TestServers/WeatherSoapServer/server
make -C TestServers/ParksRpcServer
make -C TestServers/DialupServer
make -C Clients/DialupClient
```

`Clients/DialupClient` compiles its shared protocol and serial I/O code directly from `TestServers/DialupServer` rather than maintaining a duplicate copy.

## Environment Variables

The application locates each backend by checking the corresponding environment variable. If unset or blank, it walks up from its running location to locate `AstroCampPlanner.csproj` and resolves the default path relative to the repository root.

| Variable | Default | Points at |
|---|---|---|
| `ASTROCAMPPLANNER_ASTRO_SERVER_HOME` | `TestServers/AstroTransitServer` | Directory containing the `astro_server` binary (the REST transit-time service). |
| `ASTROCAMPPLANNER_WEATHER_SERVER_HOME` | `TestServers/WeatherSoapServer` | Directory containing `weather.wsdl` and a `server/` subfolder with the `weather_server` binary (the SOAP weather bureau). |
| `ASTROCAMPPLANNER_PARKS_SERVER_HOME` | `TestServers/ParksRpcServer` | Directory containing the `rpc_server` binary (the ONC RPC parks service). |
| `ASTROCAMPPLANNER_DIALUP_SERVER_HOME` | `TestServers/DialupServer` | Directory containing the `server` binary (the modem/gateway) and where its `campplanner_modem_port` pty symlink is created. |
| `ASTROCAMPPLANNER_DIALUP_CLIENT_HOME` | `Clients/DialupClient` | Directory containing the `client` binary (the dial-up terminal used to place the $5 charge). |

### Overriding Server Locations

You can point any service at an alternative build or remote location by setting its environment variable. For example, to point the parks service at a build kept elsewhere:

```bash
export ASTROCAMPPLANNER_PARKS_SERVER_HOME=/opt/parks-rpc/server
dotnet bin/Debug/net6.0/AstroCampPlanner.dll
```

## Demo Payment Cards

When testing dial-up card authorization (via `Clients/DialupClient` and `TestServers/DialupServer`):

- **Approve**: `4242 4242 4242 4242`, expiry `01/99`, CVV `123`
- **Decline**: `4000000000000002`, expiry `01/99`, CVV `123`

Any other card number is rejected locally by the client before dialing out.
