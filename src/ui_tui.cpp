/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

// Class which manages a Text User Interface "widgets"

#include <fstream>
#include <cmath>
#include "ui_tui.hpp"
#include "app_settings.hpp"
#include "utility.hpp"
#include "log.hpp"
#include "space_date.hpp"
#include "translator.hpp"

using namespace std;
using namespace s_tui;

// Same function as getString but cleaned of every color informations
string Component::getCleanString(void)
{
	string result, s(getString());
	for (unsigned int i=0; i<s.length(); ++i) {
		if (s[i]!=startActive[0] && s[i]!=stopActive[0]) result.push_back(s[i]);
	}
	return result;
}

s_tui::Container::~Container()
{
	list<Component*>::iterator iter = childs.begin();
	while (iter != childs.end()) {
		if (*iter) {
			delete (*iter);
			(*iter)=nullptr;
		}
		iter++;
	}
}

void s_tui::Container::addComponent(Component* c)
{
	childs.push_back(c);
}

string s_tui::Container::getString(void)
{
	string s;
	list<Component*>::const_iterator iter = childs.begin();
	while (iter != childs.end()) {
		s+=(*iter)->getString();
		iter++;
	}
	return s;
}


bool s_tui::Container::onKey(SDL_Scancode k, S_TUI_VALUE s)
{
	list<Component*>::iterator iter = childs.begin();
	while (iter != childs.end()) {
		if ((*iter)->onKey(k, s)) return true;	// The signal has been intercepted
		iter++;
	}
	return false;
}

Branch::Branch() : Container()
{
	current = childs.begin();
}

string Branch::getString(void)
{
	if (!*current) return string();
	else return (*current)->getString();
}

void Branch::addComponent(Component* c)
{
	childs.push_back(c);
	if (childs.size()==1) current = childs.begin();
}

bool Branch::setValue(const string& s)
{
	list<Component*>::iterator c;
	for (c=childs.begin(); c!=childs.end(); c++) {
		if ((*c)->getCleanString()==s) {
			current = c;
			return true;
		}
	}
	return false;
}

bool Branch::setValueSpecialSlash(const string& s)
{
	list<Component*>::iterator c;
	for (c=childs.begin(); c!=childs.end(); c++) {
		string cs = (*c)->getCleanString();
		int pos = cs.find('/');
		string ccs = cs.substr(0,pos);
		if (ccs==s) {
			current = c;
			return true;
		}
	}
	return false;
}

bool Branch::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (!*current) return false;
	if (v==S_TUI_RELEASED) return (*current)->onKey(k,v);
	if (v==S_TUI_PRESSED) {
		if ((*current)->onKey(k,v)) return true;
		else {
			if (k==SDL_SCANCODE_UP) {
				if (current!=childs.begin()) --current;
				else current=--childs.end();
				return true;
			}
			if (k==SDL_SCANCODE_DOWN) {
				if (current!=--childs.end()) ++current;
				else current=childs.begin();
				return true;
			}
			return false;
		}
	}
	return false;
}

MenuBranch::MenuBranch(const string& s) : Branch(), label(s), isNavigating(false), isEditing(false)
{
}

bool MenuBranch::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (isNavigating) {
		if (isEditing) {
			if ((*Branch::current)->onKey(k, v)) return true;
			if (v==S_TUI_PRESSED && (k==SDL_SCANCODE_LEFT || k==SDL_SCANCODE_ESCAPE || k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER)) {
				isEditing = false;
				return true;
			}
			return false;
		} else {
			if (v==S_TUI_PRESSED && k==SDL_SCANCODE_UP) {
				if (Branch::current!=Branch::childs.begin()) --Branch::current;
				return true;
			}
			if (v==S_TUI_PRESSED && k==SDL_SCANCODE_DOWN) {
				if (Branch::current!=--Branch::childs.end()) ++Branch::current;
				return true;
			}
			if (v==S_TUI_PRESSED && (k==SDL_SCANCODE_RIGHT || k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER)) {
				if ((*Branch::current)->isEditable()) isEditing = true;
				return true;
			}
			if (v==S_TUI_PRESSED && (k==SDL_SCANCODE_LEFT || k==SDL_SCANCODE_ESCAPE)) {
				isNavigating = false;
				return true;
			}
			return false;
		}
	} else {
		if (v==S_TUI_PRESSED && (k==SDL_SCANCODE_RIGHT || k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER)) {
			isNavigating = true;
			return true;
		}
		return false;
	}
	return false;
}

