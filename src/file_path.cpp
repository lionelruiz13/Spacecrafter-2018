/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 of Association Sirius
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

#include "script_mgr.hpp"
#include "file_path.hpp"
#include "utility.hpp"

std::string FilePath::scriptPath;

FilePath::FilePath(const std::string& fileName)
{
	FilePath(fileName, TFP::NONE);
}

FilePath::FilePath(const std::string& fileName, const std::string& localisation)
{
	if (fileName.empty())
		return;

	//~ printf("FilePath solicité sur internationalisation\n");
	//~ printf("FilePath langue %s\n", localisation.c_str());

	if (scriptPath.empty()) {
		//~ printf("FilePath scriptMgr non initialisé\n");
		return;
	}

	std::string fileNameAdapted = fileName;
	fileNameAdapted[fileNameAdapted.size()-6]=localisation[0];
	fileNameAdapted[fileNameAdapted.size()-5]=localisation[1];
	//~ printf("FilePath internationalisation travaille sur %s\n", fileNameAdapted.c_str());

	if ( !Utility::isAbsolute(fileName)) {
		fullFileName = scriptPath+fileNameAdapted;
		// localisation dans scriptPath
		testFileExistance();
		if (isFileExist)
			return;

		fullFileName = AppSettings::Instance()->getMediaDir() + fileNameAdapted;
		// localisation dans media
		testFileExistance();
		if (isFileExist)
			return;

		//localisation non existante
		fullFileName = scriptPath+fileName;
		// localisation dans scriptPath
		testFileExistance();
		if (isFileExist)
			return;

		fullFileName = AppSettings::Instance()->getMediaDir() + fileName;
		// localisation dans media, dernier test
		testFileExistance();
	}
	else {
		//test version de localisation
		fullFileName = fileNameAdapted;
		testFileExistance();
		if (isFileExist)
			return;

		//test fichier de base
		fullFileName = fileName;
		testFileExistance();
	}
}

std::string FilePath::getPath()
{
	if (isFileExist) {
		std::size_t found = fullFileName.find_last_of("/");
		return fullFileName.substr(0,found+1);
	} else
		return "";
}


FilePath::FilePath(const std::string& fileName, TFP type)
{
	if (fileName.empty())
		return;

	//~ printf("FilePath solicité\n");

	if (!Utility::isAbsolute(fileName)) {
		// On recherche d'abord dans le dossier des scripts si le dossier existe
		//~ if (scriptPath.empty())
			//~ fullFileName = fileName; //on doit etre dans un cas particulier
		//~ else
		if ( ! scriptPath.empty() ) {
			fullFileName = scriptPath+fileName;
			testFileExistance();
		}

		//~ std::cout << "recherche existance fichier  " << fullFileName << std::endl;
		//~ printf("FilePath calculée script : %s\n", fullFileName.c_str());
		// Si le fichier n'existe pas, alors on regarde dans le dossier spécifié
		if (!isFileExist) {
			switch(type) {
				case TFP::AUDIO : fullFileName = AppSettings::Instance()->getAudioDir() + fileName; break;
				case TFP::VIDEO : fullFileName = AppSettings::Instance()->getVideoDir() + fileName; break;
				case TFP::MEDIA : fullFileName = AppSettings::Instance()->getMediaDir() + fileName; break;
				case TFP::VR360 : fullFileName = AppSettings::Instance()->getVR360Dir() + fileName; break;
				case TFP::SCRIPT: fullFileName = AppSettings::Instance()->getScriptDir() + fileName; break;
				case TFP::IMAGE : fullFileName = AppSettings::Instance()->getPictureDir() + fileName; break;
				case TFP::TEXTURE : fullFileName = AppSettings::Instance()->getTextureDir() + fileName; break;
				case TFP::DATA  : fullFileName = AppSettings::Instance()->getUserDir() + fileName; break;
				case TFP::MODEL3D : fullFileName = AppSettings::Instance()->getModel3DDir() + fileName; break;
				default: fullFileName = fileName; break;
				};
		//~ printf("FilePath calculée audio : %s\n", fullFileName.c_str());
		testFileExistance();
		}
	}
	else {
		fullFileName = fileName;
		//~ printf("FilePath non calculé : %s\n", fullFileName.c_str());
		testFileExistance();
	}
	//~ printf("au final : %s\n",fullFileName.c_str());
}


void FilePath::testFileExistance(void)
{
    if (FILE *file = fopen(fullFileName.c_str(), "r")) {
        fclose(file);
        isFileExist = true;
    } else {
        isFileExist = false;
    }
}
