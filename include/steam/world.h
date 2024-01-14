#include <stdbool.h>

struct world_conf;

static const char data_dumper[] = 
"--[[ DataDumper.lua\n"
"Copyright (c) 2007 Olivetti-Engineering SA\n"
"\n"
"Permission is hereby granted, free of charge, to any person\n"
"obtaining a copy of this software and associated documentation\n"
"files (the \"Software\"), to deal in the Software without\n"
"restriction, including without limitation the rights to use,\n"
"copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
"copies of the Software, and to permit persons to whom the\n"
"Software is furnished to do so, subject to the following\n"
"conditions:\n"
"\n"
"The above copyright notice and this permission notice shall be\n"
"included in all copies or substantial portions of the Software.\n"
"\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
"EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES\n"
"OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n"
"NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\n"
"HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,\n"
"WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
"FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n"
"OTHER DEALINGS IN THE SOFTWARE.\n"
"]]\n"
"\n"
"local dumplua_closure = [[\n"
"local closures = {}\n"
"local function closure(t)\n"
"  closures[#closures+1] = t\n"
"  t[1] = assert(loadstring(t[1]))\n"
"  return t[1]\n"
"end\n"
"\n"
"for _,t in pairs(closures) do\n"
"  for i = 2,#t do\n"
"    debug.setupvalue(t[1], i-1, t[i])\n"
"  end\n"
"end\n"
"]]\n"
"\n"
"local lua_reserved_keywords = {\n"
"	'and', 'break', 'do', 'else', 'elseif', 'end', 'false', 'for',\n"
"	'function', 'if', 'in', 'local', 'nil', 'not', 'or', 'repeat',\n"
"	'return', 'then', 'true', 'until', 'while' }\n"
"\n"
"local function keys(t)\n"
"	local res = {}\n"
"	local oktypes = { stringstring = true, numbernumber = true }\n"
"	local function cmpfct(a,b)\n"
"		if oktypes[type(a)..type(b)] then\n"
"			return a < b\n"
"		else\n"
"			return type(a) < type(b)\n"
"		end\n"
"	end\n"
"	for k in pairs(t) do\n"
"		res[#res+1] = k\n"
"	end\n"
"	table.sort(res, cmpfct)\n"
"	return res\n"
"end\n"
"\n"
"local c_functions = {}\n"
"for _,lib in pairs{'_G', 'string', 'table', 'math',\n"
"				   'io', 'os', 'coroutine', 'package', 'debug'} do\n"
"	local t = _G[lib] or {}\n"
"	lib = lib .. \".\"\n"
"	if lib == \"_G.\" then lib = \"\" end\n"
"	for k,v in pairs(t) do\n"
"		if type(v) == 'function' and not pcall(string.dump, v) then\n"
"			c_functions[v] = lib..k\n"
"		end\n"
"	end\n"
"end\n"
"\n"
"function DataDumper(value, varname, fastmode, ident)\n"
"	local defined, dumplua = {}\n"
"	-- Local variables for speed optimization\n"
"	local string_format, type, string_dump, string_rep =\n"
"	string.format, type, string.dump, string.rep\n"
"	local tostring, pairs, table_concat =\n"
"	tostring, pairs, table.concat\n"
"	local keycache, strvalcache, out, closure_cnt = {}, {}, {}, 0\n"
"	setmetatable(strvalcache, {__index = function(t,value)\n"
"		local res = string_format('%q', value)\n"
"		t[value] = res\n"
"		return res\n"
"	end})\n"
"	local fcts = {\n"
"		string = function(value) return strvalcache[value] end,\n"
"		number = function(value) return value end,\n"
"		boolean = function(value) return tostring(value) end,\n"
"		['nil'] = function(value) return 'nil' end,\n"
"		['function'] = function(value)\n"
"			return string_format(\"loadstring(%q)\", string_dump(value))\n"
"		end,\n"
"		userdata = function() error(\"Cannot dump userdata\") end,\n"
"		thread = function() error(\"Cannot dump threads\") end,\n"
"	}\n"
"	local function test_defined(value, path)\n"
"		if defined[value] then\n"
"			if path:match(\"^getmetatable.*%)$\") then\n"
"				out[#out+1] = string_format(\"s%s, %s)\\n\", path:sub(2,-2), defined[value])\n"
"			else\n"
"				out[#out+1] = path .. \" = \" .. defined[value] .. \"\\n\"\n"
"			end\n"
"			return true\n"
"		end\n"
"		defined[value] = path\n"
"	end\n"
"	local function make_key(t, key)\n"
"		local s\n"
"		if type(key) == 'string' and key:match('^[_%a][_%w]*$') then\n"
"			s = key .. \"=\"\n"
"		else\n"
"			s = \"[\" .. dumplua(key, 0) .. \"]=\"\n"
"		end\n"
"		t[key] = s\n"
"		return s\n"
"	end\n"
"	for _,k in ipairs(lua_reserved_keywords) do\n"
"		keycache[k] = '[\"'..k..'\"] = '\n"
"	end\n"
"	if fastmode then\n"
"		fcts.table = function (value)\n"
"			-- Table value\n"
"			local numidx = 1\n"
"			out[#out+1] = \"{\"\n"
"			for key,val in pairs(value) do\n"
"				if key == numidx then\n"
"					numidx = numidx + 1\n"
"				else\n"
"					out[#out+1] = keycache[key]\n"
"				end\n"
"				local str = dumplua(val)\n"
"				out[#out+1] = str..\",\"\n"
"			end\n"
"			if string.sub(out[#out], -1) == \",\" then\n"
"				out[#out] = string.sub(out[#out], 1, -2);\n"
"			end\n"
"			out[#out+1] = \"}\"\n"
"			return \"\"\n"
"		end\n"
"	else\n"
"		fcts.table = function (value, ident, path)\n"
"			if test_defined(value, path) then return \"nil\" end\n"
"			-- Table value\n"
"			local sep, str, numidx, totallen = \" \", {}, 1, 0\n"
"			local meta, metastr = (debug or getfenv()).getmetatable(value)\n"
"			if meta then\n"
"				ident = ident + 1\n"
"				metastr = dumplua(meta, ident, \"getmetatable(\"..path..\")\")\n"
"				totallen = totallen + #metastr + 16\n"
"			end\n"
"			for _,key in pairs(keys(value)) do\n"
"				local val = value[key]\n"
"				local s = \"\"\n"
"				local subpath = path or \"\"\n"
"				if key == numidx then\n"
"					subpath = subpath .. \"[\" .. numidx .. \"]\"\n"
"					numidx = numidx + 1\n"
"				else\n"
"					s = keycache[key]\n"
"					if not s:match \"^%[\" then subpath = subpath .. \".\" end\n"
"					subpath = subpath .. s:gsub(\"%s*=%s*$\",\"\")\n"
"				end\n"
"				s = s .. dumplua(val, ident+1, subpath)\n"
"				str[#str+1] = s\n"
"				totallen = totallen + #s + 2\n"
"			end\n"
"			if totallen > 80 then\n"
"				sep = \"\\n\" .. string_rep(\"  \", ident+1)\n"
"			end\n"
"			str = \"{\"..sep..table_concat(str, \",\"..sep)..\" \"..sep:sub(1,-3)..\"}\"\n"
"			if meta then\n"
"				sep = sep:sub(1,-3)\n"
"				return \"setmetatable(\"..sep..str..\",\"..sep..metastr..sep:sub(1,-3)..\")\"\n"
"			end\n"
"			return str\n"
"		end\n"
"		fcts['function'] = function (value, ident, path)\n"
"			if test_defined(value, path) then return \"nil\" end\n"
"			if c_functions[value] then\n"
"				return c_functions[value]\n"
"			elseif debug == nil or debug.getupvalue(value, 1) == nil then\n"
"				return string_format(\"loadstring(%q)\", string_dump(value))\n"
"			end\n"
"			closure_cnt = closure_cnt + 1\n"
"			local res = {string.dump(value)}\n"
"			for i = 1,math.huge do\n"
"				local name, v = debug.getupvalue(value,i)\n"
"				if name == nil then break end\n"
"				res[i+1] = v\n"
"			end\n"
"			return \"closure \" .. dumplua(res, ident, \"closures[\"..closure_cnt..\"]\")\n"
"		end\n"
"	end\n"
"	function dumplua(value, ident, path)\n"
"		return fcts[type(value)](value, ident, path)\n"
"	end\n"
"	if varname == nil then\n"
"		varname = \"return \"\n"
"	elseif varname:match(\"^[%a_][%w_]*$\") then\n"
"		varname = varname .. \" = \"\n"
"	end\n"
"	if fastmode then\n"
"		setmetatable(keycache, {__index = make_key })\n"
"		out[1] = varname\n"
"		table.insert(out,dumplua(value, 0))\n"
"		return table.concat(out)\n"
"	else\n"
"		setmetatable(keycache, {__index = make_key })\n"
"		local items = {}\n"
"		for i=1,10 do items[i] = '' end\n"
"		items[3] = dumplua(value, ident or 0, \"t\")\n"
"		if closure_cnt > 0 then\n"
"			items[1], items[6] = dumplua_closure:match(\"(.*\\n)\\n(.*)\")\n"
"			out[#out+1] = \"\"\n"
"		end\n"
"		if #out > 0 then\n"
"			items[2], items[4] = \"local t = \", \"\\n\"\n"
"			items[5] = table.concat(out)\n"
"			items[7] = varname .. \"t\"\n"
"		else\n"
"			items[2] = varname\n"
"		end\n"
"		return table.concat(items)\n"
"	end\n"
"end\n"
,
table_merger[] = 
"function TableMerger(table1, table2)\n"
"    for k, v in pairs(table2) do\n"
"        if (type(v) == \"table\") and type(table1[k]) == \"table\" then\n"
"            TableMerger(table1[k], v)\n"
"        else\n"
"            table1[k] = v\n"
"        end       \n"
"    end\n"
"\n"
"    return table1\n"
"end\n"
,
leveldataoverride_master[] = 
"return {\n"
"  desc=\"标准《饥荒》体验。\",\n"
"  hideminimap=false,\n"
"  id=\"SURVIVAL_TOGETHER\",\n"
"  location=\"forest\",\n"
"  max_playlist_position=999,\n"
"  min_playlist_position=0,\n"
"  name=\"生存\",\n"
"  numrandom_set_pieces=4,\n"
"  override_level_string=false,\n"
"  overrides={\n"
"    alternatehunt=\"default\",\n"
"    angrybees=\"default\",\n"
"    antliontribute=\"default\",\n"
"    autumn=\"default\",\n"
"    bananabush_portalrate=\"default\",\n"
"    basicresource_regrowth=\"none\",\n"
"    bats_setting=\"default\",\n"
"    bearger=\"default\",\n"
"    beefalo=\"default\",\n"
"    beefaloheat=\"default\",\n"
"    beequeen=\"default\",\n"
"    bees=\"default\",\n"
"    bees_setting=\"default\",\n"
"    berrybush=\"default\",\n"
"    birds=\"default\",\n"
"    boons=\"default\",\n"
"    branching=\"default\",\n"
"    brightmarecreatures=\"default\",\n"
"    bunnymen_setting=\"default\",\n"
"    butterfly=\"default\",\n"
"    buzzard=\"default\",\n"
"    cactus=\"default\",\n"
"    cactus_regrowth=\"default\",\n"
"    carrot=\"default\",\n"
"    carrots_regrowth=\"default\",\n"
"    catcoon=\"default\",\n"
"    catcoons=\"default\",\n"
"    chess=\"default\",\n"
"    cookiecutters=\"default\",\n"
"    crabking=\"default\",\n"
"    crow_carnival=\"default\",\n"
"    darkness=\"default\",\n"
"    day=\"default\",\n"
"    deciduousmonster=\"default\",\n"
"    deciduoustree_regrowth=\"default\",\n"
"    deerclops=\"default\",\n"
"    dragonfly=\"default\",\n"
"    dropeverythingondespawn=\"default\",\n"
"    evergreen_regrowth=\"default\",\n"
"    extrastartingitems=\"default\",\n"
"    eyeofterror=\"default\",\n"
"    fishschools=\"default\",\n"
"    flint=\"default\",\n"
"    flowers=\"default\",\n"
"    flowers_regrowth=\"default\",\n"
"    frograin=\"default\",\n"
"    frogs=\"default\",\n"
"    fruitfly=\"default\",\n"
"    ghostenabled=\"always\",\n"
"    ghostsanitydrain=\"always\",\n"
"    gnarwail=\"default\",\n"
"    goosemoose=\"default\",\n"
"    grass=\"default\",\n"
"    grassgekkos=\"default\",\n"
"    hallowed_nights=\"default\",\n"
"    has_ocean=true,\n"
"    healthpenalty=\"always\",\n"
"    hound_mounds=\"default\",\n"
"    houndmound=\"default\",\n"
"    hounds=\"default\",\n"
"    hunger=\"default\",\n"
"    hunt=\"default\",\n"
"    keep_disconnected_tiles=true,\n"
"    klaus=\"default\",\n"
"    krampus=\"default\",\n"
"    layout_mode=\"LinkNodesByKeys\",\n"
"    lessdamagetaken=\"none\",\n"
"    liefs=\"default\",\n"
"    lightcrab_portalrate=\"default\",\n"
"    lightning=\"default\",\n"
"    lightninggoat=\"default\",\n"
"    loop=\"default\",\n"
"    lureplants=\"default\",\n"
"    malbatross=\"default\",\n"
"    marshbush=\"default\",\n"
"    merm=\"default\",\n"
"    merms=\"default\",\n"
"    meteorshowers=\"default\",\n"
"    meteorspawner=\"default\",\n"
"    moles=\"default\",\n"
"    moles_setting=\"default\",\n"
"    monkeytail_portalrate=\"default\",\n"
"    moon_berrybush=\"default\",\n"
"    moon_bullkelp=\"default\",\n"
"    moon_carrot=\"default\",\n"
"    moon_fissure=\"default\",\n"
"    moon_fruitdragon=\"default\",\n"
"    moon_hotspring=\"default\",\n"
"    moon_rock=\"default\",\n"
"    moon_sapling=\"default\",\n"
"    moon_spider=\"default\",\n"
"    moon_spiders=\"default\",\n"
"    moon_starfish=\"default\",\n"
"    moon_tree=\"default\",\n"
"    moon_tree_regrowth=\"default\",\n"
"    mosquitos=\"default\",\n"
"    mushroom=\"default\",\n"
"    mutated_hounds=\"default\",\n"
"    no_joining_islands=true,\n"
"    no_wormholes_to_disconnected_tiles=true,\n"
"    ocean_bullkelp=\"default\",\n"
"    ocean_seastack=\"ocean_default\",\n"
"    ocean_shoal=\"default\",\n"
"    ocean_waterplant=\"ocean_default\",\n"
"    ocean_wobsterden=\"default\",\n"
"    palmcone_seed_portalrate=\"default\",\n"
"    palmconetree=\"default\",\n"
"    palmconetree_regrowth=\"default\",\n"
"    penguins=\"default\",\n"
"    penguins_moon=\"default\",\n"
"    perd=\"default\",\n"
"    petrification=\"default\",\n"
"    pigs=\"default\",\n"
"    pigs_setting=\"default\",\n"
"    pirateraids=\"default\",\n"
"    ponds=\"default\",\n"
"    portal_spawnrate=\"default\",\n"
"    portalresurection=\"none\",\n"
"    powder_monkey_portalrate=\"default\",\n"
"    prefabswaps_start=\"default\",\n"
"    rabbits=\"default\",\n"
"    rabbits_setting=\"default\",\n"
"    reeds=\"default\",\n"
"    reeds_regrowth=\"default\",\n"
"    regrowth=\"default\",\n"
"    resettime=\"default\",\n"
"    rifts_enabled=\"default\",\n"
"    rifts_frequency=\"default\",\n"
"    roads=\"default\",\n"
"    rock=\"default\",\n"
"    rock_ice=\"default\",\n"
"    saltstack_regrowth=\"default\",\n"
"    sapling=\"default\",\n"
"    season_start=\"default\",\n"
"    seasonalstartingitems=\"default\",\n"
"    shadowcreatures=\"default\",\n"
"    sharks=\"default\",\n"
"    spawnmode=\"fixed\",\n"
"    spawnprotection=\"default\",\n"
"    specialevent=\"default\",\n"
"    spider_warriors=\"default\",\n"
"    spiderqueen=\"default\",\n"
"    spiders=\"default\",\n"
"    spiders_setting=\"default\",\n"
"    spring=\"default\",\n"
"    squid=\"default\",\n"
"    stageplays=\"default\",\n"
"    start_location=\"default\",\n"
"    summer=\"default\",\n"
"    summerhounds=\"default\",\n"
"    tallbirds=\"default\",\n"
"    task_set=\"default\",\n"
"    temperaturedamage=\"default\",\n"
"    tentacles=\"default\",\n"
"    terrariumchest=\"default\",\n"
"    touchstone=\"default\",\n"
"    trees=\"default\",\n"
"    tumbleweed=\"default\",\n"
"    twiggytrees_regrowth=\"default\",\n"
"    walrus=\"default\",\n"
"    walrus_setting=\"default\",\n"
"    wasps=\"default\",\n"
"    weather=\"default\",\n"
"    wildfires=\"default\",\n"
"    winter=\"default\",\n"
"    winterhounds=\"default\",\n"
"    winters_feast=\"default\",\n"
"    wobsters=\"default\",\n"
"    world_size=\"default\",\n"
"    wormhole_prefab=\"wormhole\",\n"
"    year_of_the_beefalo=\"default\",\n"
"    year_of_the_bunnyman=\"default\",\n"
"    year_of_the_carrat=\"default\",\n"
"    year_of_the_catcoon=\"default\",\n"
"    year_of_the_gobbler=\"default\",\n"
"    year_of_the_pig=\"default\",\n"
"    year_of_the_varg=\"default\" \n"
"  },\n"
"  playstyle=\"survival\",\n"
"  random_set_pieces={\n"
"    \"Sculptures_2\",\n"
"    \"Sculptures_3\",\n"
"    \"Sculptures_4\",\n"
"    \"Sculptures_5\",\n"
"    \"Chessy_1\",\n"
"    \"Chessy_2\",\n"
"    \"Chessy_3\",\n"
"    \"Chessy_4\",\n"
"    \"Chessy_5\",\n"
"    \"Chessy_6\",\n"
"    \"Maxwell1\",\n"
"    \"Maxwell2\",\n"
"    \"Maxwell3\",\n"
"    \"Maxwell4\",\n"
"    \"Maxwell6\",\n"
"    \"Maxwell7\",\n"
"    \"Warzone_1\",\n"
"    \"Warzone_2\",\n"
"    \"Warzone_3\" \n"
"  },\n"
"  required_prefabs={ \"multiplayer_portal\" },\n"
"  required_setpieces={ \"Sculptures_1\", \"Maxwell5\" },\n"
"  settings_desc=\"标准《饥荒》体验。\",\n"
"  settings_id=\"SURVIVAL_TOGETHER\",\n"
"  settings_name=\"生存\",\n"
"  substitutes={  },\n"
"  version=4,\n"
"  worldgen_desc=\"标准《饥荒》体验。\",\n"
"  worldgen_id=\"SURVIVAL_TOGETHER\",\n"
"  worldgen_name=\"生存\" \n"
"}\n"
,
leveldataoverride_caves[] = 
"return {\n"
"  background_node_range={ 0, 1 },\n"
"  desc=\"探查洞穴…… 一起！\",\n"
"  hideminimap=false,\n"
"  id=\"DST_CAVE\",\n"
"  location=\"cave\",\n"
"  max_playlist_position=999,\n"
"  min_playlist_position=0,\n"
"  name=\"洞穴\",\n"
"  numrandom_set_pieces=0,\n"
"  override_level_string=false,\n"
"  overrides={\n"
"    atriumgate=\"default\",\n"
"    banana=\"default\",\n"
"    basicresource_regrowth=\"none\",\n"
"    bats=\"default\",\n"
"    bats_setting=\"default\",\n"
"    beefaloheat=\"default\",\n"
"    berrybush=\"default\",\n"
"    boons=\"default\",\n"
"    branching=\"default\",\n"
"    brightmarecreatures=\"default\",\n"
"    bunnymen=\"default\",\n"
"    bunnymen_setting=\"default\",\n"
"    cave_ponds=\"default\",\n"
"    cave_spiders=\"default\",\n"
"    cavelight=\"default\",\n"
"    chess=\"default\",\n"
"    crow_carnival=\"default\",\n"
"    darkness=\"default\",\n"
"    day=\"default\",\n"
"    daywalker=\"default\",\n"
"    dropeverythingondespawn=\"default\",\n"
"    dustmoths=\"default\",\n"
"    earthquakes=\"default\",\n"
"    extrastartingitems=\"default\",\n"
"    fern=\"default\",\n"
"    fissure=\"default\",\n"
"    flint=\"default\",\n"
"    flower_cave=\"default\",\n"
"    flower_cave_regrowth=\"default\",\n"
"    fruitfly=\"default\",\n"
"    ghostenabled=\"always\",\n"
"    ghostsanitydrain=\"always\",\n"
"    grass=\"default\",\n"
"    grassgekkos=\"default\",\n"
"    hallowed_nights=\"default\",\n"
"    healthpenalty=\"always\",\n"
"    hunger=\"default\",\n"
"    krampus=\"default\",\n"
"    layout_mode=\"RestrictNodesByKey\",\n"
"    lessdamagetaken=\"none\",\n"
"    lichen=\"default\",\n"
"    liefs=\"default\",\n"
"    lightflier_flower_regrowth=\"default\",\n"
"    lightfliers=\"default\",\n"
"    loop=\"default\",\n"
"    marshbush=\"default\",\n"
"    merms=\"default\",\n"
"    molebats=\"default\",\n"
"    moles_setting=\"default\",\n"
"    monkey=\"default\",\n"
"    monkey_setting=\"default\",\n"
"    mushgnome=\"default\",\n"
"    mushroom=\"default\",\n"
"    mushtree=\"default\",\n"
"    mushtree_moon_regrowth=\"default\",\n"
"    mushtree_regrowth=\"default\",\n"
"    nightmarecreatures=\"default\",\n"
"    pigs_setting=\"default\",\n"
"    portalresurection=\"none\",\n"
"    prefabswaps_start=\"default\",\n"
"    reeds=\"default\",\n"
"    regrowth=\"default\",\n"
"    resettime=\"default\",\n"
"    rifts_enabled_cave=\"default\",\n"
"    rifts_frequency_cave=\"default\",\n"
"    roads=\"never\",\n"
"    rock=\"default\",\n"
"    rocky=\"default\",\n"
"    rocky_setting=\"default\",\n"
"    sapling=\"default\",\n"
"    season_start=\"default\",\n"
"    seasonalstartingitems=\"default\",\n"
"    shadowcreatures=\"default\",\n"
"    slurper=\"default\",\n"
"    slurtles=\"default\",\n"
"    slurtles_setting=\"default\",\n"
"    snurtles=\"default\",\n"
"    spawnmode=\"fixed\",\n"
"    spawnprotection=\"default\",\n"
"    specialevent=\"default\",\n"
"    spider_dropper=\"default\",\n"
"    spider_hider=\"default\",\n"
"    spider_spitter=\"default\",\n"
"    spider_warriors=\"default\",\n"
"    spiderqueen=\"default\",\n"
"    spiders=\"default\",\n"
"    spiders_setting=\"default\",\n"
"    start_location=\"caves\",\n"
"    task_set=\"cave_default\",\n"
"    temperaturedamage=\"default\",\n"
"    tentacles=\"default\",\n"
"    toadstool=\"default\",\n"
"    touchstone=\"default\",\n"
"    trees=\"default\",\n"
"    weather=\"default\",\n"
"    winters_feast=\"default\",\n"
"    world_size=\"default\",\n"
"    wormattacks=\"default\",\n"
"    wormhole_prefab=\"tentacle_pillar\",\n"
"    wormlights=\"default\",\n"
"    worms=\"default\",\n"
"    year_of_the_beefalo=\"default\",\n"
"    year_of_the_bunnyman=\"default\",\n"
"    year_of_the_carrat=\"default\",\n"
"    year_of_the_catcoon=\"default\",\n"
"    year_of_the_gobbler=\"default\",\n"
"    year_of_the_pig=\"default\",\n"
"    year_of_the_varg=\"default\" \n"
"  },\n"
"  required_prefabs={ \"multiplayer_portal\" },\n"
"  settings_desc=\"探查洞穴…… 一起！\",\n"
"  settings_id=\"DST_CAVE\",\n"
"  settings_name=\"洞穴\",\n"
"  substitutes={  },\n"
"  version=4,\n"
"  worldgen_desc=\"探查洞穴…… 一起！\",\n"
"  worldgen_id=\"DST_CAVE\",\n"
"  worldgen_name=\"洞穴\" \n"
"}\n"
;


/**
 * @brief Create a world configuration from 'leveldataoverride.lua', if file no
 *          exist, will create an instance with default value.
 * 
 * @param path Path to 'leveldataoverride.lua', must be absolute path
 * @param is_master Is the leveldataoverride for master or cave
 * @return struct world_conf* The world_conf instance, stand for the
 *                              configuration
 */
struct world_conf *new_world_conf(const char *path, bool is_master);


/**
 * @brief save world_conf instance to 'leveldataoverride.lua', the location of
 *          the 'leveldataoverride.lua' is where the file being open by
 *          'new_world_conf'
 * @param conf world configuration instances, created by 'new_world_conf()'
 */
void world_conf_save(struct world_conf *conf);


/**
 * @brief delete world_conf instance and free all the memory
 * 
 * @param conf world_conf instances, created by 'new_world_conf()'
 * @code
 * struct world_conf *conf = new_world_conf(const char *path, bool is_master);
 * world_conf_save(conf);
 * world_conf_delete(conf);
 * @endcode
 */
void world_conf_delete(struct world_conf *conf);