string MenuBranch::getString(void)
{
	if (!isNavigating) return label;
	if (isEditing) (*Branch::current)->setActive(true);
	string s(Branch::getString());
	if (isEditing) (*Branch::current)->setActive(false);
	return s;
}



MenuBranchItem::MenuBranchItem(const string& s) : Branch(), label(s), isEditing(false)
{
}

bool MenuBranchItem::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (isEditing) {
		if ((*Branch::current)->onKey(k, v)) return true;
		if (v==S_TUI_PRESSED && (k==SDL_SCANCODE_LEFT || k==SDL_SCANCODE_ESCAPE || k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER)) {
			isEditing = false;
			return true;
		}
		return false;
	} else {
		if (v==S_TUI_PRESSED && k==SDL_SCANCODE_UP) {
			if (Branch::current!=Branch::childs.begin()) --Branch::current;
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (v==S_TUI_PRESSED && k==SDL_SCANCODE_DOWN) {
			if (Branch::current!=--Branch::childs.end()) ++Branch::current;
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (v==S_TUI_PRESSED && (k==SDL_SCANCODE_RIGHT || k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER)) {
			if ((*Branch::current)->isEditable()) isEditing = true;
			return true;
		}
		if (v==S_TUI_PRESSED && (k==SDL_SCANCODE_LEFT || k==SDL_SCANCODE_ESCAPE)) {
			return false;
		}
		return false;
	}
	return false;
}

string MenuBranchItem::getString(void)
{
	if (active) (*Branch::current)->setActive(true);
	string s(label + Branch::getString());
	if (active) (*Branch::current)->setActive(false);
	return s;
}

BooleanItem::BooleanItem(bool init_state, const string& _label, const string& _string_activated,
                           const string& _string_disabled) :
	Bistate(init_state), label(_label)
{
	string_activated = _string_activated;
	string_disabled = _string_disabled;
}

bool BooleanItem::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (v==S_TUI_PRESSED && (k==SDL_SCANCODE_UP || k==SDL_SCANCODE_DOWN) ) {
		state = !state;
		if (!onChangeCallback.empty()) onChangeCallback();
		return true;
	}
	return false;
}

string BooleanItem::getString(void)
{
	return label + (active ? startActive : "") +
	       (state ? string_activated : string_disabled) +
	       (active ? stopActive : "");
}

string s_tui::Integer::getString(void)
{
	return (active ? startActive : "") + Utility::doubleToString(value) + (active ? stopActive : "");
}

string Decimal::getString(void)
{
	return (active ? startActive : "") + Utility::doubleToString(value) + (active ? stopActive : "");
}


