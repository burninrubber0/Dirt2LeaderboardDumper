# DiRT 2 Leaderboard Dumper
Dumps the database for the DiRT 2 PS3 leaderboards. Uses POST requests to retrieve the ranking XMLs.

Requests are limited to 256 records, so this tool gets 256-record segments and merges them into a single file for each leaderboard.

The output XML is just what's sent by the server. The CSV refines that data into something more readable and parses the info field. **Do not open the CSVs in Excel!** Excel can and will format times incorrectly. Use something like OpenOffice or just a text editor like notepad if you must.

PC and Xbox 360 require a different approach. See the [DLL branch](https://github.com/burninrubber0/Dirt2LeaderboardDumper/tree/dll).

Support for Dirt 3 is planned. See [issue #2](https://github.com/burninrubber0/Dirt2LeaderboardDumper/issues/2).
