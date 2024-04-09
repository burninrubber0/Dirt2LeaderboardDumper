# DiRT 2 Leaderboard Dumper
Dumps the database for the DiRT 2 PS3 leaderboards. Uses POST requests to retrieve the ranking XMLs.

Requests are limited to 256 records, so this tool gets 256-record segments and merges them into a single file for each leaderboard.

The output XML is just what's sent by the server. The CSV refines that data into something more readable and parses the info field. **Do not open the CSVs in Excel!** Excel can and will format times incorrectly. Use something like OpenOffice or just a text editor like notepad if you must.

Xbox 360 is unsupported because traffic is encrypted. PC is similar, which makes sense as it uses GFWL. It might become possible to decrypt PC packets in the future, in which case I'll update this tool.