bool IntegerItem::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;
	if (!numInput) {
		if (k==SDL_SCANCODE_UP) {
			increment();
			if (value>mmax) {
				value = mmax;
			}
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDL_SCANCODE_DOWN) {
			decrement();
			if (value<mmin) {
				value = mmin;
			}
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}

		if (k==SDL_SCANCODE_KP_0 || k==SDL_SCANCODE_KP_1 || k==SDL_SCANCODE_KP_2 || k==SDL_SCANCODE_KP_3 || k==SDL_SCANCODE_KP_4 || k==SDL_SCANCODE_KP_5 ||
		        k==SDL_SCANCODE_KP_6 || k==SDL_SCANCODE_KP_7 || k==SDL_SCANCODE_KP_8 || k==SDL_SCANCODE_KP_9 || k==SDL_SCANCODE_KP_MINUS) {
			// Start editing with numerical numbers
			numInput = true;
			strInput.clear();
			char c=0;
			switch (k) {
				case SDL_SCANCODE_KP_0 :
					c=48;
					break;
				case SDL_SCANCODE_KP_1 :
					c=49;
					break;
				case SDL_SCANCODE_KP_2 :
					c=50;
					break;
				case SDL_SCANCODE_KP_3 :
					c=51;
					break;
				case SDL_SCANCODE_KP_4 :
					c=52;
					break;
				case SDL_SCANCODE_KP_5 :
					c=53;
					break;
				case SDL_SCANCODE_KP_6 :
					c=54;
					break;
				case SDL_SCANCODE_KP_7 :
					c=55;
					break;
				case SDL_SCANCODE_KP_8 :
					c=56;
					break;
				case SDL_SCANCODE_KP_9 :
					c=57;
					break;
				case SDL_SCANCODE_KP_MINUS :
					c=45;
					break;
				default:
					break;
			}
			strInput += c;

			return true;
		}
		return false;
	} else {	// numInput == true
		if (k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER) {
			numInput=false;
			istringstream is(strInput);
			is >> value;
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}

		if (k==SDL_SCANCODE_UP) {
			istringstream is(strInput);
			is >> value;
			increment();
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			strInput = Utility::doubleToString(value);
			return true;
		}
		if (k==SDL_SCANCODE_DOWN) {
			istringstream is(strInput);
			is >> value;
			decrement();
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			strInput = Utility::doubleToString(value);
			return true;
		}

		if (k==SDL_SCANCODE_KP_0 || k==SDL_SCANCODE_KP_1 || k==SDL_SCANCODE_KP_2 || k==SDL_SCANCODE_KP_3 || k==SDL_SCANCODE_KP_4 || k==SDL_SCANCODE_KP_5 ||
		        k==SDL_SCANCODE_KP_6 || k==SDL_SCANCODE_KP_7 || k==SDL_SCANCODE_KP_8 || k==SDL_SCANCODE_KP_9 || k==SDL_SCANCODE_KP_MINUS) {
			// The user was already editing
			char c = 0;
			switch (k) {
				case SDL_SCANCODE_KP_0 :
					c=48;
					break;
				case SDL_SCANCODE_KP_1 :
					c=49;
					break;
				case SDL_SCANCODE_KP_2 :
					c=50;
					break;
				case SDL_SCANCODE_KP_3 :
					c=51;
					break;
				case SDL_SCANCODE_KP_4 :
					c=52;
					break;
				case SDL_SCANCODE_KP_5 :
					c=53;
					break;
				case SDL_SCANCODE_KP_6 :
					c=54;
					break;
				case SDL_SCANCODE_KP_7 :
					c=55;
					break;
				case SDL_SCANCODE_KP_8 :
					c=56;
					break;
				case SDL_SCANCODE_KP_9 :
					c=57;
					break;
				case SDL_SCANCODE_KP_MINUS :
					c=45;
					break;
				default:
					break;
			}
			strInput += c;
			return true;
		}

		if (k==SDL_SCANCODE_ESCAPE) {
			numInput=false;
			return false;
		}
		return true; // Block every other characters
	}

	return false;
}


string IntegerItem::getString(void)
{
	if (numInput) return label + (active ? startActive : "") + strInput + (active ? stopActive : "");
	else return label + (active ? startActive : "") + Utility::doubleToString(value) + (active ? stopActive : "");
}


// logarithmic steps
int IntegerItemLogstep::increment()
{

	for (int i=1; i<10; i++) {
		if (value < pow(10.f, i)) {
			value += int(pow(10.f, i-1));
			return value;
		}
	}
	return (++value);
}


// logarithmic steps
int IntegerItemLogstep::decrement()
{

	for (int i=10; i>0; i--) {
		if (value > pow(10.f, i)) {
			if (value >= 2 * pow(10.f, i)) {
				value -= int(pow(10.f, i));
			} else {
				value -= int(pow(10.f, i-1));
			}
			return value;
		}
	}
	return(--value);
}


