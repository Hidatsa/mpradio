#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <sys/stat.h>
using namespace std;
#include "datastruct.h"
#include "files.h"

constexpr auto RDS_CTL= "/home/pi/rds_ctl";

extern settings s;
list<string> pqueue;
list<string>::iterator it;
playbackStatus ps;

void legacy_rds_init()
{
	remove(RDS_CTL);
	mkfifo(RDS_CTL,S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
	chmod(RDS_CTL,0777);
}

void set_next_element(int qsize)
{
	if(s.resumePlayback && !ps.resumed){
		//load_playback_status();    //already called at init stage
		return;
	}
	if(s.shuffle)
		ps.songIndex=(rand() % qsize);
	else
		ps.songIndex=0;
}

/** 
  * DO NOT HINIBIT THIS FUNCTION
  * as it adds the needed " " for the path as well
  */
string trim_audio_track(string &path)
{
	string trim="";

	if(s.resumePlayback && !ps.resumed){
		int filesize=get_file_size(path);
		float duration=get_song_duration(path);
		int bs=get_file_bs(filesize,duration);

		if(ps.playbackPosition >= 5)		/**< resume few seconds back */
			ps.playbackPosition-=5;

		cout<<"seeking the track..."<<endl;
		trim="dd bs="+to_string(bs)+"k skip="+to_string(ps.playbackPosition)+" if=\""+path+"\" | ";
		path=" - ";
		ps.resumed=true;
	}else{
		trim="";
		ps.playbackPosition=0;
		path="\""+path+"\"";
	}

	return trim;
}

void set_output(string &output)
{
	cout<<"setting audio output to "<<s.output<<endl;
	if(s.output == "FM" || s.output == "fm")
		return;
	if(s.output == "ANALOG" || s.output == "analog"){
		cout<<"ANALOG"<<endl;
		output="aplay";
	}
}

void set_effects(string &sox_params)
{
	sox_params+="compand 0.3,1 6:-70,-60,-20 -5 -90 0.2";
}

int play_storage()
{
	bool repeat = true;
	srand (time(NULL));
	legacy_rds_init();
	
	load_playback_status();
	thread persistPlayback (update_playback_status);

	while(repeat){
		get_list();		/**< generate a file list */
		int qsize=pqueue.size();
	
		string sox="sox -t "+s.format+" -v "+s.storageGain+" -r 48000 -G";
		string sox_params="-t wav - ";
		set_effects(sox_params);
		string pifm1="/home/pi/PiFmRds/src/pi_fm_rds -ctl /home/pi/rds_ctl -ps";
		string pifm2="-rt";
		string pifm3="-audio - -freq";
		string output="";
		string songpath;
		
		if(qsize <= 0) repeat=false;		/**< infinite loop protection if no file are present */
		
		while(qsize > 0)
		{
			it=pqueue.begin();
			set_next_element(qsize);
			advance (it,ps.songIndex);
			songpath=*it;

			size_t found = songpath.find_last_of("/");	/**< extract song name out of the absolute file path */
	  		string songname=songpath.substr(found+1);
			ps.songName=songname;
			cout<<endl<<"PLAY: "<<songpath<<endl;
	
			sox=trim_audio_track(songpath)+sox;		/**< substitute songname with stdin (-) from dd if playback must be resumed */
			output=pifm1+" "+"\""+songname+"\""+" "+pifm2+" "+"\""+songname+"\""+" "+pifm3+" "+s.freq;
			set_output(output);			/**< change output device if specified */
	
			string cmdline=sox+" "+songpath+" "+sox_params+" | "+output;
	
			update_now_playing(songname);

			system(cmdline.c_str());

			pqueue.erase(it);	/**< shorten the playlist and save it after playback */
			qsize--;
			save_list(qsize);
			remove("/home/pi/ps");	/**< removing playback status file as not needed when playback ends */
		}
	}
	return 0;
}

int play_bt(string device)
{
	string sox_params="";
	string output="sudo /home/pi/PiFmRds/src/pi_fm_rds -ps 'BLUETOOTH' -rt 'A2DP BLUETOOTH' -freq "+s.freq+" -audio -";
	set_output(output);			/**< change output device if specified */
	if(s.btBoost)
		set_effects(sox_params);
	
	string cmdline="arecord -D bluealsa -f cd -c 2 | sox -t raw -v "+s.btGain+" -G -b 16 -e signed -c 2 -r 44100 - -t wav - "+sox_params+" | "+output;

	system(cmdline.c_str());
	return 0;
}

