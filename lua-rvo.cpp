
#include "lua.hpp"

#include "RVO.h"
#include "RVOSimulator.h"
#include "Vector2.h"

#ifndef M_PI
const float M_PI = 3.14159265358979323846f;
#endif

static RVO::RVOSimulator * check_userdata(lua_State *L, int idx) {
	void *ret = lua_touserdata(L, idx);
	luaL_argcheck(L, ret != NULL, idx, "Userdata should not be NULL");
	return (RVO::RVOSimulator *) ret;
}

static void retVector(lua_State *L, RVO::Vector2 v) {
	// pos.x
	lua_pushstring(L, "x");
	lua_pushnumber(L, v.x());
	lua_rawset(L, -2);
	// pos.y
	lua_pushstring(L, "y");
	lua_pushnumber(L, v.y());
	lua_rawset(L, -2);
}

static int lnew(lua_State *L) {
	RVO::RVOSimulator *sim = new RVO::RVOSimulator();
	lua_pushlightuserdata(L, sim);
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_setmetatable(L, -2);
	return 1;
}

static int ldelete(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
	delete sim;
	sim = NULL;
	return 0;
} 

static int lget_time_step(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	lua_pushnumber(L, sim->getTimeStep());
	return 1;
}

static int lset_time_step(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
	sim->setTimeStep(lua_tonumber(L, 2));
	return 0;
}

//float timeStep, float neighborDist, size_t maxNeighbors, 
//float timeHorizon, float timeHorizonObst, float radius, float maxSpeed, const Vector2 &velocity
static int lset_agent_defaults(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	float neighbor_dist = lua_tonumber(L, 2);
	int max_neighbors = lua_tointeger(L, 3);
	float time_horizon = lua_tonumber(L, 4);
	float time_horizon_obst = lua_tonumber(L, 5);
	float radius = lua_tonumber(L, 6);
	float max_speed = lua_tonumber(L, 7);
	float vx = luaL_optnumber(L, 8, 0.0);
	float vy = luaL_optnumber(L, 9, 0.0);
	sim->setAgentDefaults(neighbor_dist, max_neighbors, time_horizon, 
												time_horizon_obst, radius, max_speed, RVO::Vector2(vx, vy));  
	return 0;
}

static int ladd_agent(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	int n = lua_gettop(L) - 1;
	float x = lua_tonumber(L, 2);
	float y = lua_tonumber(L, 3);

	int id;
	if (n == 2) {
		id = sim->addAgent(RVO::Vector2(x, y));
	} else if (n == 8 || n == 10) {
		float neighbor_dist = lua_tonumber(L, 4);
		int max_neighbors = lua_tointeger(L, 5);
		float time_horizon = lua_tonumber(L, 6);
		float time_horizon_obst = lua_tonumber(L, 7);
		float radius = lua_tonumber(L, 8);
		float max_speed = lua_tonumber(L, 9);
		float vx = luaL_optnumber(L, 10, 0.0);
		float vy = luaL_optnumber(L, 11, 0.0);
		id = sim->addAgent(RVO::Vector2(x, y), neighbor_dist, max_neighbors, time_horizon,
				time_horizon_obst, radius, max_speed, RVO::Vector2(vx, vy));
	} else {
		return luaL_error(L, "addAgent args count error");
	}
	lua_pushinteger(L, id);
	return 1;
}

static int ladd_obstacle(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	int n = lua_gettop(L)- 1;
	if (n % 2 != 0)
		return luaL_error(L, "addObstacle args count must be even");

	std::vector<RVO::Vector2> obstacle;
	for (int i = 0; i < n/2; i++)
	{
		float x = lua_tonumber(L, 2 * i + 1);
		float y = lua_tonumber(L, 2 * i + 2);
		obstacle.push_back(RVO::Vector2(x, y));
	}

	int no = sim->addObstacle(obstacle);
	if (no == RVO::RVO_ERROR)
		no = -1;
	lua_pushinteger(L, no);

	return 1;
}

static int lprocess_obstacle(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	sim->processObstacles();
	return 0;
}

