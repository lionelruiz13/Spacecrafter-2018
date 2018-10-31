#include "save_screen.hpp"
#include <SDL2/SDL.h>

#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"
#include "log.hpp"
//~ #include "spacecrafter.hpp"

//~ using namespace std;

SaveScreen::SaveScreen(unsigned int _size)
{
	isAvariable = true;

	size_screen = _size;
	nb_cores= std::max(1,SDL_GetCPUCount()-1); //on veut garder un thread pour la boucle principale
	freeSlot = -1;

	buffer = nullptr;
	tab = nullptr;

	buffer = new unsigned char*[nb_cores];
	if (buffer==nullptr) {
		Log.write("SaveScreen : erreur création buffer", cLog::LOG_TYPE::L_ERROR);
		isAvariable = false;
		return;
	}

	tab = new bool[nb_cores];
	if (tab==nullptr) {
		Log.write("SaveScreen : erreur création tab", cLog::LOG_TYPE::L_ERROR);
		isAvariable = false;
		return;
	}

	for(unsigned int i = 0; i < nb_cores; i++) {
		buffer[i]= nullptr;
		buffer[i] = new unsigned char[3 * size_screen * size_screen];
		if (buffer[i] == nullptr) {
			Log.write("SaveScreen : erreur création buffer individuel", cLog::LOG_TYPE::L_ERROR);
			tab[i]= false;
			isAvariable = false;
			return;
		} else
			tab[i]=true;
	}

	threadpool = nullptr;
	threadpool = new std::thread[nb_cores];
}


SaveScreen::~SaveScreen()
{
	if (!isAvariable)
		return;

	//verification que tous les threads soient terminés
	while (isAllFree() != true) {
		SDL_Delay(50);
	}

	for(unsigned int i = 0; i < nb_cores; i++) {
		delete[] buffer[i];
	}
	delete[] buffer;
	delete[] tab;
}

bool SaveScreen::isAllFree()
{
	mtx.lock();
	for(unsigned int i=0; i< nb_cores; i++) {
		if (tab[i]==false) {
			mtx.unlock();
			return false;
		}
	}
	mtx.unlock();
	return true;
}

void SaveScreen::saveScreenBuffer(const std::string &fileName)
{
	if (!isAvariable)
		return;

	mtx.lock();
	threadpool[freeSlot] = taskThread(fileName, freeSlot);
	tab[freeSlot]=false;
	threadpool[freeSlot].detach();
	freeSlot=-1;
	mtx.unlock();
}

void SaveScreen::getFreeIndex()
{
	freeSlot=-1;
	while (freeSlot == -1) {
		mtx.lock();
		for(unsigned int i=0; i<nb_cores; i++) {
			if (tab[i] == true) {
				//~ printf("trouvé ! %i\n", i);
				freeSlot=i;
				break;
			}
		}
		mtx.unlock();
		SDL_Delay(50);
		//~ printf("tout est occupé\n");
	}
	//~ printf("valeur de freeSlot %i\n",freeSlot);
}

void SaveScreen::saveScreenToFile(const std::string &fileName, int bufferIndice)
{
	//~ printf("dans SaveScreen::saveScreenToFile\n");
	tje_encode_to_file_at_quality(fileName.c_str(), 3,size_screen, size_screen, 3, buffer[bufferIndice]);
	mtx.lock();
	tab[bufferIndice]=true;
	mtx.unlock();
}
