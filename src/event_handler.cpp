/*
 * event_handler.cpp
 * 
 * Copyright 2018 NIVOIX Olivier <olivier@Aspire-ES1-131>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include "event_handler.hpp"
#include "event.hpp"
#include "event_manager.hpp"
#include "core_io.hpp"

void EventHandler::handleEvents() {
	while (eventManager->haveEvents()) {
		// assume e != nullptr
		const Event* e = eventManager->getEvent();
		this->handle(e);
		delete e;
	}
}

void EventHandler::handle(const Event* e) {
	//~ std::cout << "EventHandler handle" << std::endl;
	switch(e->getEventType()) {

		case Event::E_NOT_SET : 
			// std::cout << "---> test not set" << std::endl;
			break;

		case Event::E_ALTITUDE:
			// std::cout << "---> test altitude changed to " << e->getDouble() << std::endl;
			break;

		case Event::E_ATMOSPHERE:
			if (e->getBool()) {
				// std::cout << "---> test atmosphere on" << std::endl;
				coreIO->milkyWayUseIris(false);
			} else {
				// std::cout << "---> test atmosphere off" << std::endl;			
				coreIO->milkyWayUseIris(true);
			}
				// std::cout << "---> test atmosphere on" << std::endl;
			// } else {
				// std::cout << "---> test atmosphere off" << std::endl;			
			// }
			break;

		default: break;
	}	
}