static int ldoStep(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
	sim->doStep();
	return 0;
}

static int lget_agent_pref_velocity(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	int id = lua_tointeger(L, 2);
	RVO::Vector2 v = sim->getAgentPrefVelocity(id);
	lua_pushnumber(L, v.x());
	lua_pushnumber(L, v.y());
	return 2;
}

static int lset_agent_pref_velocity(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	int n = lua_gettop(L) - 1;
	if (n != 3)
		return luaL_error(L, "setAgentPrefVelocity args count error");

	int id = lua_tointeger(L, 2);
	float x = lua_tonumber(L, 3);
	float y = lua_tonumber(L, 4);
	sim->setAgentPrefVelocity(id, RVO::Vector2(x, y));
	return 0;
}

static int lget_agent_velocity(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	int id = lua_tointeger(L, 2);
	RVO::Vector2 pos = sim->getAgentVelocity(id);
	lua_pushnumber(L, pos.x());
	lua_pushnumber(L, pos.y());
	return 2;
}

static int lset_agent_velocity(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int n = lua_gettop(L) - 1;
	if (n != 3)
		return luaL_error(L, "setAgentVelocity args count error");

	int id = lua_tointeger(L, 2);
	float x = lua_tonumber(L, 3);
	float y = lua_tonumber(L, 4);
	sim->setAgentVelocity(id, RVO::Vector2(x, y));
	return 0;
}

static int lget_agent_max_speed(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int id = lua_tointeger(L, 2);
	float ms = sim->getAgentMaxSpeed(id);
	lua_pushnumber(L, ms);
	return 1;
}

static int lset_agent_max_speed(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int n = lua_gettop(L) - 1;
	if (n != 2)
		return luaL_error(L, "setAgentMaxSpeed args count error");

	int id = lua_tointeger(L, 2);
	float ms = lua_tonumber(L, 3);
	sim->setAgentMaxSpeed(id, ms);
	return 0;
}

static int lget_agent_radius(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	int id = lua_tointeger(L, 2);
	float ms = sim->getAgentRadius(id);
	lua_pushnumber(L, ms);
	return 1;
}

static int lset_agent_radius(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int n = lua_gettop(L) - 1;
	if (n != 2)
		return luaL_error(L, "setAgentRadius args count error");

	int id = lua_tointeger(L, 2);
	float ms = lua_tonumber(L, 3);
	sim->setAgentRadius(id, ms);
	return 0;
}

static int lget_agent_time_horizon(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	int id = lua_tointeger(L, 2);	
	float ms = sim->getAgentTimeHorizon(id);
	lua_pushnumber(L, ms);
	return 1;
}

static int lset_agent_time_horizon(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int n = lua_gettop(L) - 1;
	if (n != 2)
		return luaL_error(L, "setAgentTimeHorizon args count error");

	int id = lua_tointeger(L, 2);
	float ms = lua_tonumber(L, 3);

	sim->setAgentTimeHorizon(id, ms);
	return 0;
}

static int lget_agent_time_horizon_obst(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	int id = lua_tointeger(L, 2);	
	float ms = sim->getAgentTimeHorizonObst(id);
	lua_pushnumber(L, ms);
	return 1;
}

static int lset_agent_time_horizon_obst(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int n = lua_gettop(L) - 1;
	if (n != 2)
		return luaL_error(L, "setAgentTimeHorizonObst args count error");

	int id = lua_tointeger(L, 2);
	float ms = lua_tonumber(L, 3);

	sim->setAgentTimeHorizonObst(id, ms);
	return 0;
}

static int lget_agent_max_neighbors(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int id = lua_tointeger(L, 2);
	float ms = sim->getAgentMaxNeighbors(id);
	lua_pushnumber(L, ms);
	return 1;
}

static int lset_agent_max_neighbors(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int n = lua_gettop(L) - 1;
	if (n != 2)
		return luaL_error(L, "setAgentMaxNeighbors args count error");
	
	int id = lua_tointeger(L, 2);
	float ms = lua_tonumber(L, 3);
	sim->setAgentMaxNeighbors(id, ms);
	return 0;
}

