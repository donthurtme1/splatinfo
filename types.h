#ifndef TYPES_H
#define TYPES_H

enum mode {
	TURF = 1,
	RAIN = 2,
	AREA = 3,
	TOWER = 4,
	CLAM = 5,
	TRICOLOR = 6
};

enum stage {
	SCORCH_GORGE = 1,
	EELTAIL_ALLEY,
	HAGGLEFISH_MARKET,
	UNDERTOW_SPILLWAY,
	UMAMI_RUINS,
	MINCEMEAT_METALWORKS,
	BRINEWATER_SPRINGS,
	BARNACLE_AND_DIME,
	FLOUNDER_HEIGHTS,
	HAMMERHEAD_BRIDGE,
	MUSEUM_D_ALFONSINO,
	MAHI_MAHI_RESORT,
	INKBLOT_ART_ACADEMY,
	STURGEON_SHIPYARD,
	MAKOMART,
	WAHOO_WORLD,
	HUMPBACK_PUMP_TRACK,
	MANTA_MARIA,
	CRABLEG_CAPITAL,
	SHIPSHAPE_CARGO_CO,
	ROBO_ROM_EN,
	BLUEFIN_DEPOT,
	MARLIN_AIRPORT,
	LEMIURA_HUB
};

static const char *stage_str[] = {
	"(null)",
	"Scorch Gorge",
	"Eeltail Alley",
	"Hagglefish Market",
	"Undertow Spillway",
	"Um'ami Ruins",
	"Mincemeat Metalworks",
	"Brinewater Springs",
	"Barnacle & Dime",
	"Flounder Heights",
	"Hammerhead Bridge",
	"Museum d'Alfonsino",
	"Mahi-Mahi Resort",
	"Inkblot Art Academy",
	"Sturgeon Shipyard",
	"MakoMart",
	"Wahoo World",
	"Humpback Pump Track",
	"Manta Maria",
	"Crableg Capital",
	"Shipshape Cargo Co.",
	"Robo ROM-en",
	"Bluefin Depot",
	"Marlin Airport",
	"Lemiura Hub"
};

typedef struct {
	char * regular_mode;
	enum stage regular_stage[2];
	char * series_mode;
	enum stage series_stage[2];
	char * open_mode;
	enum stage open_stage[2];
	char * x_mode;
	enum stage x_stage[2];
} Rotation;

#endif