bool DecimalItem::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;
	if (!numInput) {
		if (k==SDL_SCANCODE_UP) {
			value+=delta;
			if (value>mmax) {
				if (wrap) value = mmin + (value-mmax);
				else value = mmax;
			}
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDL_SCANCODE_DOWN) {
			value-=delta;
			if (value<mmin) {
				if (wrap) value = mmax - (mmin-value);
				else value = mmin;
			}
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}

		if (k==SDL_SCANCODE_KP_0 || k==SDL_SCANCODE_KP_1 || k==SDL_SCANCODE_KP_2 || k==SDL_SCANCODE_KP_3 || k==SDL_SCANCODE_KP_4 || k==SDL_SCANCODE_KP_5 ||
		        k==SDL_SCANCODE_KP_6 || k==SDL_SCANCODE_KP_7 || k==SDL_SCANCODE_KP_8 || k==SDL_SCANCODE_KP_9 || k==SDL_SCANCODE_KP_PERIOD || k==SDL_SCANCODE_KP_MINUS) {
			// Start editing with numerical numbers
			numInput = true;
			strInput.clear();

			char c = 0;
			switch (k) {
				case SDL_SCANCODE_KP_0 :
					c=48;
					break;
				case SDL_SCANCODE_KP_1 :
					c=49;
					break;
				case SDL_SCANCODE_KP_2 :
					c=50;
					break;
				case SDL_SCANCODE_KP_3 :
					c=51;
					break;
				case SDL_SCANCODE_KP_4 :
					c=52;
					break;
				case SDL_SCANCODE_KP_5 :
					c=53;
					break;
				case SDL_SCANCODE_KP_6 :
					c=54;
					break;
				case SDL_SCANCODE_KP_7 :
					c=55;
					break;
				case SDL_SCANCODE_KP_8 :
					c=56;
					break;
				case SDL_SCANCODE_KP_9 :
					c=57;
					break;
				case SDL_SCANCODE_KP_MINUS :
					c=45;
					break;
				case SDL_SCANCODE_KP_PERIOD:
					c=46;
					break;
				default:
					break;
			}
			strInput += c;

			return true;
		}
	} else {	// numInput == true
		if (k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER || k==SDL_SCANCODE_LEFT || k==SDL_SCANCODE_RIGHT) {
			numInput=false;
			istringstream is(strInput);
			is >> value;
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			if (!onChangeCallback.empty()) onChangeCallback();

			// somewhat improved
			if (k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER) return false;
			else return true;
		}

		if (k==SDL_SCANCODE_UP) {
			istringstream is(strInput);
			is >> value;
			value+=delta;
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			strInput = Utility::doubleToString(value);
			return true;
		}
		if (k==SDL_SCANCODE_DOWN) {
			istringstream wistemp(strInput);
			wistemp >> value;
			value-=delta;
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			strInput = Utility::doubleToString(value);
			return true;
		}

		if (k==SDL_SCANCODE_KP_0 || k==SDL_SCANCODE_KP_1 || k==SDL_SCANCODE_KP_2 || k==SDL_SCANCODE_KP_3 || k==SDL_SCANCODE_KP_4 || k==SDL_SCANCODE_KP_5 ||
		        k==SDL_SCANCODE_KP_6 || k==SDL_SCANCODE_KP_7 || k==SDL_SCANCODE_KP_8 || k==SDL_SCANCODE_KP_9 || k==SDL_SCANCODE_KP_PERIOD || k==SDL_SCANCODE_KP_MINUS) {
			// The user was already editing
			char c = 0;
			switch (k) {
				case SDL_SCANCODE_KP_0 :
					c=48;
					break;
				case SDL_SCANCODE_KP_1 :
					c=49;
					break;
				case SDL_SCANCODE_KP_2 :
					c=50;
					break;
				case SDL_SCANCODE_KP_3 :
					c=51;
					break;
				case SDL_SCANCODE_KP_4 :
					c=52;
					break;
				case SDL_SCANCODE_KP_5 :
					c=53;
					break;
				case SDL_SCANCODE_KP_6 :
					c=54;
					break;
				case SDL_SCANCODE_KP_7 :
					c=55;
					break;
				case SDL_SCANCODE_KP_8 :
					c=56;
					break;
				case SDL_SCANCODE_KP_9 :
					c=57;
					break;
				case SDL_SCANCODE_KP_MINUS :
					c=45;
					break;
				case SDL_SCANCODE_KP_PERIOD:
					c=46;
					break;
				default:
					break;
			}
			strInput += c;
			return true;
		}

		if (k==SDL_SCANCODE_ESCAPE) {
			numInput=false;
			return false;
		}
		return true; // Block every other characters
	}

	return false;
}

