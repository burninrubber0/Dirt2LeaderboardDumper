#include <httplib.h>
#include <tinyxml2.h>

#include <map>
#include <memory>
#include <string>

class Dumper
{
public:
	Dumper(int argc, char* argv[]);
	int result = 0;
	
private:
	const int entriesPerRequest = 256; // Limited to 256 entries per request
	const int numBoards = 289; // 289 leaderboards numbered sequentially
	const int numDisciplines = 7;
	
	void dumpLeaderboard();
	void setupRequest(tinyxml2::XMLDocument& request);
	void updateRequest(tinyxml2::XMLDocument& request, int boardId, int entryIndex, int entryCount);
	int getTotal(httplib::Result result);
	tinyxml2::XMLDocument* mergeSegments(std::vector<tinyxml2::XMLDocument*>& segments, int boardId);
	void save(tinyxml2::XMLDocument* board, bool saveAsCsv, int trackId, int disciplineId, bool isTimeBoard = true);
	std::string makeCsv(tinyxml2::XMLDocument* board, bool isTimeBoard = true);
	std::string formatAsTime(std::string milliseconds);

	const std::map<int, std::string> boards = {
		{ 1, "Baja - Ensenada Sprint" },
		{ 8, "Baja - Ensenada" },
		{ 15, "Baja - Spillway" },
		{ 22, "Baja - Spillway Short" },
		{ 29, "Baja - Spillway Long" },
		{ 36, "Baja - Peninsula Run" },
		{ 43, "China - Li River" },
		{ 50, "China - Li River Hillclimb" },
		{ 57, "China - Yu Yong" },
		{ 64, "China - Yu Yong B" },
		{ 71, "Croatia - Mali Alan Pass" },
		{ 78, "Croatia - Hairpin Run" },
		{ 85, "Croatia - Chapel Run" },
		{ 92, "Croatia - Novigrad" },
		{ 99, "Japan - Shibuya" },
		{ 106, "Japan - Shibuya B" },
		{ 113, "London - Battersea Bridge" },
		{ 120, "London - Battersea Air" },
		{ 127, "L.A. - Marina Air" },
		{ 134, "L.A. - Marina Sprint" },
		{ 141, "Malaysia - Ladang Long" },
		{ 148, "Malaysia - Ladang Sprint" },
		{ 155, "Malaysia - Ladang Mine Run" },
		{ 162, "Malaysia - Rawang Trail" },
		{ 169, "Malaysia - Tanamera" },
		{ 176, "Malaysia - Tanamera Falls" },
		{ 183, "Morocco - Kasbah Run" },
		{ 190, "Morocco - Ait Benhaddou" },
		{ 197, "Morocco - Tamdaght" },
		{ 204, "Morocco - Kasbah Trail" },
		{ 211, "Morocco - Ounila Descent" },
		{ 218, "Morocco - Ounila Valley" },
		{ 225, "Morocco - Hammada Circuit" },
		{ 232, "Morocco - Hammada Sprint" },
		{ 239, "Utah - Kane Creek" },
		{ 246, "Utah - Hurrah Pass" },
		{ 253, "China - Li River Descent" },
		{ 260, "China - Yu Yong Sprint" },
		{ 267, "China - Yu Yong Sprint B" },
		{ 274, "London - Battersea Bridge B" },
		{ 281, "L.A. - Stadium Sprint" },
		{ 288, "Online Fame" },
		{ 289, "Dirt Tour XP" }
	};
	const std::map<int, std::string> disciplines = {
		{ 0, "Rally" },
		{ 1, "Rallycross" },
		{ 2, "Trailblazer" },
		{ 3, "Stock Baja" },
		{ 4, "Raid T1" },
		{ 5, "Trophy Trucks" },
		{ 6, "Class 1 Buggies" },
	};
	const std::map<int, std::string> vehicles = {
		{ 70, "Kincaid Ford F-150 Trophy Truck" },
		{ 73, "Chevrolet Silverado CK-1500" },
		{ 75, "West Coast Choppers Stuka TT" },
		{ 76, "Dodge Ram Trophy Truck" },
		{ 78, "Colin McRae R4" },
		{ 79, "Colin McRae R4" },
		{ 80, "Colin McRae R4" },
		{ 81, "Mitsubishi Lancer Evolution X" },
		{ 82, "Mitsubishi Lancer Evolution X" },
		{ 83, "Mitsubishi Lancer Evolution X" },
		{ 84, "Subaru Impreza WRX STI" },
		{ 85, "Subaru Impreza WRX STI" },
		{ 86, "Subaru Impreza WRX STI" },
		{ 87, "Subaru Impreza STI Group N" },
		{ 88, "Subaru Impreza STI Group N" },
		{ 89, "Subaru Impreza STI Group N" },
		{ 90, "Hummer HX" },
		{ 92, "Toyota FJ Cruiser" },
		{ 94, "Honda Ridgeline" },
		{ 96, "Dodge Power Wagon" },
		{ 98, "Herbst Smithbuilt Buggy" },
		{ 100, "PRC-1 Buggy" },
		{ 102, "DeJong MXR" },
		{ 104, "Brian Ickler Buggy" },
		{ 106, "Mitsubishi Racing Lancer" },
		{ 108, "VW Race Touareg 2" },
		{ 110, "Bowler Nemesis" },
		{ 112, "Hummer H3" },
		{ 114, "Nissan 350Z" },
		{ 115, "Nissan 350Z" },
		{ 116, "Nissan 350Z" },
		{ 117, "Pontiac Solstice GXP" },
		{ 118, "Pontiac Solstice GXP" },
		{ 119, "Pontiac Solstice GXP" },
		{ 120, "BMW Z4 M Coupe Motorsport" },
		{ 121, "BMW Z4 M Coupe Motorsport" },
		{ 122, "BMW Z4 M Coupe Motorsport" },
		{ 123, "Mitsubishi Eclipse GT" },
		{ 124, "Mitsubishi Eclipse GT" },
		{ 125, "Mitsubishi Eclipse GT" },
		{ 126, "Toyota Stadium Truck" },
		{ 127, "MG Metro 6R4" },
		{ 128, "Ford RS200 Evolution" },
		{ 129, "Mitsubishi Pajero Dakar 1993" },
		{ 130, "Dallenbach Special" },
		{ 131, "Ford Escort MK II" },
		{ 132, "1995 Subaru Impreza WRX STI" },
		{ 133, "Mitsubishi Lancer Evolution IX" },
		{ 134, "Mitsubishi Lancer Evolution IX" },
		{ 135, "Mitsubishi Lancer Evolution IX" },
		{ 137, "Subaru Impreza WRX STI [X Games]" },
		{ 138, "Mitsubishi Lancer Evolution X [X]" },
		{ 140, "Colin McRae R4 [X Games]" }
	};
	const std::map<int, std::string> grades = {
		{ 0, "All-Star" },
		{ 1, "Pro" },
		{ 2, "Rookie" }
	};
};
