#include "perf_debug.hpp"
#include <iostream>

void PerformanceDebugger::updateStats(unsigned long long duration, time* t)   //Mise à jour des statistiques temporelles
{
	t->total += duration; //Mise à jour du temps total
	if(duration < t->min) t->min = duration; //Mise à jour du minimum
	if(duration > t->max) t->max = duration; //Mise à jour du maximum
	if(!duration) ++t->nullCount;
	else if(duration < DISTRIBUTION_MAX) ++t->distribution[(int)log10(duration)]; //Mise à jour de la répartition
	else ++t->distribution[DISTRIBUTION_SIZE-1];
}

/*
Signification du fichier de sortie :

Name : nom passé en paramètre lors des appels à startTimer et stopTimer
Count : démarrages de chaque timer
State : état tu timer au moment de l'export des données
EStart : démarrages du timer alors que le timer n'était pas arrêté
EStop : arrêts du timer alors que le timer n'était pas démarré
Total : temps total en microsecondes
Avg : temps moyen
Min : temps minimum
Max : temps maximum
0 : temps = 0 (inexploitable)
10^x : temps supérieur à 10^x mais inférieur à 10^(x+1)
10^x+ : temps supérieur à 10^x
*/
void PerformanceDebugger::exportData(const std::string FilePath, bool Trunc)   //Exporte des données dans le fichier
{
	std::map<std::string, timer> timersSorted; //Création d'une map triée
	for ( auto it = timers.begin(); it != timers.end(); ++it ) timersSorted[it->first] = it->second; //On parcours les timers et on les range dans la map triée
	if(Trunc) outputFileStream.open((FilePath == "" ? outputFile : FilePath), std::ofstream::out | std::ofstream::trunc); //On essaye d'ouvrir le fichier de destination
	else outputFileStream.open((FilePath == "" ? outputFile : FilePath), std::ofstream::out | std::ofstream::app); //On essaye d'ouvrir le fichier de destination
	if (!outputFileStream.is_open()) { //Si le fichier n'est pas ouvert
		std::cerr << "(EE): Couldn't open Performance Debugger export file ! Please check file/directory permissions" << std::endl; //On envoie une erreur sur la sortie
		throw; //On envoie une exception
	}
	outputFileStream << "Name,Count,State,EStart,EStop,"; //Début du header du csv
	exportStatsDataHeader("CPU"); //Header CPU du csv
	exportStatsDataHeader("Wall"); //Header Wall du csv
	outputFileStream << std::endl; //Fin du header dans le csv
	for ( auto it = timersSorted.begin(); it != timersSorted.end(); ++it ) { //On parcours les timers de la map triée
		outputFileStream << it->first << ","; //On écrit le nom du timer
		exportTimerData(&it->second); //On écrit le reste des données du timer
	}
	outputFileStream << std::endl;
	outputFileStream.close(); //On ferme le fichier
}

void PerformanceDebugger::exportStatsDataHeader(const std::string Type)   //Exporte le header d'un type de timer
{
	outputFileStream << Type << " Total," << "Avg," << "Min," << "Max,";
	outputFileStream << Type << " 0,";
	for(int i = 0; i < DISTRIBUTION_SIZE-1; ++i) outputFileStream << "10^" << i << ",";
	outputFileStream << "10^" << DISTRIBUTION_SIZE-1 << "+,";
}

void PerformanceDebugger::exportStatsData(time* t, unsigned long long callCount)   //Exporte les données temporelles d'un timer
{
	outputFileStream << t->total << "," << (callCount ? t->total/callCount : -1) << "," << t->min << "," << t->max << "," << t->nullCount << ",";
	for(int i = 0; i < DISTRIBUTION_SIZE; ++i) outputFileStream << t->distribution[i] << ",";
}

void PerformanceDebugger::exportTimerData(timer* t)   //Export les données d'un timer
{
	outputFileStream << t->callCount << "," << t->state << "," << t->wrongStartCount << "," << t->wrongStopCount << ",";
	exportStatsData(&t->cpuTime, t->callCount);
	exportStatsData(&t->wallTime, t->callCount);
	outputFileStream << std::endl;
}
