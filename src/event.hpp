#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>

class Event{

public :
    enum Event_Type : char { 
        E_NOT_SET,
        E_ALTITUDE,
        E_ATMOSPHERE
        //....
    };
    
    Event(){ 
        type = E_NOT_SET;
    }
    
    Event(Event_Type _type, bool _bValue){
		type=_type;
		bValue = _bValue;
	}
    
    Event(Event_Type _type, float _fValue){
		type=_type;
		fValue = _fValue;
	}
    
    Event(Event_Type _type, double _dValue){
		type=_type;
		dValue = _dValue;
	}
    
    Event(Event_Type _type, const std::string &_sValue){
		type=_type;
		sValue = _sValue;
	}

    Event_Type getEventType() const {
        return type;
    }
    
    double getDouble() const {
		return dValue;
	}
    
    float getFloat() const {
		return fValue;
	}
	
	double getBool() const {
		return bValue;
	}
	
	std::string getString() const {
		return sValue;
	}
    
protected :
    Event_Type type;
    double dValue;
    float fValue;
    bool bValue;
    std::string sValue;
};

#endif
