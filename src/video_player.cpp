/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright 2017 Immersive Adventure
 *
 *
 */

#include "video_player.hpp"
#include <fstream>
#include <SDL2/SDL.h>
#include "log.hpp"
#include "spacecrafter.hpp"
#include "s_texture.hpp"

using namespace std;

VideoPlayer::VideoPlayer()
{
	isAlive = false;
	isInPause = false;
	derive = 0;
	isSeeking = false;
}

VideoPlayer::~VideoPlayer()
{
	playStop();
}

void VideoPlayer::pause()
{
}

void VideoPlayer::init()
{
	isAlive = false;
	isInPause= false;
	derive = 0;
	isSeeking = false;

}

bool VideoPlayer::RestartVideo()
{
	return false;
}

void VideoPlayer::getInfo()
{

}

int VideoPlayer::play(std::string _fileName)
{
	isAlive = false;
	return 0;
}

void VideoPlayer::update()
{
	if (! isAlive)
		return;
}

void VideoPlayer::playStop()
{
	if (isAlive==false)
		return;
}


void VideoPlayer::initTexture()
{
}



/* lets take a leap forward the video */
bool VideoPlayer::JumpVideo(float deltaTime, float &reallyDeltaTime)
{
	if (isAlive==false)
		return false;
}

/* lets take a leap forward the video */
// TODO a factoriser avec la fonction VideoPlayer::JumpVideo
bool VideoPlayer::Invertflow(float &reallyDeltaTime)
{
	if (isAlive==false)
		return false;
}
