#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP


#include <iostream>

class CoreIO;
class core;
class EventManager;
class Event;

class EventHandler{
public:
	EventHandler(CoreIO* _coreIO, EventManager* _eventManager) {
		coreIO = _coreIO;
		eventManager = _eventManager;
	}
	EventHandler(EventHandler const &) = delete;
	EventHandler& operator = (EventHandler const &) = delete;

	~EventHandler() {
	}

	void handleEvents();
    void handle(const Event* e);

protected :
	CoreIO* coreIO = nullptr;
	EventManager* eventManager = nullptr;
};


#endif
