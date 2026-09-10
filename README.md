# AstroCampPlanner

A .NET 6 CLI demo where the user picks a national park and a constellation, 
the system finds the next date/time that constellation is highest over the park,
checks the weather bureau, and charges $5 for the service.

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

Demo payment cards (see `Clients/DialupClient`):
approve with `4242 4242 4242 4242`, decline with `4000000000000002`, both
with expiry `01/99` and CVV `123`. Anything else is rejected locally before
the modem ever dials out.

## License

MIT — see [LICENSE](LICENSE).
