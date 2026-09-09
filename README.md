# AstroCampPlanner

A .NET 6 CLI demo that picks a national park and a constellation, finds the
next date/time that constellation is highest over the park, checks the
weather bureau, and charges $5 for the service. It's a deliberately
old-fashioned integration: four backend "services", each speaking a
different communications technology from a different era, glued together
with no shared abstraction:

| Silo                     | Technology         | Era        |
|--------------------------|---------------------|------------|
| `ParksRpcSilo.cs`        | ONC RPC (TCP)       | ~1980s-90s |
| `WeatherSoapSilo.cs`     | SOAP / WCF          | ~2000s     |
| `AstroRestSilo.cs`       | REST / JSON         | ~2015+     |
| `DialupPaymentSilo.cs`   | Dial-up modem (shelled out to a C binary) | ~1990s |

The app is self-contained: the client libraries it links against live under
`Clients/`, and the backend servers it talks to live under `TestServers/`.
Nothing outside this directory is required to run it.

## Running it

After cloning, fetch/build everything (NuGet packages via `dotnet restore`,
then the C backends) with:

```
./setup.sh
```

or do it by hand:

```
dotnet restore
dotnet build
dotnet bin/Debug/net6.0/AstroCampPlanner.dll
```

`setup.sh` checks for `rpcgen`/`libtirpc-dev` (needed to build
`TestServers/ParksRpcServer`) before building; see **Rebuilding the
backends** below if it's missing.

On first run, each silo notices its backend isn't up yet and starts it
automatically (the RPC, SOAP, and REST servers as background TCP listeners;
the dial-up modem bank as a pty-backed process). Those processes are left
running afterward, so subsequent runs reuse them and start instantly. Kill
them yourself (`pkill -f TestServers`) if you want a clean slate, or just
reboot.

Demo payment cards (see `Clients/DialupClient` / `TestServers/DialupServer`):
approve with `4242 4242 4242 4242`, decline with `4000000000000002`, both
with expiry `01/99` and CVV `123`. Anything else is rejected locally before
the modem ever dials out.

## Environment variables

Each silo locates its backend by joining a "home" directory with a known
relative path (e.g. `<home>/astro_server`). By default that home directory
is computed from this project's own location — no path is hardcoded — but
every one of them can be overridden if you've moved a server, want to point
at a different build, or are running the backend on another machine.

| Variable                                | Default                                    | Points at |
|------------------------------------------|--------------------------------------------|-----------|
| `ASTROCAMPPLANNER_ASTRO_SERVER_HOME`      | `TestServers/AstroTransitServer`            | Directory containing the `astro_server` binary (the REST transit-time service). |
| `ASTROCAMPPLANNER_WEATHER_SERVER_HOME`    | `TestServers/WeatherSoapServer`              | Directory containing `weather.wsdl` and a `server/` subfolder with the `weather_server` binary (the SOAP weather bureau). |
| `ASTROCAMPPLANNER_PARKS_SERVER_HOME`      | `TestServers/ParksRpcServer`                 | Directory containing the `rpc_server` binary (the ONC RPC parks service). |
| `ASTROCAMPPLANNER_DIALUP_SERVER_HOME`     | `TestServers/DialupServer`                   | Directory containing the `server` binary (the modem/gateway) and where its `campplanner_modem_port` pty symlink is created. |
| `ASTROCAMPPLANNER_DIALUP_CLIENT_HOME`     | `Clients/DialupClient`                       | Directory containing the `client` binary (the dial-up terminal used to place the $5 charge). |

"Default" above means: if the variable is unset (or blank), the app walks
up from its own running location to find `AstroCampPlanner.csproj`, then
joins the path shown. This makes the out-of-the-box behavior independent of
where the repository is checked out, while still leaving every path fully
overridable.

Example — pointing the parks service at a build kept somewhere else:

```
export ASTROCAMPPLANNER_PARKS_SERVER_HOME=/opt/parks-rpc/server
dotnet bin/Debug/net6.0/AstroCampPlanner.dll
```

## Rebuilding the backends

Each subdirectory under `TestServers/` and `Clients/DialupClient` has its
own `Makefile`:

```
make -C TestServers/AstroTransitServer
make -C TestServers/WeatherSoapServer/server
make -C TestServers/ParksRpcServer
make -C TestServers/DialupServer
make -C Clients/DialupClient
```

`TestServers/ParksRpcServer` needs `libtirpc-dev` and `rpcgen` (the
checked-in `parks.h`/`parks_xdr.c`/`parks_svc.c` are generated output with
hand edits on top of the dispatcher's `main()`, so the Makefile does not
regenerate them; regenerate from `parks.x` only if you're prepared to
reapply that edit). `Clients/DialupClient` compiles its shared
protocol/serial-I/O code directly from `TestServers/DialupServer` rather
than keeping a second copy.

## License

MIT — see [LICENSE](LICENSE).