static int lget_agent_position(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int id = lua_tointeger(L, 2);
	RVO::Vector2 pos = sim->getAgentPosition(id);
	lua_pushnumber(L, pos.x());
	lua_pushnumber(L, pos.y());
	return 2;
}

static int lset_agent_position(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int n = lua_gettop(L) - 1;
	if (n != 3)
		return luaL_error(L, "setAgentPosition args count error");

	int id = lua_tointeger(L, 2);
	float x = lua_tonumber(L, 3);
	float y = lua_tonumber(L, 4);
	sim->setAgentPosition(id, RVO::Vector2(x, y));
	return 0;
}

static int lget_agent_neighbors_num(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
		
	int id = lua_tointeger(L, 2);
	size_t num = sim->getAgentNumAgentNeighbors(id);
	lua_pushinteger(L, num);
	return 1;
}

static int lget_agent_neighbor(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);

	int id = lua_tointeger(L, 2);
	int idx = lua_tointeger(L, 3) -1;
	
	int nid = sim->getAgentAgentNeighbor(id, idx);
	lua_pushinteger(L, nid);
	return 1;
}

static int lget_global_time(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
	float num = sim->getGlobalTime();
	lua_pushnumber(L, num);
	return 1;
}

static int lget_num_agents(lua_State *L) {
	RVO::RVOSimulator *sim = check_userdata(L, 1);
	size_t num = sim->getNumAgents();
	lua_pushinteger(L, num);
	return 1;
}

static void
create_meta(lua_State *L, luaL_Reg *l, const char *name) {
	int n = 0;
	while(l[n].name)
		++n;
	lua_newtable(L);
	lua_createtable(L, 0, n);
	int i;
	for (i=0;i<n;i++) {
		lua_pushcfunction(L, l[i].func);
		lua_setfield(L, -2, l[i].name);
	}
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, name);
	lua_setfield(L, -2, "__metatable");
}

extern "C" int luaopen_rvo (lua_State* L) {
	luaL_checkversion(L);
	lua_newtable(L);

	luaL_Reg l[] = {
		{ "getTimeStep", lget_time_step},
		{ "setTimeStep", lset_time_step},
		{ "setAgentDefaults", lset_agent_defaults},
		{ "addAgent", ladd_agent},
		{ "addObstacle", ladd_obstacle},
		{ "processObstacles", lprocess_obstacle},
		{ "doStep", ldoStep},

		{ "getAgentNumAgentNeighbors", lget_agent_neighbors_num },
		{ "getAgentAgentNeighbor", lget_agent_neighbor },
		{ "getNumAgents", lget_num_agents },
		{ "getGlobalTime", lget_global_time },

		{ "getAgentPrefVelocity", lget_agent_pref_velocity},
		{ "setAgentPrefVelocity", lset_agent_pref_velocity},
		{ "getAgentVelocity", lget_agent_velocity},
		{ "setAgentVelocity", lset_agent_velocity},
		{ "getAgentMaxSpeed", lget_agent_max_speed},
		{ "setAgentMaxSpeed", lset_agent_max_speed},
		{ "getAgentPosition", lget_agent_position},
		{ "setAgentPosition", lset_agent_position},
		{ "getAgentRadius", lget_agent_radius},
		{ "setAgentRadius", lset_agent_radius},
		{ "getAgentTimehorizon", lget_agent_time_horizon},
		{ "setAgentTimeHorizon", lset_agent_time_horizon},
		{ "getAgentTimehorizonObst", lget_agent_time_horizon_obst},
		{ "setAgentTimeHorizonObst", lset_agent_time_horizon_obst},
		{ "getAgentMaxNeighbors", lget_agent_max_neighbors},
		{ "setAgentMaxNeighbors", lset_agent_max_neighbors},
		{ NULL, NULL },
	};

	create_meta(L, l, "rvo");
	lua_pushcclosure(L, lnew, 1);
	lua_setfield(L, -2, "new");
	lua_pushcfunction(L, ldelete);
	lua_setfield(L, -2, "delete");
	
	return 1;
}