/*
 * Copyright (C) 2016 Association Sirius
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _CORE_IO_HPP_
#define _CORE_IO_HPP_

#include "hip_star_mgr.hpp"
#include "core.hpp"
#include "starNavigator.hpp"
#include "tully.hpp"
#include "oort.hpp"
#include "dso3d.hpp"
#include "landscape.hpp"
#include "starLines.hpp"
#include "body_trace.hpp"
#include "screenFader.hpp"
#include "body_decor.hpp"

class CoreIO {
public:
	CoreIO(Core* _core);
	~CoreIO() {};

	////////////////////////////////////////////////////////////////////////////////
	// Atmosphere---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Atmosphere
	void atmosphereSetFlag(bool b) {
		core->bodyDecor->setAtmosphereState(b);
	}
	//! Get flag for displaying Atmosphere
	bool atmosphereGetFlag(void) const {
		return core->bodyDecor->getAtmosphereState();
	}

	//! set flag for vp Optoma
	void atmosphereSetFlagOptoma(bool b) {
		core->atmosphere->setFlagOptoma(b);
	}

	//! Get flag for vp Optoma
	bool atmosphereGetFlagOptoma(void) const {
		return core->atmosphere->getFlagOptoma();
	}

	//! Set atmosphere fade duration in s
	void atmosphereSetFadeDuration(float f) {
		core->atmosphere->setFaderDuration(f);
	}

	//! Get atmosphere fade duration in s
	float atmosphereGetFadeDuration(void) const {
		return core->atmosphere->getFaderDuration();
	}

	//~ ////////////////////////////////////////////////////////////////////////////////
	//~ // Body---------------------------
	//~ ////////////////////////////////////////////////////////////////////////////////

	void BodyOJMLoad(const std::string &mode, const std::string &name, const std::string &filename, const std::string &pathFile, const Vec3f &Position, const float multiplier) {
		core->ojmMgr->load(mode, name, filename, pathFile, Position, multiplier);
	}

	void BodyOJMRemove(const std::string &mode, const std::string &name){
		core->ojmMgr->remove(mode, name);
	}

	void BodyOJMRemoveAll(const std::string &mode){
		core->ojmMgr->removeAll(mode);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Camera---------------------------
	////////////////////////////////////////////////////////////////////////////////
	
	bool cameraAddAnchor(stringHash_t& param) {
		return core->anchorManager->addAnchor(param); 
	}

	bool cameraRemoveAnchor(const std::string &name) {
		return core->anchorManager->removeAnchor(name);
	}

	bool cameraSwitchToAnchor(const std::string &name) {
		return core->anchorManager->switchToAnchor(name);
	}

	bool cameraMoveToPoint(double x, double y, double z){
		return core->anchorManager->setCurrentAnchorPos(Vec3d(x,y,z));
	}
	
	bool cameraMoveToPoint(double x, double y, double z, double time){
		return core->anchorManager->moveTo(Vec3d(x,y,z),time);
	}
	
	bool cameraMoveToBody(const std::string& bodyName, double time, double alt = -1.0){

		if(bodyName == "selected"){
			return core->anchorManager->moveToBody(core->getSelectedPlanetEnglishName(), time, alt);
		}

		if(bodyName == "default"){
			return core->anchorManager->moveToBody(core->ssystem->getEarth()->getEnglishName(), time, alt);
		}

		return core->anchorManager->moveToBody(bodyName,time, alt);
	}
	
	bool cameraMoveRelativeXYZ( double x, double y, double z) {
		return core->anchorManager->moveRelativeXYZ(x,y,z);
	}
	
	bool cameraTransitionToPoint(const std::string& name){
		return core->anchorManager->transitionToPoint(name);
	}
	
	bool cameraTransitionToBody(const std::string& name){

		if(name == "selected"){
			return core->anchorManager->transitionToBody(core->getSelectedPlanetEnglishName());
		}

		return core->anchorManager->transitionToBody(name);
	}

	bool cameraSave(const std::string& name = "anchor"){
		return core->anchorManager->saveCameraPosition(core->settings->getUserDir() + "anchors/" + name);
	}
	
	bool loadCameraPosition(const std::string& filename){
		return core->anchorManager->loadCameraPosition(core->settings->getUserDir() + "anchors/" + filename);
	}
	
	bool lookAt(double az, double alt, double time = 1.){
		return core->navigation->lookAt(az, alt, time);
	}
	
	bool cameraSetFollowRotation(const std::string& name, bool value){
		return core->anchorManager->setFollowRotation(value);
	}

	void cameraSetRotationMultiplierCondition(float v) {
		core->anchorManager->setRotationMultiplierCondition(v);
	}

	bool cameraAlignWithBody(const std::string& name, double duration){
		return core->anchorManager->alignCameraToBody(name,duration);
	}

	////////////////////////////////////////////////////////////////////////////////
	// CardinalsPoints---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Cardinals Points
	void cardinalsPointsSetFlag(bool b) {
		core->cardinals_points->setFlagShow(b);
	}
	//! Get flag for displaying Cardinals Points
	bool cardinalsPointsGetFlag(void) const {
		return core->cardinals_points->getFlagShow();
	}

	//! Set Cardinals Points color
	void cardinalsPointsSetColor(const Vec3f& v) {
		core->cardinals_points->setColor(v);
	}
	//! Get Cardinals Points color
	Vec3f cardinalsPointsGetColor(void) const {
		return core->cardinals_points->getColor();
	}


	////////////////////////////////////////////////////////////////////////////////
	// Constellations---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set display flag of constellation lines
	void constellationSetFlagLines(bool b) {
		core->asterisms->setFlagLines(b);
	}
	//! Get display flag of constellation lines
	bool constellationGetFlagLines(void) {
		return core->asterisms->getFlagLines();
	}

	//! Set display flag of constellation art
	void constellationSetFlagArt(bool b) {
		core->asterisms->setFlagArt(b);
	}
	//! Get display flag of constellation art
	bool constellationGetFlagArt(void) {
		return core->asterisms->getFlagArt();
	}

	//! Set display flag of constellation names
	void constellationSetFlagNames(bool b) {
		core->asterisms->setFlagNames(b);
	}
	//! Get display flag of constellation names
	bool constellationGetFlagNames(void) {
		return core->asterisms->getFlagNames();
	}

	//! Set display flag of constellation boundaries
	void constellationSetFlagBoundaries(bool b) {
		core->asterisms->setFlagBoundaries(b);
	}
	//! Get display flag of constellation boundaries
	bool constellationGetFlagBoundaries(void) {
		return core->asterisms->getFlagBoundaries();
	}
	Vec3f constellationGetColorBoundaries(void) const {
		return core->asterisms->getBoundaryColor();
	}

	//! Set constellation art intensity
	void constellationSetArtIntensity(float f) {
		core->asterisms->setArtIntensity(f);
	}
	//! Get constellation art intensity
	float constellationGetArtIntensity(void) const {
		return core->asterisms->getArtIntensity();
	}

	//! Set constellation art intensity
	void constellationSetArtFadeDuration(float f) {
		core->asterisms->setArtFadeDuration(f);
	}
	//! Get constellation art intensity
	float constellationGetArtFadeDuration(void) const {
		return core->asterisms->getArtFadeDuration();
	}

	//! Set whether selected constellation is drawn alone
	void constellationSetFlagIsolateSelected(bool b) {
		core->asterisms->setFlagIsolateSelected(b);
	}
	//! Get whether selected constellation is drawn alone
	bool constellationGetFlagIsolateSelected(void) {
		return core->asterisms->getFlagIsolateSelected();
	}

	//! Get constellation line color
	Vec3f constellationGetColorLine() const {
		return core->asterisms->getLineColor();
	}
	//! Set constellation line color
	void constellationSetColorLine(const Vec3f& v) {
		core->asterisms->setLineColor(v);
	}

	//! Get constellation names color
	Vec3f constellationGetColorNames() const {
		return core->asterisms->getLabelColor();
	}
	//! Set constellation names color
	void constellationSetColorNames(const Vec3f& v) {
		core->asterisms->setLabelColor(v);
	}

	//! Set constellation names color
	void constellationSetColorNames(const std::string &argName, const Vec3f& v) {
		core->asterisms->setLabelColor(argName, v);
	}

	//! Get constellation art color
	Vec3f constellationGetColorArt() const {
		return core->asterisms->getArtColor();
	}
	//! Set constellation line color
	void constellationSetColorArt(const Vec3f& v) {
		core->asterisms->setArtColor(v);
	}

	void constellationSetColorBoundaries(const Vec3f& v) {
		core->asterisms->setBoundaryColor(v);
	}

	void constellationSetLineColor(const std::string &argName, const Vec3f& v) {
		core->asterisms->setLineColor(argName, v);
	}

	void constellationSetArtIntensity(const std::string &argName, float intensity) {
		core->asterisms->setArtIntensity(argName, intensity);
	}

	////////////////////////////////////////////////////////////////////////////////
	// dateSun---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! return the JD time when the sun go down
	double dateSunRise(double _jd, double _longitude, double _latitude) {
		return core->timeMgr->dateSunRise(_jd,_longitude, _latitude);
	}

	//! return the JD time when the sun set up
	double dateSunSet(double _jd, double _longitude, double _latitude) {
		return core->timeMgr->dateSunSet(_jd,_longitude, _latitude);
	}

	//! return the JD time when the sun cross the meridian
	double dateSunMeridian(double _jd, double _longitude, double _latitude) {
		return core->timeMgr->dateSunMeridian(_jd,_longitude, _latitude);
	}


	////////////////////////////////////////////////////////////////////////////////
	// DSO---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! hide a particular DSO
	void dsoSelectName(std::string DSOName, bool hide) const {
		return core->nebulas->selectName(hide, DSOName);
	}

	//! hide all DSO
	void dsoHideAll() const {
		core->nebulas->hideAll();
	}

	//! show (unhide) all DSO
	void dsoShowAll() const {
		core->nebulas->showAll();
	}

	//! select all DSO in constellationName to be hidden or showed
	void dsoSelectConstellation(bool hide, std::string constellationName) const {
		core->nebulas->selectConstellation(hide, constellationName);
	}

	//! select all DSO with typeName to be hidden or showed
	void dsoSelectType(bool hide, std::string typeName) const {
		core->nebulas->selectType(hide, typeName);
	}


	////////////////////////////////////////////////////////////////////////////////
	// Fog---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Fog
	void fogSetFlag(bool b) {
		core->landscape->setFlagShowFog(b);
	}
	//! Get flag for displaying Fog
	bool fogGetFlag(void) const {
		return core->landscape->getFlagShowFog();
	}

	////////////////////////////////////////////////////////////////////////////////
	// Illuminate---------------------------
	////////////////////////////////////////////////////////////////////////////////
	bool illuminateLoad(std::string filename, double ra, double de, double angular_size, std::string name, double r, double g, double b, float rotation);

	std::string illuminateRemove(const std::string& name);
	std::string illuminateRemoveAll();

	void illuminateSetSize (double value) {
		core->illuminate_size=value;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Landscape---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Get flag for displaying Landscape
	void landscapeSetFlag(bool b) {
		core->landscape->setFlagShow(b);
	}
	//! Get flag for displaying Landscape
	bool landscapeGetFlag(void) const {
		return core->landscape->getFlagShow();
	}

	////////////////////////////////////////////////////////////////////////////////
	// Milky Way---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Milky Way
	void milkyWaySetFlag(bool b) {
		core->milky_way->setFlagShow(b);
	}
	//! Get flag for displaying Milky Way
	bool milkyWayGetFlag(void) const {
		return core->milky_way->getFlagShow();
	}

	//! Set flag for displaying Zodiacal Light
	void milkyWaySetFlagZodiacal(bool b) {
		core->milky_way->setFlagZodiacal(b);
	}
	//! Get flag for displaying Zodiacal Light
	bool milkyWayGetFlagZodiacal(void) const {
		return core->milky_way->getFlagZodiacal();
	}

	//! Set Milky Way intensity
	void milkyWaySetIntensity(float f) {
		core->milky_way->setIntensity(f);
	}
	//! Get Milky Way intensity
	float milkyWayGetIntensity(void) const {
		return core->milky_way->getIntensity();
	}

	void milkyWayRestoreDefault() {
		core->milky_way->restoreDefaultMilky();
	}

	void milkyWaySetDuration(float f) {
		core->milky_way->setFaderDuration(f);
	}

	void milkyWayRestoreIntensity() {
		core->milky_way->restoreIntensity();
	}

	void milkyWayUseIris(bool useIt) {
		core->milky_way->useIrisTexture(useIt);
	}

	//! Change Milkyway texture
	void milkyWayChange(std::string mdir, float _intensity) {
		core->milky_way->changeMilkywayState(mdir, _intensity);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Nebulae---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Nebulae
	void nebulaSetFlag(bool b) {
		core->nebulas->setFlagShow(b);
		core->dso3d->setFlagShow(b);
	}
	//! Get flag for displaying Nebulae
	bool nebulaGetFlag(void) const {
		return core->nebulas->getFlagShow();
	}

	//! Set flag for displaying Nebulae Hints
	void nebulaSetFlagHints(bool b) {
		core->nebulas->setFlagHints(b);
	}
	//! Get flag for displaying Nebulae Hints
	bool nebulaGetFlagHints(void) const {
		return core->nebulas->getFlagHints();
	}

	//! Set Nebulae Hints circle scale
	void nebulaSetCircleScale(float f) {
		core->nebulas->setNebulaCircleScale(f);
	}
	//! Get Nebulae Hints circle scale
	float nebulaGetCircleScale(void) const {
		return core->nebulas->getNebulaCircleScale();
	}

	//! Set flag for displaying Nebulae as bright
	void nebulaSetFlagBright(bool b) {
		core->nebulas->setFlagBright(b);
	}
	//! Get flag for displaying Nebulae as brigth
	bool nebulaGetFlagBright(void) const {
		return core->nebulas->getFlagBright();
	}

	//! Set maximum magnitude at which nebulae hints are displayed
	void nebulaSetMaxMagHints(float f) {
		core->nebulas->setMaxMagHints(f);
	}
	//! Get maximum magnitude at which nebulae hints are displayed
	float nebulaGetMaxMagHints(void) const {
		return core->nebulas->getMaxMagHints();
	}

	//! return the color for the DSO object
	Vec3f nebulaGetColorLabels(void) const {
		return core->nebulas->getLabelColor();
	}

	//! return the color of the DSO circle
	Vec3f nebulaGetColorCircle(void) const {
		return core->nebulas->getCircleColor();
	}

	void nebulaSetPictoSize(int value) const {
		core->nebulas->setPictoSize(value);
	}

	//!set Flag DSO Name who display DSO name
	void nebulaSetFlagNames (bool value) {
		core->nebulas->setNebulaNames(value);
	}

	//!get flag DSO Name who display DSO name
	bool nebulaGetFlagNames () {
		return core->nebulas->getNebulaNames();
	}

	void nebulaSetColorLabels(const Vec3f& v) {
		core->nebulas->setLabelColor(v);
	}
	void nebulaSetColorCircle(const Vec3f& v) {
		core->nebulas->setCircleColor(v);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Observatory---------------------------
	////////////////////////////////////////////////////////////////////////////////

	std::string observatoryGetLandscapeName() {
		return core->observatory->getLandscapeName();
	}

	double observatoryGetLatitude() {
		return core->observatory->getLatitude();
	}

	double observatoryGetLongitude() {
		return core->observatory->getLongitude();
	}

	double observatoryGetAltitude() {
		return core->observatory->getAltitude();
	}

	void observatorySetLatitude(double l) {
		core->observatory->setLatitude(l);
	}

	void observatorySetLongitude(double l) {
		core->observatory->setLongitude(l);
	}

	void observatorySetAltitude(double l) {
		core->observatory->setAltitude(l);
	}


	void observatorySaveBodyInSolarSystem() {
		core->observatory->saveBodyInSolarSystem();
	}

	void observatoryLoadBodyInSolarSystem() {
		core->observatory->loadBodyInSolarSystem();
	}

	void observatoryFixBodyToSun() {
		core->observatory->fixBodyToSun();
	}

	////////////////////////////////////////////////////////////////////////////////
	// Oort    ---------------------------
	////////////////////////////////////////////////////////////////////////////////
	bool oortGetFlagShow() {
		return core->oort->getFlagShow();
	}

	void oortSetFlagShow(bool b) {
		core->oort->setFlagShow(b);
	}

	////////////////////////////////////////////////////////////////////////////////
	// PersonXX---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Personal Line
	void personnalSetFlag(bool b) {
		core->personal->setFlagShow(b);
	}
	//! Get flag for displaying Personal Line
	bool personnalGetFlag(void) const {
		return core->personal->getFlagShow();
	}
	Vec3f personnalGetColor(void) const {
		return core->personal->getColor();
	}

	void personalLoad(std::string filename) const {
		core->personal->loadData(filename);
	}

	void personalClear() const {
		core->personal->clear();
	};

	//! Set flag for displaying Personeq Line
	void personeqSetFlag(bool b) {
		core->personeq->setFlagShow(b);
	}
	//! Get flag for displaying Personeq Line
	bool personeqGetFlag(void) const {
		return core->personeq->getFlagShow();
	}
	Vec3f personeqGetColor(void) const {
		return core->personeq->getColor();
	}

	void personeqLoad(std::string filename) const {
		core->personeq->loadData(filename);
	}

	void personeqClear() const {
		core->personeq->clear();
	};

	void personalSetColor(const Vec3f& v) {
		core->personal->setColor(v);
	}
	void personeqSetColor(const Vec3f& v) {
		core->personeq->setColor(v);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Planets---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Planets
	void planetsSetFlag(bool b) {
		core->ssystem->setFlagPlanets(b);
	}
	//! Get flag for displaying Planets
	bool planetsGetFlag(void) const {
		return core->ssystem->getFlagPlanets();
	}

	//! Set flag for displaying Planets Trails
	void planetsSetFlagTrails(bool b) {
		core->ssystem->setFlagTrails(b);
	}
	//! Get flag for displaying Planets Trails
	bool planetsGetFlagTrails() const {
		return core->ssystem->getFlagTrails();
	}

	//! Set flag for displaying Planets Axis
	void planetsSetFlagAxis(bool b) {
		core->ssystem->setFlagAxis(b);
	}
	//! Get flag for displaying Planets Axis
	bool planetsGetFlagAxis(void) const {
		return core->ssystem->getFlagAxis();
	}


	//! Set flag for displaying Planets Hints
	void planetsSetFlagHints(bool b) {
		core->ssystem->setFlagHints(b);
	}
	//! Get flag for displaying Planets Hints
	bool planetsGetFlagHints(void) const {
		return core->ssystem->getFlagHints();
	}

	//! Set flag for displaying Planets Orbits
	void planetsSetFlagOrbits(bool b) {
		core->ssystem->setFlagPlanetsOrbits(b);
	}

	//! Set flag for displaying Planet name Orbit
	void planetsSetFlagOrbits(const std::string &_name, bool b) {
		core->ssystem->setFlagPlanetsOrbits(_name, b);
	}

	//! Get flag for displaying Planets Orbits
	bool planetsGetFlagOrbits(void) const {
		return core->ssystem->getFlagPlanetsOrbits();
	}

	//! Set flag for displaying Satellites Orbits
	void satellitesSetFlagOrbits(bool b) {
		core->ssystem->setFlagSatellitesOrbits(b);
	}

	//! Get flag for displaying Satellites Orbits
	bool satellitesGetFlagOrbits(void) const {
		return core->ssystem->getFlagSatellitesOrbits();
	}
	//! Set flag for displaying Planets & Satellites Orbits
	void planetSetFlagOrbits(bool b) {
		core->ssystem->setFlagSatellitesOrbits(b);
		core->ssystem->setFlagPlanetsOrbits(b);
		//core->ssystem->setFlagOrbits(b);
	}

	void planetSetColor(const std::string& englishName, const std::string& color, Vec3f c) const {
		core->ssystem->setBodyColor(englishName, color, c);
	}

	Vec3f planetGetColor(const std::string& englishName, const std::string& color) const {
		return core->ssystem->getBodyColor(englishName, color);
	}

	void planetSetDefaultColor(const std::string& color, Vec3f c) const {
		core->ssystem->setDefaultBodyColor(color, c);
	}

	Vec3f planetGetDefaultColor(const std::string& colorName) const {
		return core->ssystem->getDefaultBodyColor(colorName);
	}

	bool hideSatellitesFlag(){
		return core->ssystem->getHideSatellitesFlag();
	}

	void setHideSatellites(bool val){
		core->ssystem->toggleHideSatellites(val);
	}

	//! Set base planets display scaling factor
	void planetsSetScale(float f) {
		core->ssystem->setScale(f);
	}
	//! Get base planets display scaling factor
	float planetsGetScale(void) const {
		return core->ssystem->getScale();
	}

	//! Set planets viewer scaling factor
	void planetSetSizeScale(std::string name, float f) {
		core->ssystem->setPlanetSizeScale(name, f);
	}

	//! Get planets viewer scaling factor
	float planetGetSizeScale(std::string name) {
		return core->ssystem->getPlanetSizeScale(name);
	}

	////////////////////////////////////////////////////////////////////////////////
	// SkyDraw ---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void skyDrawSetColor(Vec3f a) {
		core->skyDraw->setColor(a);
	}

	void skyDrawAddPoints(std::string str) {
		core->skyDraw->addPoints(str);
	}

	void skyDrawClear() {
		core->skyDraw->clear();
	}

	void skyDrawSetFlag(bool newval) {
		core->skyDraw->setFlagShow(newval);
	}

	bool skyDrawGetFlag() {
		return core->skyDraw->getFlagShow();
	}

	////////////////////////////////////////////////////////////////////////////////
	// StarLines---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying
	void starLinesSetFlag(bool b) {
		core->starLines->setFlagShow(b);
	}

	//! Get flag for displaying
	bool starLinesGetFlag(void) const {
		return core->starLines->getFlagShow();
	}

	//! Vide tous les tampons de tracé
	void starLinesDrop(void) const {
		core->starLines->drop();
	}

	//! Charge un ensemble d'asterismes d'un fichier
	void starLinesLoadData(const std::string &fileName) {
		core->starLines->loadData(fileName);
	}

	//! Charge un asterisme à partir d'une ligne
	void starLinesLoadAsterism(std::string record) const {
		core->starLines->loadStringData(record);
	}

	//! supprime le cata logue complet des asterismes
	void starLinesClear() {
		core->starLines->clear();
	}

	void starLinesLoadCat(const std::string &fileName){
		core->starLines->loadHipCatalogue(fileName);
	}

	void starLinesLoadBinCat(const std::string &fileName){
		core->starLines->loadHipBinCatalogue(fileName);
	}


	////////////////////////////////////////////////////////////////////////////////
	// Skyline et Skygrid---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void skyLineMgrSetColor(std::string name, Vec3f a) {
		core->skyLineMgr->setColor(name, a);
	}

	void skyGridMgrSetColor(std::string name, Vec3f a) {
		core->skyGridMgr->setColor(name, a);
	}

	const Vec3f& skyLineMgrGetColor(std::string name) {
		return core->skyLineMgr->getColor(name);
	}

	const Vec3f& skyGridMgrGetColor(std::string name) {
		return core->skyGridMgr->getColor(name);
	}

	void skyLineMgrFlipFlagShow(std::string name) {
		core->skyLineMgr->flipFlagShow(name);
	}

	void skyGridMgrFlipFlagShow(std::string name) {
		core->skyGridMgr->flipFlagShow(name);
	}

	void skyLineMgrSetFlagShow(std::string name, bool value) {
		core->skyLineMgr->setFlagShow(name, value);
	}

	void skyGridMgrSetFlagShow(std::string name, bool value) {
		core->skyGridMgr->setFlagShow(name, value);
	}

	bool skyLineMgrGetFlagShow(std::string name) {
		return core->skyLineMgr->getFlagShow(name);
	}

	bool skyGridMgrGetFlagShow(std::string name) {
		return core->skyGridMgr->getFlagShow(name);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Stars---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starSetFlag(bool b) {
		core->hip_stars->setFlagStars(b);
		core->starNav->setFlagStars(b);
	}

	bool starGetFlag(void) const {
		return core->hip_stars->getFlagStars();
	}

	void starSetTraceFlag(bool b) {
		core->hip_stars->setFlagTrace(b);
	}

	bool starGetTraceFlag(void) const {
		return core->hip_stars->getFlagTrace();
	}

	void starSetColorTable(int p, Vec3f a) {
		core->hip_stars->setColorStarTable(p,a);
	}

	void starSetDuration(float f) {
		return core->hip_stars->setFaderDuration(f);
	}

	float starGetSizeLimit(void) const {
		return core->hip_stars->getStarSizeLimit();
	}

	void starSetFlagName(bool b) {
		core->hip_stars->setFlagNames(b);
	}
	bool starGetFlagName(void) const {
		return core->hip_stars->getFlagNames();
	}

	void starSetLimitingMag(float f) {
		core->hip_stars->setMagConverterMaxScaled60DegMag(f);
	}
	float starGetLimitingMag(void) const {
		return core->hip_stars->getMagConverterMaxScaled60DegMag();
	}

	void starSetFlagSciName(bool b) {
		core->hip_stars->setFlagSciNames(b);
	}
	bool starGetFlagSciName(void) const {
		return core->hip_stars->getFlagSciNames();
	}

	void starSetFlagTwinkle(bool b) {
		core->hip_stars->setFlagTwinkle(b);
	}
	bool starGetFlagTwinkle(void) const {
		return core->hip_stars->getFlagTwinkle();
	}

	void starSetMaxMagName(float f) {
		core->hip_stars->setMaxMagName(f);
	}
	float starGetMaxMagName(void) const {
		return core->hip_stars->getMaxMagName();
	}

	void starSetSizeLimit(float f) {
		core->starNav->setStarSizeLimit(f);
		core->setStarSizeLimit(f);
	}

	void starSetMaxMagSciName(float f) {
		core->hip_stars->setMaxMagName(f);
	}
	float starGetMaxMagSciName(void) const {
		return core->hip_stars->getMaxMagName();
	}

	void starSetScale(float f) {
		core->starNav->setScale(f);
		core->hip_stars->setScale(f);
	}
	float starGetScale(void) const {
		return core->hip_stars->getScale();
	}

	void starSetMagScale(float f) {
		core->starNav->setMagScale(f);
		core->hip_stars->setMagScale(f);
	}
	float starGetMagScale(void) const {
		return core->hip_stars->getMagScale();
	}

	void starSetTwinkleAmount(float f) {
		core->hip_stars->setTwinkleAmount(f);
	}
	float  starGetTwinkleAmount(void) const {
		return core->hip_stars->getTwinkleAmount();
	}

	////////////////////////////////////////////////////////////////////////////////
	// StarNavigator---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starNavigatorClear(){
		core->starNav->clear();
	}

	void starNavigatorLoad(const std::string &fileName, bool binaryMode){
		core->starNav->loadData(fileName, binaryMode);
	}

	void starNavigatorLoadRaw(const std::string &fileName){
		core->starNav->loadRawData(fileName);
	}

	void starNavigatorLoadOther(const std::string &fileName){
		core->starNav->loadOtherData(fileName);
	}

	void starNavigatorSave(const std::string &fileName, bool binaryMode){
		core->starNav->saveData(fileName, binaryMode);
	}

	////////////////////////////////////////////////////////////////////////////////
	// SunTrace---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying SunTrace
	void bodyTraceSetFlag(bool b) const {
		core->bodytrace->setFlagShow(b);
	}
	//! Get flag for displaying SunTrace
	bool bodyTraceGetFlag(void) const {
		return core->bodytrace->getFlagShow();
	}

	void bodyTraceSetPen(bool b) const {
		core->bodytrace->setPen(b);
	}

	bool bodyTraceGetPen() const {
		return core->bodytrace->getPen();
	}

	void bodyTraceClear () const {
		core->bodytrace->clear();
	}

	void bodyTraceHide(std::string value) const;

	void bodyTraceBodyChange(std::string bodyName) const {
		core->ssystem->bodyTraceBodyChange(bodyName);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Text_usr---------------------------
	////////////////////////////////////////////////////////////////////////////////
	bool textAdd(std::string name, std::string text, int altitude, int azimuth, std::string textSize, Vec3f &color, int duration) {
		return core->text_usr->add(name, text,altitude, azimuth, textSize, color, duration);
	}

	bool textAdd(std::string name, std::string text, int altitude, int azimuth, std::string textSize, int duration) {
		return core->text_usr->add(name, text,altitude, azimuth, textSize, duration);
	}

	void textDel(std::string name) {
		core->text_usr->del(name);
	}

	void textClear() {
		core->text_usr->clear();
	}

	void textNameUpdate(std::string name, std::string text) {
		core->text_usr->nameUpdate(name, text);
	}

	void textDisplay(std::string name , bool displ) {
		core->text_usr->textDisplay(name, displ);
	}

	void textFadingDuration(int a) {
		core->text_usr->setFadingDuration(a);
	}

	void textSetDefaultColor(const Vec3f& v) {
		core->text_usr->setColor(v);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Tully---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void tullySetFlagShow(bool v) {
		core->tully->setFlagShow(v);
	}

	bool tullyGetFlagShow() {
		return core->tully->getFlagShow();
	}

	void tullySetColor(const std::string &colorMode)
	{
		if (colorMode=="white")
			core->tully->setWhiteColor(true);
		if (colorMode=="custom")
			core->tully->setWhiteColor(false);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Time---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set time speed in JDay/sec
	void timeSetSpeed(double ts) {
		core->timeMgr->setTimeSpeed(ts);
	}
	//! Get time speed in JDay/sec
	double timeGetSpeed(void) const {
		return core->timeMgr->getTimeSpeed();
	}

	//! Set the current date in Julian Day
	void setJDay(double JD) {
		core->timeMgr->setJDay(JD);
	}
	//! Get the current date in Julian Day
	double getJDay(void) const {
		return core->timeMgr->getJDay();
	}


	////////////////////////////////////////////////////////////////////////////////
	// UBO---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void uboSetAmbientLight(float v) {
		core->ubo_cam->setAmbientLight(v);
	}

	float uboGetAmbientLight() {
		return core->ubo_cam->getAmbientLight();
	}

private:
	Core* core;
};


#endif
