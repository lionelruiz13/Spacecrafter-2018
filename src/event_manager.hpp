#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP


#include "event.hpp"
#include <list>
#include <algorithm>

class EventManager{
public:
	EventManager(EventManager const &) = delete;
	EventManager& operator = (EventManager const &) = delete;
	
	static void End() {
		if (instance != nullptr) {
			delete instance;
		}
		instance = nullptr;
	}
	
    static EventManager * getInstance(){
        return instance == nullptr ? new EventManager() : instance;
    }

	static void Init(){
		if (instance != nullptr)
			delete instance;
		instance = new EventManager();
	}

    void queue(const Event* event){
		//~ std::cout << "Add event" << std::endl;
        envents.push_back(event);
    }

	bool haveEvents() {
		return !envents.empty();
	}

	const Event* getEvent() {
		const Event* event = envents.front();
		envents.pop_front();
		return event; 
	}

private:
    EventManager(){
		//~ std::cout << "Creation de l'EventManager" << std::endl;
        // chain = new EventHandler();
    }

	~EventManager() {
		std::for_each(envents.begin(), envents.end(), [](const Event* e) { delete e; });
		envents.clear();
		//~ std::cout << "Delete EventManager" << std::endl;
        // if (chain)
		// 	delete chain;
	}

    //singleton pour ne pas changer toutes les classes du logiciel
    static EventManager * instance;
    // la liste des évènements
    std::list<const Event*> envents;
    //La chaine qui s'occuper de traiter les évenemnts
    // EventHandler * chain;
};

#endif
