#include "core_io.hpp"
#include "utility.hpp"

CoreIO::CoreIO(Core* _core)
{
	core = _core;
}


void CoreIO::bodyTraceHide(std::string value) const
{
	if (value=="all")
		core->bodytrace->hide(-1);
	else
		core->bodytrace->hide(Utility::strToInt(value));
}

bool CoreIO::illuminateLoad(std::string filename, double ra, double de, double angular_size, std::string name, double r, double g, double b, float rotation)
{
	bool created = false;
	// if no size, so take default value from flag illuminate_star
	if (angular_size==0.0)
		angular_size=core->illuminate_size;

	if(core->illuminates) {
		created = core->illuminates->loadIlluminate(filename, ra, de, angular_size, name, r,g,b, rotation);
	}
	return created;
}


std::string CoreIO::illuminateRemove(const std::string& name)
{
	std::string error = core->illuminates->removeIlluminate(name);
	return error;
}

std::string CoreIO::illuminateRemoveAll()
{
	return core->illuminates->removeAllIlluminate();
}

