#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

enum mode {
	TURF = 1,
	RAIN,
	AREA,
	LIFT,
	CLAM,
	TRICOLOR
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

static const char *mode_str[] = {
	"(null)",
	"Turf",
	"Rain",
	"Zones",
	"Tower",
	"Clams",
	"Tricolor"
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

#define container_of(ptr, type, member) \
	(type *)((unsigned long)ptr - offsetof(type, member))

#define list_for_each(pos, type, head, member) \
	for (type *pos = container_of((head)->next, type, member); \
		 &pos->member != head; \
		 pos = container_of(pos->member.next, type, member))

#define list_for_each_entry(pos, type, head) \
	for (type *pos = container_of(head, type, list); \
		 ((unsigned long)pos + offsetof(type, list)) != NULL; \
		 pos = container_of(pos->member.next, type, list))

typedef struct list_head {
	struct list_head *next, *prev;
} List;

typedef uint32_t PhaseId[3];

typedef struct {
	List list;
	enum mode regular_mode;
	enum stage regular_stage[2];
	enum mode series_mode;
	enum stage series_stage[2];
	enum mode open_mode;
	enum stage open_stage[2];
	enum mode x_mode;
	enum stage x_stage[2];
} Rotation;

typedef struct {
	enum mode mode;
	enum stage map;
} MapMode;

#endif
