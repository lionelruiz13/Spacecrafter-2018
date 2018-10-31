/*
Classe de debug de performances
Utilité : Cette classe permet de générer des données statistiques sur les temps de calcul
Usage : à inclure dans le programme C++, il est possible de labéliser les timers
Remarque : cette classe est optimisée pour les fonctionnalités et l'aisance d'utilisation et non pour les performances et la précision. 
A ne pas utiliser en production car elle pourrait légèrement dégrader les performances du programme.
Auteur : Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour immersiveadventure.net
Mise à jour le 07/06/2017
*/


#ifndef PERF_DEBUG
#define PERF_DEBUG

#include <climits>
#include <string>
#include <unordered_map>
#include <map>
#include <chrono>
#include <fstream>
#include <math.h>

#define DISTRIBUTION_SIZE 6
#define DISTRIBUTION_MAX 100000

class PerformanceDebugger {

public:
	/*!
	*  \brief Constructeur
	*  \param DefaultOutputFilePath : chemin du fichier par défaut pour l'export (optionnel)
	*/
	PerformanceDebugger(std::string DefaultOutputFilePath = "/tmp/Spacecrafter-perf.csv") { outputFile = DefaultOutputFilePath; } //Constructeur //TODO add Windows FilePath
	~PerformanceDebugger() {} //Destructeur (exporte les données dans le fichier)

	/*!
	*  \brief Démarre un timer (le timer doit être déjà arrêté avant)
	*  \param Label : Nom du timer
	*/
	void startTimer(const std::string Label) { //Démarre un timer
		timer *t = &timers[Label]; //Récupère le pointeur sur le timer d'après son nom
		if(t->state) { //Si le timer est démarré
			stopTimerCpuAndWall(t); //On arrête le timer
			++t->wrongStartCount; //On incrémente le nombre de démarrage la éxécutés
		} else t->state = true; //Sinon on pas l'état du timer à démarré
		startTimerCpuAndWall(t); //On démarre le timer
	}

	/*!
	*  \brief Stop un timer (le timer doit être déjà démarré avant)
	*  \param Label : Nom du timer
	*/
	void stopTimer(const std::string Label) { //Arête un timer
		timer *t = &timers[Label]; //Récupère le pointeur sur le timer d'après son nom
		if(t->state) { //Si le timer est démarré
			stopTimerCpuAndWall(t); //On arrête le timer
			t->state = false; //On passe l'état du timer à arrêté
		} else ++t->wrongStopCount; //On incrémente le nombre d'arrêts mal éxécutés
	}

	/*!
	*  \brief Exporte les statistiques temporelles
	*  \param FilePath : fichier de destination (optionnel)
	*  \param Trunc : vrai si le fichier doit être écrasé (par défaut), faux si les statistiques doivent être écrites à la suite
	*/
	void exportData(const std::string FilePath = "", bool Trunc = true); //Exporte des données dans le fichier (appelé par le destructeur)


private:
	//Structure contenant les données temporelles
	struct time {
		unsigned long long total = 0; //Temps total
		unsigned long long min = ULLONG_MAX; //Temps minimal
		unsigned long long max = 0; //Temps maximal
		unsigned long long nullCount; //Répartition
		unsigned long long distribution[DISTRIBUTION_SIZE]; //Répartition
	};

	//Structure contenant les données d'un timer
	struct timer {
		bool state = false; //Etat du timer
		unsigned long long callCount = 0; //Nombre de démarrages du timer
		unsigned long long wrongStartCount = 0; //Nombre de démarrages du timer alors qu'il était déjà démarré
		unsigned long long wrongStopCount = 0; //Nombre d'arrêtsdu timer alors qu'il était déjà arrêté
		std::clock_t cpuStart; //Date de démarrage du timer CPU
		time cpuTime; //Données temporelles du timer CPU
		std::chrono::high_resolution_clock::time_point wallStart; //Date de démarrage du timer temps réel
		time wallTime; //Données du timer temps réel
	};

	//Map contenant les timers et leur nom
	std::unordered_map<std::string, timer> timers; 

	//Nom du fichier de sortie
	std::string outputFile;
	//Flux du fichier de sortie
	std::ofstream outputFileStream;

	//Mise à jour des statistiques temporelles
	void updateStats(unsigned long long duration, time* t);

	//Démarrage des timers
	void startTimerCpuAndWall(timer* t) {
		++t->callCount; //Incrémentation du nombre d'appels
		t->cpuStart = std::clock(); //Sauvegarde de la date de démarrage CPU
		t->wallStart = std::chrono::high_resolution_clock::now(); //Sauvegarde de la date de démarrage temps réel
	}

	//Arret des timers
	void stopTimerCpuAndWall(timer* t) {
		//Seauvegarde de la date d'arrêt CPU
		std::clock_t cpuStop = std::clock();
		//Sauvegarde de la date d'arrêt temps réel
		std::chrono::high_resolution_clock::time_point wallStop = std::chrono::high_resolution_clock::now(); 
		//Calcul du temps CPU et mise à jour des statistiques temporelles
		updateStats((cpuStop - t->cpuStart)*(1000000/CLOCKS_PER_SEC), &t->cpuTime);
		//Calcul du temps réel et mise à jour des statistiques temporelles
		updateStats(std::chrono::duration<unsigned long long, std::nano>(wallStop - t->wallStart).count()/1000, &t->wallTime);
	}

	//Exporte le header d'un type de timer
	void exportStatsDataHeader(const std::string Type);
	//Exporte les données du timer
	void exportTimerData(timer* t);
	//Exporte les données temporelles d'un timer
	void exportStatsData(time* t, unsigned long long callCount);
};


class AutoPerfDebug {

public:
	AutoPerfDebug(PerformanceDebugger* pd, const std::string Label): pdebug(pd), label(Label) {
		pdebug->startTimer(label);    //Constructeur (démarre le timer)
	}

	~AutoPerfDebug() {
		pdebug->stopTimer(label);    //Destructeur (arrête le timer)
	}

private:
	PerformanceDebugger* pdebug;
	std::string label;
};

//do not include #include "perf_debug.hpp" in all .h
#ifdef __main__
PerformanceDebugger pd;
#else
extern PerformanceDebugger pd;
#endif

#endif
