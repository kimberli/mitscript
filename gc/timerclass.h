#pragma once

#include <string>
#include <iostream>
#include <chrono>

using namespace std::chrono;

struct timeunit {
	long long tv_sec;
	long long tv_usec;
};


/*
Timer class can be used to measure time in a program.
*/
class timerclass{
	unsigned long long tottime;
	unsigned long long ctime;
	timeunit stime, endtime;
	const std::string name;
	void readclock(timeunit* tv) {
		time_point<high_resolution_clock, milliseconds> msc = time_point_cast<milliseconds>(high_resolution_clock::now());
		tv->tv_sec = msc.time_since_epoch().count() / 1000;
		tv->tv_usec = msc.time_since_epoch().count() % 1000;
	}
	public:
	timerclass(){
		tottime = 0;	
		ctime = 0;
	}

	timerclass(const std::string& pname):name(pname){
		tottime = 0;	
		ctime = 0;
	}
	
	inline timerclass& start(){
		tottime = 0;		
		readclock(&stime);
		return *this;
	}
	inline timerclass& restart(){	
		readclock(&stime);
		return *this;
	}
	inline timerclass& stop(){
		readclock(&endtime);
		ctime =  1000000*(endtime.tv_sec - stime.tv_sec)+
					   (endtime.tv_usec - stime.tv_usec);		
		tottime += ctime;
		stime.tv_sec = endtime.tv_sec;
		stime.tv_usec = endtime.tv_usec;
		return *this;
	}
	inline double get_cur_ms(){
		return (ctime/1000.0);
	}
	inline double get_tot_ms(){
		return (tottime/1000.0);
	}
	inline timerclass& print(){
		std::cout<<name<<":  "<<(ctime/1000.0)<<" ms ";
		if( tottime != ctime) std::cout<<(tottime/1000.0)<<" ms";
		std::cout<<std::endl;
		return *this;
	}
	inline timerclass& print(const std::string& msg){
		std::cout<<name<<" "<<msg<<":  "<<(ctime/1000.0)<<" ms ";
		if( tottime != ctime) std::cout<<(tottime/1000.0)<<" ms";
		std::cout<<std::endl;
		return *this;
	}
};


#endif /*TIMERCLASS_H_*/