string DecimalItem::getString(void)
{
	// Can't directly write value in os because there is a float precision limit bug..
	static char tempstr[16];
	sprintf(tempstr,"%.2f", value);
	string vstr = tempstr;

	if (numInput) return label + (active ? startActive : "") + strInput + (active ? stopActive : "");
	else return label + (active ? startActive : "") + vstr + (active ? stopActive : "");
}


TimeItem::TimeItem(const string& _label, double _JD) :
	CallbackComponent(), JD(_JD), current_edit(NULL), label(_label),
	y(nullptr), m(nullptr), d(nullptr), h(nullptr), mn(nullptr), s(nullptr)
{
	// Note: range limits are 1 beyond normal range to allow for rollover
	// as date is calculated after updates and range limits are enforced then.
	// trystan 10-7-2010: enforce a max negative date of -4712. The Julian calendar
	// is undefined for lesser years and the math falls apart.
	y = new IntegerItem(SpaceDate::getMinSimulationJD(),
	                     SpaceDate::getMaxSimulationJD(), 2000);
	m = new IntegerItem(0, 13, 1);
	d = new IntegerItem(0, 32, 1);
	h = new IntegerItem(-1, 24, 0);
	mn = new IntegerItem(-1, 60, 0);
	s = new IntegerItem(-1, 60, 0);
	current_edit = y;
}

TimeItem::~TimeItem()
{
	delete y;
	delete m;
	delete d;
	delete h;
	delete mn;
	delete s;
}

bool TimeItem::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;

	if (current_edit->onKey(k,v)) {
		computeJD();
		computeYmdhms();
		if (!onChangeCallback.empty()) onChangeCallback();
		return true;
	} else {
		if (k==SDL_SCANCODE_ESCAPE) {
			return false;
		}

		if (k==SDL_SCANCODE_RIGHT) {
			if (current_edit==y) current_edit=m;
			else if (current_edit==m) current_edit=d;
			else if (current_edit==d) current_edit=h;
			else if (current_edit==h) current_edit=mn;
			else if (current_edit==mn) current_edit=s;
			else if (current_edit==s) current_edit=y;
			return true;
		}
		if (k==SDL_SCANCODE_LEFT) {
			if (current_edit==y) current_edit=s;
			else if (current_edit==m) current_edit=y;
			else if (current_edit==d) current_edit=m;
			else if (current_edit==h) current_edit=d;
			else if (current_edit==mn) current_edit=h;
			else if (current_edit==s) current_edit=mn;
			return true;
		}
	}

	return false;
}

// Convert Julian day to yyyy/mm/dd hh:mm:ss and return the string
string TimeItem::getString(void)
{
	computeYmdhms();

	string s1[6];
	string s2[6];
	if (current_edit==y && active) {
		s1[0] = startActive;
		s2[0] = stopActive;
	}
	if (current_edit==m && active) {
		s1[1] = startActive;
		s2[1] = stopActive;
	}
	if (current_edit==d && active) {
		s1[2] = startActive;
		s2[2] = stopActive;
	}
	if (current_edit==h && active) {
		s1[3] = startActive;
		s2[3] = stopActive;
	}
	if (current_edit==mn && active) {
		s1[4] = startActive;
		s2[4] = stopActive;
	}
	if (current_edit==s && active) {
		s1[5] = startActive;
		s2[5] = stopActive;
	}

	return label +
	       s1[0] + y->getString() + s2[0] + "/" +
	       s1[1] + m->getString() + s2[1] + "/" +
	       s1[2] + d->getString() + s2[2] + " " +
	       s1[3] + h->getString() + s2[3] + ":" +
	       s1[4] + mn->getString() + s2[4] + ":" +
	       s1[5] + s->getString() + s2[5];
}


// for use with commands - no special characters, just the local date
string TimeItem::getDateString(void)
{
	computeYmdhms();  // possibly redundant

	return y->getString() + ":" +
	       m->getString() + ":" +
	       d->getString() + "T" +
	       h->getString() + ":" +
	       mn->getString() + ":" +
	       s->getString();
}

