/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2014 DigitalGlobe (http://www.digitalglobe.com/)
 */

/*
    fcodeCommon

    FCODE translations that are common for all products. Product specific FCODES go in the individual
    product "rules" scripts.
    
    These can be used for BOTH import and export so the values need to be unique. Non-unique values go
    in the individual product rules scripts.
*/


fcodeCommon = {
    // The fcodeXXX rules are for finding/setting an FCODE
    // Common rules: import and export with "unique" values
    one2one : [
      ['F_CODE','AA010','landuse','surface_mining'], // Extraction Mine 
      ['F_CODE','AA011','raw:fcode','AA011'], // Shear Wall 
      ['F_CODE','AA020','mine:superstructure','yes'], // Mine Shaft Superstructure 
      ['F_CODE','AA040','man_made','rig'], // Rig/Superstructure
      ['F_CODE','AA045','raw:fcode','AA045'], // Borehole 
      ['F_CODE','AA052','raw:fcode','AA052'], // Hydrocarbons Field 
      ['F_CODE','AA054','raw:fcode','AA054'], // Non-water Well 
      ['F_CODE','AB000','landuse','landfill'], // Disposal Site / Waste Pile
      ['F_CODE','AB010','amenity','recycling'], // Wrecking Yard/Scrap Yard, Recycling Site
      ['F_CODE','AB021','raw:fcode','AB021'], // Liquid Diffuser
      ['F_CODE','AB030','amenity','waste_processing'], // Waste Processing 
      ['F_CODE','AB040','landuse','aeration_basin'], // Aeration Basin
      ['F_CODE','AB507','raw:fcode','AB507'], // Waste Heap
      ['F_CODE','AC010','raw:fcode','AC010'], // Blast-furnace 
      ['F_CODE','AC020','industrial','catalytic_cracker'], // Catalytic Cracker 
      ['F_CODE','AC030','water','settling_basin'], // Settling Basin/Sludge Pond, Settling Pond
      ['F_CODE','AC040','raw:fcode','AC040'], // Hydrocarbon Products Facility, Oil/Gas Facilities
      ['F_CODE','AC060','raw:fcode','AC060'], // Industrial Furnace
      ['F_CODE','AC507','raw:fcode','AC507'], // Sewage Treatment Plant
      ['F_CODE','AD010','power','plant'], // Electric Power Station, Power Plant
      ['F_CODE','AD020','raw:fcode','AD020'], // Solar Panel 
      ['F_CODE','AD025','raw:fcode','AD025'], // Solar Farm 
      ['F_CODE','AD030','power','substation'], // Power Substation, Substation/Transformer Yard 
      ['F_CODE','AD041','raw:fcode','AD041'], // Nuclear Reactor Containment 
      ['F_CODE','AD050','raw:fcode','AD050'], // Heating Facility 
      ['F_CODE','AD055','raw:fcode','AD055'], // Cooling Facility 
      ['F_CODE','AD060','raw:fcode','AD060'], // Wind Farm 
      ['F_CODE','AF010','man_made','chimney'], // Chimney/Smokestack 
      ['F_CODE','AF020','conveyor','yes'], // Conveyor
      ['F_CODE','AF021','raw:fcode','AF021'], // Bucket Elevator 
      ['F_CODE','AF030','tower:type','cooling'], // Cooling Tower  
      ['F_CODE','AF040','man_made','crane'], // Crane 
      ['F_CODE','AF050','raw:fcode','AF050'], // Excavating Machine 
      ['F_CODE','AF060','raw:fcode','AF060'], // Engine Test Cell 
      ['F_CODE','AF070','man_made','flare'], // Flare Pipe 
      ['F_CODE','AF080','raw:fcode','AF080'], // Hopper 
      ['F_CODE','AG030','raw:fcode','AG030'], // Shopping Complex 
      ['F_CODE','AG040','raw:fcode','AG040'], // Office Park 
      ['F_CODE','AG050','advertising','billboard'], // Billboard 
      ['F_CODE','AH025','earthwork','yes'], // Engineered Earthwork 
      ['F_CODE','AH055','fortified','yes'], // Fortified Building 
      ['F_CODE','AH060','military','bunker'], // Underground Bunker 
      ['F_CODE','AH070','barrier','check_point'], // Checkpoint 
      ['F_CODE','AI020','raw:fcode','AI020'], // Caravan Park, Mobile Home/Mobile Home Park 
      ['F_CODE','AI021','raw:fcode','AI021'], // Mobile Home Park 
      ['F_CODE','AI030','camp','yes'], // Camp 
      ['F_CODE','AJ030','raw:fcode','AJ030'], // Feedlot/Stockyard, Holding Pen
      ['F_CODE','AJ050','man_made','windmill'], // Windmill
      ['F_CODE','AJ051','generator:source','wind'], // Windmotor, Wind Turbine 
      ['F_CODE','AJ055','raw:fcode','AJ055'], // Water Mill 
      ['F_CODE','AJ080','raw:fcode','AJ080'], // Stable 
      ['F_CODE','AJ085','building','barn'], // Barn 
      ['F_CODE','AJ110','building','greenhouse'], // Greenhouse 
      ['F_CODE','AK020','tourism','theme_park_attraction'], // Amusement Park Attraction 
      ['F_CODE','AK030','tourism','theme_park'], // Amusement Park 
      ['F_CODE','AK040','leisure','pitch'], // Athletic Field, Sports Ground 
      // ['F_CODE','AK050','sport','tennis'], // Tennis Court - Valid NFDD but not in TDS Spec
      ['F_CODE','AK060','tourism','camp_site'], // Campground/Campsite 
      ['F_CODE','AK061','raw:fcode','AK061'], // Picnic Site 
      ['F_CODE','AK070','raw:fcode','AK070'], // Drive In Theater 
      ['F_CODE','AK080','man_made','outdoor_theatre_screen'], // Outdoor Theatre Screen
      ['F_CODE','AK090','leisure','fairground'], // Fairground
      ['F_CODE','AK100','leisure','golf_course'], // Golf Course 
      ['F_CODE','AK101','raw:fcode','AK101'], // Golf Driving Range 
      ['F_CODE','AK110','building','grandstand'], // Grandstand 
      ['F_CODE','AK120','leisure','park'], // Park 
      ['F_CODE','AK121','raw:fcode','AK121'], // Lookout 
      ['F_CODE','AK130','leisure','track'], // Racetrack 
      ['F_CODE','AK150','raw:fcode','AK150'], // Ski-jump 
      ['F_CODE','AK155','raw:fcode','AK155'], // Ski-run 
      ['F_CODE','AK160','leisure','stadium'], // Stadium/Amphitheater 
      ['F_CODE','AK161','raw:fcode','AK161'], // Scoreboard 
      ['F_CODE','AK164','raw:fcode','AK164'], // Amphitheatre 
      ['F_CODE','AK170','leisure','swimming_pool'], // Swimming Pool 
      ['F_CODE','AK180','tourism','zoo'], // Zoo/Safari Park
      ['F_CODE','AL010','facility','yes'], // Facility 
      ['F_CODE','AL011','raw:fcode','AL011'], // Installation 
      ['F_CODE','AL012','raw:fcode','AL012'], // Archeological Site 
      ['F_CODE','AL013',undefined,undefined], // Building - See custom rules
      ['F_CODE','AL014','raw:fcode','AL014'], // Non-building Structure 
      ['F_CODE','AL017','emergency','fire_hydrant'], // Fire Hydrant 
      ['F_CODE','AL018','building:superstructure','yes'], // Building Superstructure Addition 
      ['F_CODE','AL019','building','shed'], // Shed 
      ['F_CODE','AL020','landuse','built_up_area'], // Built-up Area 
      ['F_CODE','AL025','man_made','cairn'], // Cairn 
      ['F_CODE','AL030','landuse','cemetery'], // Cemetery 
      ['F_CODE','AL036','tomb','yes'], // Tomb 
      ['F_CODE','AL060','raw:fcode','AL060'], // Dragon's Teeth 
      ['F_CODE','AL065','raw:fcode','AL065'], // Minefield 
      ['F_CODE','AL070','barrier','fence'], // Fence 
      ['F_CODE','AL073','man_made','flagpole'], // Flagpole 
      ['F_CODE','AL080','man_made','gantry'], // Gantry 
      ['F_CODE','AL099','building','hut'], // Hut 
      ['F_CODE','AL105','place','settlement'], // Settlement 
      ['F_CODE','AL110','tower:type','light'], // Light Support Structure 
      ['F_CODE','AL120','military','missile_site'], // Missile Site
      ['F_CODE','AL130','historic','monument'], // Memorial Monument 
      ['F_CODE','AL140','raw:fcode','AL140'], // Particle Accelerator 
      ['F_CODE','AL142','raw:fcode','AL142'], // Astronomical Observatory 
      ['F_CODE','AL155','overhead_obstruction','yes'], // Overhead Obstruction
      ['F_CODE','AL165','raw:fcode','AL165'], // Pipeline Or Pipe Crossing Point 
      ['F_CODE','AL170','raw:fcode','AL170'], // Plaza/City Square, Public Square 
      ['F_CODE','AL175','raw:fcode','AL175'], // Courtyard 
      ['F_CODE','AL180','raw:fcode','AL180'], // Retail Stand 
      ['F_CODE','AL195','ramp','yes'], // Ramp
      ['F_CODE','AL200','ruins','yes'], // Ruins 
      ['F_CODE','AL201','raw:fcode','AL201'], // Interest Site 
      ['F_CODE','AL208','place','shanty_town'], // Shanty Town 
      ['F_CODE','AL211','route:protection','yes'], // Transportation Route Protection Structure 
      ['F_CODE','AL241','man_made','tower'], // Tower 
      ['F_CODE','AL250','raw:fcode','AL250'], // Underground Dwelling 
      ['F_CODE','AL260','barrier','wall'], // Wall 
      ['F_CODE','AL270','landuse','plantation'], // Industrial Farm 
      ['F_CODE','AL351','space_facility','yes'], // Space Facility
      ['F_CODE','AL371','raw:fcode','AL371'], // Manor House
      ['F_CODE','AL375','historic','castle'], // Castle
      ['F_CODE','AL376','raw:fcode','AL376'], // Castle Complex
      ['F_CODE','AL510','raw:fcode','AL510'], // Tethered Balloon 
      ['F_CODE','AM010','landuse','depot'], // Storage Depot 
      ['F_CODE','AM011','shipping_container','yes'], // Shipping Container
      ['F_CODE','AM020','man_made','silo'], // Grain Bin/Silo, Grain Storage Structure 
      ['F_CODE','AM030','man_made','grain_elevator'], // Grain Elevator 
      ['F_CODE','AM040','landuse','mineral_pile'], // Mineral Pile 
      ['F_CODE','AM060','man_made','bunker'], // Surface Bunker 
      ['F_CODE','AM065','raw:fcode','AM065'], // Munition Storage Facility 
      ['F_CODE','AM070','man_made','storage_tank'], // Storage Tank 
      ['F_CODE','AM071','raw:fcode','AM071'], // Tank Farm 
      ['F_CODE','AM075','amenity','fuel_storage'], // Fuel Storage Facility 
      ['F_CODE','AM080','man_made','water_tower'], // Water Tower 
      ['F_CODE','AN010','railway','rail'], // Railway 
      ['F_CODE','AN050','railway','sidetrack'], // Railway Sidetrack 
      ['F_CODE','AN060','service','yard'], // Railway Yard 
      ['F_CODE','AN075','railway','turntable'], // Railway Turntable 
      ['F_CODE','AN076','railway','roundhouse'], // Roundhouse 
      ['F_CODE','AN085','raw:fcode','AN085'], // Railway Signal 
      ['F_CODE','AP010','highway','track'], // Cart Track 
      ['F_CODE','AP020',undefined,undefined], // Road Interchange - See RIT
      ['F_CODE','AP030',undefined,undefined], // Road - See Custom rules
      ['F_CODE','AP033','raw:fcode','AP033'], // Engineered Turn Around Site
      ['F_CODE','AP055','raw:fcode','AP055'], // Tank Trail
      ['F_CODE','AP056','raw:fcode','AP056'], // Tank Crossing
      ['F_CODE','AP040','barrier','gate'], // Gate
      ['F_CODE','AP041','barrier','yes'], // Vehicle Barrier 
      ['F_CODE','AP050','highway','path'], // Trail 
      ['F_CODE','AQ035','raw:fcode','AQ035'], // Sidewalk 
      ['F_CODE','AQ040','bridge','yes'], // Bridge 
      ['F_CODE','AQ045','bridge:span','yes'], // Bridge Span 
      ['F_CODE','AQ050','bridge:superstructure','yes'], // Bridge Superstructure
      ['F_CODE','AQ055','bridge:pylon','yes'], // Bridge Tower/Bridge Pylon 
      ['F_CODE','AQ056','bridge:pier','yes'], // Bridge Pier
      ['F_CODE','AQ059','raw:fcode','AQ059'], // Transportation Route Characteristic Change 
      ['F_CODE','AQ060','control_tower','yes'], // Control Tower 
      ['F_CODE','AQ062','crossing_point','yes'], // Crossing
      ['F_CODE','AQ063','man_made','causeway'], // Causeway Structure 
      ['F_CODE','AQ065','tunnel','culvert'], // Culvert 
      ['F_CODE','AQ068','barrier','block'], // Transportation Block 
      ['F_CODE','AQ070','route','ferry'], // Ferry Crossing 
      ['F_CODE','AQ075','raw:fcode','AQ075'], // Ice Route 
      ['F_CODE','AQ080','amenity','ferry_terminal'], // Ferry Station, Ferry Site 
      ['F_CODE','AQ090','entrance','yes'], // Entrance and/or exit - Valid NFDD but not in TDS Spec
      ['F_CODE','AQ095','tunnel','mouth'], // Tunnel Mouth 
      ['F_CODE','AQ110','raw:fcode','AQ110'], // Mooring Mast 
      ['F_CODE','AQ111','raw:fcode','AQ111'], // Prepared Watercourse Crossing 
      ['F_CODE','AQ113','man_made','pipeline'], // Pipeline 
      ['F_CODE','AQ114','raw:fcode','AQ114'], // Storm Drain 
      ['F_CODE','AQ115','raw:fcode','AQ115'], // Utility Cover 
      ['F_CODE','AQ116','man_made','pumping_station'], // Pumping Station 
      ['F_CODE','AQ118','raw:fcode','AQ118'], // Sharp Curve 
      ['F_CODE','AQ120','raw:fcode','AQ120'], // Steep Grade 
      ['F_CODE','AQ125','public_transport','station'], // Transportation Station 
      ['F_CODE','AQ130','tunnel','yes'], // Tunnel 
      ['F_CODE','AQ135','highway','rest_area'], // Roadside Rest Area
      ['F_CODE','AQ140','amenity','parking'], // Vehicle Lot, Vehicle Storage/Parking Area 
      ['F_CODE','AQ141','parking','garage'], // Parking Garage 
      ['F_CODE','AQ150','raw:fcode','AQ150'], // Stair 
      ['F_CODE','AQ151','raw:fcode','AQ151'], // Arcade 
      ['F_CODE','AQ160','highway','traffic_signals'], // Traffic Light
      ['F_CODE','AQ161','highway','street_lamp'], // Street Lamp
      ['F_CODE','AQ162','street_sign','yes'], // Street Sign
      ['F_CODE','AQ170','amenity','fuel'], // Motor Vehicle Station
      ['F_CODE','AQ180','raw:fcode','AQ180'], // Weigh Station 
      ['F_CODE','AT005','cable','yes'], // Cable 
      ['F_CODE','AT010','landmark','dish_aerial'], // Dish Aerial 
      ['F_CODE','AT011','raw:fcode','AT011'], // Aerial 
      ['F_CODE','AT012','raw:fcode','AT012'], // Aerial Farm 
      ['F_CODE','AT041','cableway','yes'], // Cableway 
      ['F_CODE','AT042','pylon','yes'], // Pylon 
      ['F_CODE','AT045','man_made','radar_station'], // Radar Station, Radar Transmitter 
      ['F_CODE','BA010','shoreline','yes'], // Coastline/Shoreline, Land Water Boundary
      ['F_CODE','BA023','foreshore','yes'], // Foreshore (Precise IHO)  - Valid NFDD Not in TDS
      ['F_CODE','BA030','place','island'], // Island 
      ['F_CODE','BA040','water','tidal'], // Tidal Water
      ['F_CODE','BA050','natural','beach'], // Beach - Valid but not in TDS Spec
      ['F_CODE','BB005','landuse','harbour'], // Harbor, Harbour 
      ['F_CODE','BB009','raw:fcode','BB009'], // Port 
      ['F_CODE','BB010','raw:fcode','BB010'], // Anchorage  - Valid NFDD Not in TDS
      ['F_CODE','BB081','shoreline_construction','yes'], // Shoreline Construction 
      ['F_CODE','BB082','leisure','slipway'], // Shoreline Ramp 
      ['F_CODE','BB090','raw:fcode','BB090'], // Dry Dock 
      ['F_CODE','BB100','seamark:fishing_facility:category','stake'], // Fishing Stakes - Valid NAS not in TDS
      ['F_CODE','BB110','raw:fcode','BB110'], // Fish Traps/Fish Weirs 
      ['F_CODE','BB155','raw:fcode','BB155'], // Maritime Signal Station - Valid NFDD Not in TDS
      ['F_CODE','BB199','raw:fcode','BB199'], // Floating Dry Dock 
      ['F_CODE','BB201','raw:fcode','BB201'], // Small Craft Facility 
      ['F_CODE','BB241','raw:fcode','BB241'], // Shipyard 
      ['F_CODE','BC010','man_made','beacon'], // Beacon - Valid NFDD Not in TDS
      ['F_CODE','BC050','man_made','lighthouse'], // Lighthouse 
      ['F_CODE','BC070','raw:fcode','BC070'], // Light Vessel 
      ['F_CODE','BD020','seamark:obstruction:category','crib'], // Crib - Valid NAS Not in TDS
      ['F_CODE','BD030','discolored_water','yes'], // Discolored Water - Valid NAS Not in TDS
      ['F_CODE','BD050','foul_ground','yes'], // Foul Ground - Valid NAS Not in TDS
      ['F_CODE','BD061','raw:fcode','BD061'], // Aquatic Vegetation - Valid NAS Not in TDS
      ['F_CODE','BD100','structural_pile','yes'], // Structural Pile
      ['F_CODE','BD115','raw:fcode','BD115'], // Offshore Construction 
      ['F_CODE','BD120','natural','reef'], // Reef - Valid NFDD Not in TDS
      ['F_CODE','BD130','seamark:obstruction:category','rock'], // Hazardous Rock  - Valid NAS Not in TDS
      ['F_CODE','BD140','seamark:obstruction:category','stump'], // Snag / Stump
      ['F_CODE','BD180','historic','wreck'], // Wreck - Valid NFDD Not in TDS
      ['F_CODE','BD181','raw:fcode','BD181'], // Hulk 
      ['F_CODE','BH010','bridge','aqueduct'], // Aqueduct
      ['F_CODE','BH012','raw:fcode','BH012'], // Qanat Shaft 
      ['F_CODE','BH015','wetland','bog'], // Bog 
      ['F_CODE','BH020','waterway','canal'], // Canal 
      ['F_CODE','BH030','waterway','ditch'], // Ditch 
      ['F_CODE','BH040','water','wastewater'], // Filtration beds/Aeration beds, Water Treatment Bed 
      ['F_CODE','BH051','place','maritime_farm'], // Fish Farm Facility 
      ['F_CODE','BH065','raw:fcode','BH065'], // Water Race 
      ['F_CODE','BH070','ford','yes'], // Ford 
      ['F_CODE','BH075','amenity','fountain'], // Fountain 
      ['F_CODE','BH077','raw:fcode','BH077'], // Hummock 
      ['F_CODE','BH082','water','lake'], // Inland Waterbody
      ['F_CODE','BH090','natural','wetland'], // Land Subject to Inundation
      ['F_CODE','BH095','wetland','yes'], // Wetland - In NFDD & NAS but not TDS
      ['F_CODE','BH100','raw:fcode','BH100'], // Moat
      ['F_CODE','BH110','raw:fcode','BH110'], // Penstock
      ['F_CODE','BH116','raw:fcode','BH116'], // Aquifer 
      ['F_CODE','BH120','waterway','rapids'], // Rapids 
      ['F_CODE','BH135','landuse','rice_field'], // Rice Field 
      ['F_CODE','BH140',undefined,undefined], // River - See custom rules
      ['F_CODE','BH145','waterway','vanishing_point'], // Vanishing point 
      ['F_CODE','BH150','raw:fcode','BH150'], // Salt Flat, Salt Pan 
      ['F_CODE','BH155','raw:fcode','BH155'], // Salt Evaporator 
      ['F_CODE','BH160','raw:fcode','BH160'], // Sabkha 
      ['F_CODE','BH165','waterway','spillway'], // Spillway 
      ['F_CODE','BH170','natural','spring'], // Natural Pool, Spring/Water Hole 
      ['F_CODE','BH180','waterway','waterfall'], // Waterfall 
      ['F_CODE','BH220','raw:fcode','BH220'], // Waterwork 
      ['F_CODE','BH230','man_made','water_well'], // Water Well
      ['F_CODE','BI005','raw:fcode','BI005'], // Vessel Lift 
      ['F_CODE','BI006','raw:fcode','BI006'], // Ship Elevator 
      ['F_CODE','BI010','cistern','yes'], // Cistern 
      ['F_CODE','BI020','waterway','dam'], // Dam/Weir 
      ['F_CODE','BI030','raw:fcode','BI030'], // Lock 
      ['F_CODE','BI040','waterway','flow_control'], // Sluice Gate
      ['F_CODE','BI044','flood_control','yes'], // Flood Control Structure
      ['F_CODE','BI045','raw:fcode','BI045'], // Basin Gate 
      ['F_CODE','BI050','tower','water_intake'], // Water Intake Tower 
      ['F_CODE','BI060','raw:fcode','BI060'], // Fish Ladder 
      ['F_CODE','BI070','raw:fcode','BI070'], // Gauging Station 
      ['F_CODE','BJ020','raw:fcode','BJ020'], // Moraine 
      ['F_CODE','BJ030','natural','glacier'], // Glacier 
      ['F_CODE','BJ031','natural','crevasse'], // Crevasse 
      ['F_CODE','BJ040','natural','ice_cliff'], // Ice Cliff 
      ['F_CODE','BJ060','natural','ice_peak'], // Ice Peak 
      ['F_CODE','BJ065','natural','ice_shelf'], // Ice Shelf 
      ['F_CODE','BJ080','raw:fcode','BJ080'], // Polar Ice 
      ['F_CODE','BJ099','raw:fcode','BJ099'], // Ice-cap 
      ['F_CODE','BJ100','landcover','snowfield'], // Snow Field and/or Ice-field
      ['F_CODE','BJ110','natural','tundra'], // Tundra 
      ['F_CODE','CA010','contour','elevation'], // Elevation Contour
      ['F_CODE','CA030','raw:fcode','CA030'], // Spot Elevation 
      ['F_CODE','DA005','raw:fcode','DA005'], // Asphalt Lake 
      ['F_CODE','DA010','raw:fcode','DA010'], // Soil Surface Region 
      ['F_CODE','DB010','natural','cliff'], // Bluff/Cliff/Escarpment, Steep Terrain Face 
      ['F_CODE','DB028','raw:fcode','DB028'], // Cave Chamber
      ['F_CODE','DB029','natural','cave_entrance'], // Cave Mouth
      ['F_CODE','DB061','natural','crevice'], // Crevice
      ['F_CODE','DB070','cut','yes'], // Cut 
      ['F_CODE','DB071','man_made','cut_edge'], // Cut Line ## In TDS, this is the edge of the cut
      ['F_CODE','DB080','raw:fcode','DB088'], // Depression 
      ['F_CODE','DB090','man_made','embankment'], // Embankment, Embankment/Fill 
      ['F_CODE','DB100','raw:fcode','DB100'], // Esker 
      ['F_CODE','DB110','raw:fcode','DB110'], // Fault, Geologic Fault 
      ['F_CODE','DB115','raw:fcode','DB155'], // Geothermal Feature, Geothermal Outlet 
      ['F_CODE','DB150','mountain_pass','yes'], // Mountain Pass 
      ['F_CODE','DB160','raw:fcode','DB160'], // Rock Formation 
      ['F_CODE','DB170','natural','dune'], // Sand Dune/Sand Hills
      ['F_CODE','DB180','natural','volcano'], // Volcano
      ['F_CODE','DB190','natural','volcanic_dyke'], // Volcanic Dyke
      ['F_CODE','DB211','raw:fcode','DB211'], // Landslide Mass 
      ['F_CODE','EA010','landuse','farmland'], // Crop Land 
      ['F_CODE','EA020','raw:fcode','EA020'], // Hedgerow 
      ['F_CODE','EA030','landuse','plant_nursery'], // Plant Nursery
      ['F_CODE','EA031','garden:type','botanical'], // Botanical Garden 
      ['F_CODE','EA040','landuse','orchard'], // Orchard/Plantation 
      ['F_CODE','EA050','landuse','vineyard'], // Vineyard
      ['F_CODE','EA055','landuse','hop_garden'], // Hop Field
      ['F_CODE','EB010','natural','grassland'], // Grassland 
      ['F_CODE','EB020','natural','thicket'], // Thicket (was FACC Scrub/Brush/Bush)
      ['F_CODE','EB070','natural','scrub'], // Brush
      ['F_CODE','EC005','natural','tree'], // Tree 
      ['F_CODE','EC010','raw:fcode','EC010'], // Bamboo/Cane 
      ['F_CODE','EC015','landuse','forest'], // Forest 
      ['F_CODE','EC020','raw:fcode','EC020'], // Oasis 
      ['F_CODE','EC040','man_made','cutline'], // Cleared Way/Cut Line/Firebreak 
      ['F_CODE','EC060','raw:fcode','EC060'], // Forest Clearing 
      ['F_CODE','ED010','wetland','marsh'], // Marsh
      ['F_CODE','ED020','wetland','swamp'], // Swamp 
      ['F_CODE','EE010','raw:fcode','EE010'], // Logging Area, Logging Site 
      ['F_CODE','EE030','raw:fcode','EE030'], // Desert 
      ['F_CODE','FA000','raw:fcode','FA000'], // Administrative Boundary 
      ['F_CODE','FA012','raw:fcode','FA012'], // Contaminated Region 
      ['F_CODE','FA015','military','range'], // Firing Range/Gunnery Range 
      ['F_CODE','FA100','raw:fcode','FA100'], // Test Area, Test Site 
      ['F_CODE','FA110','raw:fcode','FA110'], // International Date Line 
      ['F_CODE','FA165','raw:fcode','FA165'], // Training Area, Training Site 
      ['F_CODE','FA210','boundary','protected_area'], // Conservation Area
      ['F_CODE','GA034','raw:fcode','GA034'], // Aeronautical Radio Navigation Service - Valid NFDD Not in TDS
      ['F_CODE','GB005','aeroway','aerodrome'], // Land Aerodrome 
      ['F_CODE','GB015','aeroway','apron'], // Apron/Hardstand 
      ['F_CODE','GB020','aeroway','arresting_gear'], // Arresting Gear - Valid NFDD Not in TDS
      ['F_CODE','GB030','aeroway','helipad'], // Helipad 
      ['F_CODE','GB035','aeroway','heliport'], // Heliport 
      ['F_CODE','GB040','launch_pad','yes'], // Launch Pad 
      ['F_CODE','GB045','aeroway','stopway'], // Overrun/Stopway 
      ['F_CODE','GB050','military:revetment','yes'], // Aircraft/Defensive Revetment
      ['F_CODE','GB055','aeroway','runway'], // Runway 
      ['F_CODE','GB056','aeroway','runway_endpoint'], // Runway Endpoint - Valid NFDD Not in TDS
      ['F_CODE','GB065','raw:fcode','GB065'], // Seaplane Base, Water Aerodrome 
      ['F_CODE','GB070','raw:fcode','GB070'], // Seaplane Run 
      ['F_CODE','GB075','aeroway','taxiway'], // Taxiway 
      ['F_CODE','GB230','building','hangar'], // Aircraft Hangar 
      ['F_CODE','GB250','raw:fcode','GB250'], // Hardened Aircraft Shelter 
      ['F_CODE','GB485','raw:fcode','GB485'], // Approach Lighting System - Valid NFDD Not in TDS
      ['F_CODE','IA040','raw:fcode','IA040'], // Land Parcel
      ['F_CODE','SU001','military','installation'], // Military Installation
      ['F_CODE','SU004','raw:fcode','SU004'], // Cantonment Area 
      ['F_CODE','SU030','raw:fcode','SU030'], // Installation Boundary
      ['F_CODE','ZB030','raw:fcode','ZB030'], // Boundary Monument
      ['F_CODE','ZB050','man_made','survey_point'], // Survey Point
      ['F_CODE','ZD020','raw:fcode','ZD020'], // Void Collection Area
      ['F_CODE','ZD030','raw:fcode','ZD030'], // Cultural Context Location - Valid but not extracted in any TDS spec
      ['F_CODE','ZD040','named_location','yes'], // Named Location
      ['F_CODE','ZD045','annotation','yes'], // Annotated Location, Text Description
      ['F_CODE','ZD070','raw:fcode','ZD070'], // Water Measurement Location
      ['F_CODE','ZI002','raw:fcode','ZI002'], // Restriction Information
      ['F_CODE','ZI026','raw:fcode','ZI026'], // Feature Metadata
      ['F_CODE','ZI027','raw:fcode','ZI027'], // Feature Atribute Metadata
      ['F_CODE','ZI031','source:metadata','dataset'], // Dataset Metadata
      ['F_CODE','ZI039','source:metadata','entity'], // Entity Collection Metadata
      ['F_CODE','ZI040','source:metadata','spatial_entity'], // Spatial Metadata Entity
      ['F_CODE','ZI041','source:metadata','non_spatial_entity'], // Non-Spatial Metadata Entity
      ['F_CODE','ZJ001','raw:fcode','ZJ001'], // Resource Metadata Relation
    ], // End fcodeOne2one

} // End of fcodeCommon
