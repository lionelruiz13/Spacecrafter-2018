#include "body_decor.hpp"
#include "body.hpp"
#include "milkyway.hpp"
#include "atmosphere.hpp"

BodyDecor::BodyDecor(MilkyWay* _milky, Atmosphere* _atmosphere)
{
    milky = _milky;
    atmosphere = _atmosphere;
}



void BodyDecor::anchorAssign()
{
    drawLandscape = false;
    drawMeteor = false;
    atmosphere->setFlagShow(false);
    // on est dans l'espace
    drawBody = true;
    milky->useIrisTexture(true);
}

void BodyDecor::bodyAssign(double altitude, const AtmosphereParams* atmParam)
{

    // gestion du landscape et du body
    if (altitude<atmParam->limLandscape) {
        drawLandscape = true;
        drawBody = false;
    } else {
        drawLandscape = false;
        drawBody = true;
    }

    bool hasAtmosphere = atmParam->hasAtmosphere;

    if (!hasAtmosphere) {
        milky->useIrisTexture(true);
        drawMeteor = false;
        atmosphere->setFlagShow(false);
        return;
    }

    bool outZoneAtmosphere = (altitude > atmParam->limSup);

    //on est sur un body mais en dehors de la zone de l'atmosphere
    if (outZoneAtmosphere) {
        atmosphere->setFlagShow(false);
        drawMeteor = false;
        milky->useIrisTexture(true);
        return;
    }

    //on est sur un body qui a une atmosphere et on est dedans
    //l'utilisateur ne veut pas tracer d'atmosphere
    if (!atmState) {
        atmosphere->setFlagShow(false);
        milky->useIrisTexture(true);
        drawMeteor = false;
        return;
    }

    // on se retrouve dans le cas sur un body qui a une atmosphere et on est proche de l'astre
    if (altitude < atmParam->limInf) {
        atmosphere->setFlagShow(true);
        milky->useIrisTexture(false);
        drawMeteor = true;
        return;
    }
    else {
        atmosphere->setFlagShow(false);
        milky->useIrisTexture(true);
        drawMeteor = true;
        return;
    }
}