// trystan 10-7-2010: clean-up to use consistent date conversion algorithms
void TimeItem::computeYmdhms(void)
{
	int year, month, day, hour, minute;
	SpaceDate::DateTimeFromJulianDay(JD, &year, &month, &day, &hour, &minute, &second);

	y->setValue(year);
	m->setValue(month);
	d->setValue(day);
	h->setValue(hour);
	mn->setValue(minute);
	s->setValue(floor(second));
}

// trystan 10-7-2010: clean-up to use consistent date conversion algorithms
void TimeItem::computeJD(void)
{
	JD = SpaceDate::JulianDayFromDateTime(y->getValue(), m->getValue(), d->getValue(),
	        h->getValue(), mn->getValue(), second);
}


// Widget used to set time zone. Initialized from a file of type /usr/share/zoneinfo/zone.tab
TimeZoneitem::TimeZoneitem(const string& zonetab_file, const string& _label) : label(_label)
{
	if (zonetab_file.empty()) {
		cerr << "Can't find file \"" << zonetab_file << "\"\n" ;
		Log.write("TimeZoneitem : unable to read "+zonetab_file, cLog::LOG_TYPE::L_ERROR);
		exit(0);
	}

	ifstream is(zonetab_file.c_str());

	string unused, tzname;
	char zoneline[256];
	int i;

	while (is.getline(zoneline, 256)) {
		if (zoneline[0]=='#') continue;
		istringstream istr(zoneline);
		istr >> unused >> unused >> tzname;
		i = tzname.find("/");
		if (continents.find(tzname.substr(0,i))==continents.end()) {
			continents.insert(pair<string, MultiSetItem<string> >(tzname.substr(0,i),MultiSetItem<string>()));
			continents[tzname.substr(0,i)].addItem(tzname.substr(i+1,tzname.size()));
			continents_names.addItem(tzname.substr(0,i));
		} else {
			continents[tzname.substr(0,i)].addItem(tzname.substr(i+1,tzname.size()));
		}
	}
	is.close();
	current_edit=&continents_names;
}

TimeZoneitem::~TimeZoneitem()
{
}

bool TimeZoneitem::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;

	if (current_edit->onKey(k,v)) {
		if (!onChangeCallback.empty()) onChangeCallback();
		return true;
	} else {
		if (k==SDL_SCANCODE_ESCAPE) {
			return false;
		}

		if (k==SDL_SCANCODE_RIGHT) {
			if (current_edit==&continents_names) current_edit = &continents[continents_names.getCurrent()];
			else current_edit=&continents_names;
			return true;
		}
		if (k==SDL_SCANCODE_LEFT) {
			if (current_edit==&continents_names) return false;
			else current_edit = &continents_names;
			return true;
		}
	}

	return false;
}

string TimeZoneitem::getString(void)
{
	string s1[2], s2[2];
	if (current_edit==&continents_names && active) {
		s1[0] = startActive;
		s2[0] = stopActive;
	}
	if (current_edit!=&continents_names && active) {
		s1[1] = startActive;
		s2[1] = stopActive;
	}

	return label + s1[0] + continents_names.getCurrent() + s2[0] + "/" + s1[1] +
	       continents[continents_names.getCurrent()].getCurrent() + s2[1];
}

string TimeZoneitem::gettz(void)   // should be const but gives a boring error...
{
	if (continents.find(continents_names.getCurrent())!=continents.end())
		return continents_names.getCurrent() + "/" + continents[continents_names.getCurrent()].getCurrent();
	else return continents_names.getCurrent() + "/error" ;
}

void TimeZoneitem::settz(const string& tz)
{
	int i = tz.find("/");
	continents_names.setCurrent(tz.substr(0,i));
	continents[continents_names.getCurrent()].setCurrent(tz.substr(i+1,tz.size()));
}


string ActionItem::getString(void)
{
	if (difftime(time(NULL), tempo) > 3) {
		if (active) {
			return label + startActive + string_prompt1 + stopActive;
		} else return label + string_prompt1;
	} else {
		if (active) {
			return label + startActive + string_prompt2 + stopActive;
		} else return label + string_prompt2;
	}
}

