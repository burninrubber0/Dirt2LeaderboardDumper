# DiRT 2 Leaderboard Dumper DLL

Scrapes the leaderboards for Colin McRae: Dirt 2.

As the name suggests, this is done via DLL injection. Put simply, the DLL:

- Waits 60 seconds for the user to log into GFWL,
- Calls `XUserCreateStatsEnumeratorByRank()` and `XEnumerate()` for the specified leaderboards and ranks, and
- Saves the data as CSV.

A configuration file `dumper_config.yaml` can be used. If it isn't, the entire Dirt 2 PC leaderboard is dumped by default. It contains the following options:

- `titleId`: Hexadecimal title ID of the game to be dumped. Supported title IDs are 0x434D0819 (Dirt 2 X360) and 0x434D0820 (Dirt 2 PC).
- `startLeaderboard` and `endLeaderboard`: Inclusive range of the leaderboards IDs to be dumped. Use an end of `0` to dump all leaderboards.
- `startRank` and `endRank`: Inclusive range of the ranks to be dumped. Use an end of `0` to dump all ranks.

Using a rank limit is strongly recommended. A complete Dirt 2 X360 leaderboard dump takes roughly 52 hours. This is because a single request takes about 3/4 of a second and there's a limit of 100 rows per request. Async requests were attempted, but the server appears to place them in a queue, so any speed increase from doing so is negligible.

Support for Dirt 3 is planned. See [issue #2](https://github.com/burninrubber0/Dirt2LeaderboardDumper/issues/2).

# Credits

- SubstituteR for [DllWrapper](https://github.com/SubstituteR/DllWrapper), the 32-bit version of which is used as a base.
- fktn-k for [fkYaml](https://github.com/fktn-k/fkYAML), used for YAML handling.