bool ActionItem::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (v==S_TUI_PRESSED && (k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER)) {
		// Call the callback if enter is pressed
		if (!onChangeCallback.empty()) onChangeCallback();
		tempo = time(NULL);
		return false; // menubranch will now make inactive
	}
	return false;
}

void ActionItem::translateActions()
{
	string_prompt1 = _(englishStringPrompt1);
	string_prompt2 = _(englishStringPrompt2);
}

string ActionConfirmItem::getString(void)
{

	if (difftime(time(NULL), tempo) < 3) {
		return label + string_prompt2;
	}

	if (active) {
		if (isConfirming) {
			return label + startActive + string_confirm + stopActive;
		} else {
			return label + startActive + string_prompt1 + stopActive;
		}
	} else return label + string_prompt1;
}

bool ActionConfirmItem::onKey(SDL_Scancode k, S_TUI_VALUE v)
{

	if (difftime(time(NULL), tempo) < 3) return false;  // no edits while displays Done message

	if (v==S_TUI_PRESSED) {

		if(k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER) {
			if (isConfirming) {
				// Call the callback if enter is pressed
				if (!onChangeCallback.empty()) onChangeCallback();
				tempo = time(NULL);
				isConfirming = false;
				return false; // menubranch will now make inactive
			} else {
				isConfirming = true;
				return true;
			}
		} else if(k==SDL_SCANCODE_ESCAPE || k==SDL_SCANCODE_LEFT) {  // NOTE: SDL_SCANCODE_LEFT is not working here
			if (isConfirming) {
				isConfirming = false;
				return true;
			}
			return false; // menubranch will now make inactive
		} else {
			return true;
		}
	}
	return false;
}

void ActionConfirmItem::translateActions()
{
	string_prompt1 = _(englishStringPrompt1);
	string_prompt2 = _(englishStringPrompt2);
	string_confirm = _(englishStringConfirm);
}


VectorItem::VectorItem(const string& _label, Vec3d _init_vector) :
	CallbackComponent(), current_edit(NULL), label(_label),
	a(NULL), b(NULL), c(NULL)
{
	a = new DecimalItem(0, 1, 0, "", 0.05);
	b = new DecimalItem(0, 1, 0, "", 0.05);
	c = new DecimalItem(0, 1, 0, "", 0.05);
	current_edit = a;
	setVector(_init_vector);
}

VectorItem::~VectorItem()
{
	delete a;
	delete b;
	delete c;
}

bool VectorItem::onKey(SDL_Scancode k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;

	if (current_edit->onKey(k,v)) {

		if (!onChangeCallback.empty()) onChangeCallback();
		return true;
	} else {

		// somewhat improved
		if (k==SDL_SCANCODE_RETURN || k==SDL_SCANCODE_KP_ENTER) {
			if (!onChangeCallback.empty()) onChangeCallback();
			return false;
		}


		if (k==SDL_SCANCODE_ESCAPE) {
			return false;
		}

		if (k==SDL_SCANCODE_RIGHT) {
			if (current_edit==a) current_edit=b;
			else if (current_edit==b) current_edit=c;
			else if (current_edit==c) current_edit=a;
			return true;
		}
		if (k==SDL_SCANCODE_LEFT) {
			if (current_edit==a) current_edit=c;
			else if (current_edit==c) current_edit=b;
			else if (current_edit==b) current_edit=a;
			return true;
		}
	}

	return false;
}


string VectorItem::getString(void)
{
	string s1[3];
	string s2[3];
	if (current_edit==a && active) {
		s1[0] = startActive;
		s2[0] = stopActive;
	}
	if (current_edit==b && active) {
		s1[1] = startActive;
		s2[1] = stopActive;
	}
	if (current_edit==c && active) {
		s1[2] = startActive;
		s2[2] = stopActive;
	}

	return label +
	       s1[0] + a->getString() + s2[0] + " " +
	       s1[1] + b->getString() + s2[1] + " " +
	       s1[2] + c->getString() + s2[2] + " ";
}

// Specialization for strings because operator wstream << string does not exists..
template<>
string MultiSetItem<string>::getString(void)
{
	if (current==items.end()) return label;
	return label + (active ? startActive : "") + *current + (active ? stopActive : "");
}
